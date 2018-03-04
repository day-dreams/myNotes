select,poll,epoll详解
===


本片笔记重点在于分析三者的特性.

## 解决问题


传统的socket IO调用都是阻塞的,比如read,recv等.

显然我们需要非阻塞的IO调用,于是有了某些设置socket为非阻塞的操作.socket被设置后,如果read,recv,connect等操作无法立即完成,即数据没有准备好,函数直接返回一个错误码.程序可以转而执行其他任务,再切回来继续调用IO.

但非阻塞IO经常意味着:对同一个socket进行多次轮询.显然浪费了CPU时间.

IO复用虽然也有部分轮询,但是提供了同时处理多个套接字的功能;并且,某些api还改善了轮询,使用更高效的处理方式.



## select

### 使用


```c
long sys_select(int n, fd_set __user *inp, fd_set __user *outp, fd_set __user *exp, struct timeval __user *tvp);
```

select的使用与FD_SET,timeval紧密联系.要注意的是,内核会修改几个指针对象指向的变量,包括timeval.

n描述的是要检查的最大描述符加一.

#### fd_set

代表一个socket集合.内部是个unsigned long数组,每个bit都对应一个socket.由于限制,select只能处理[0,1024)范围内的socket.如果要检查大于1023的socket fd,实际上是无法处理的.

相关操作有:
```c

// definition in kernel
typedef struct {
	unsigned long fds_bits [__FDSET_LONGS];
} __kernel_fd_set;
typedef __kernel_fd_set		fd_set;

// operations
#define FD_SETSIZE		__FD_SETSIZE	//一般是1024,限制了单次select的最多socket
#define FD_SET(fd,fdsetp)	__FD_SET(fd,fdsetp)//add socket
#define FD_CLR(fd,fdsetp)	__FD_CLR(fd,fdsetp)//rmove socket
#define FD_ISSET(fd,fdsetp)	__FD_ISSET(fd,fdsetp)//test if fd in fdset
#define FD_ZERO(fdsetp)		__FD_ZERO(fdsetp)//clear fdset
```

#### time_val

代表一段时间.

```c

struct timeval {
	time_t		tv_sec;		/* seconds */
	suseconds_t	tv_usec;	/* microseconds */
};
```

#### 例子

```c

fd_set to_read;
timeval t;

FS_SET(0,to_read); //标准输入
t.tv_uset=100;

int status=select(0+1,&to_read,NULL,NULL,&t);

```



### 实现剖析

现在来剖析实现细节.

select内部会准备好要select的bitmap,timeout等;真正的select机制是通过do_select完成的.

大体完成这些工作:
* 把传入的timeval转化为long型变量,如果没传入,就使用允许的最大超时时间
* 把传入的fdset转化为bitmap,并准备好要返回的bitmap
* 调用do_select
* do_select返回后,如果进程有需要处理的signal,则先返回select,并告诉调用者需要重新调用select(可能导致do_select做的工作白费)
* 如果没有待处理的signal,则设置好返回值,返回即可.

```c
asmlinkage long
sys_select(int n, fd_set __user *inp, fd_set __user *outp, fd_set __user *exp, struct timeval __user *tvp)
{
	fd_set_bits fds;
	char *bits;
	long timeout;
	int ret, size, max_fdset;

	timeout = MAX_SCHEDULE_TIMEOUT; /* 最大超时时限 */
	if (tvp) {			/* 如果传入了tvp,则设置好timoout变量  */
		time_t sec, usec;

		if ((ret = verify_area(VERIFY_READ, tvp, sizeof(*tvp)))
		    || (ret = __get_user(sec, &tvp->tv_sec))
		    || (ret = __get_user(usec, &tvp->tv_usec)))
			goto out_nofds;

		ret = -EINVAL;
		if (sec < 0 || usec < 0)
			goto out_nofds;

		if ((unsigned long) sec < MAX_SELECT_SECONDS) {
			timeout = ROUND_UP(usec, 1000000/HZ);
			timeout += sec * (unsigned long) HZ;
		}
	}

	ret = -EINVAL;
	if (n < 0)
		goto out_nofds;

	/* max_fdset can increase, so grab it once to avoid race */
	max_fdset = current->files->max_fdset;		/* 检查n是否超过了当前进程的最大fd */
	if (n > max_fdset)
		n = max_fdset;

	/*
	 * We need 6 bitmaps (in/out/ex for both incoming and outgoing),
	 * since we used fdset we need to allocate memory in units of
	 * long-words. 
	 */
	ret = -ENOMEM;
	size = FDS_BYTES(n);
	bits = select_bits_alloc(size); /* 内部调用kmalloc,为bitmap分配内存*/
	if (!bits)
		goto out_nofds;
	fds.in      = (unsigned long *)  bits;	
	fds.out     = (unsigned long *) (bits +   size);
	fds.ex      = (unsigned long *) (bits + 2*size);
	fds.res_in  = (unsigned long *) (bits + 3*size);
	fds.res_out = (unsigned long *) (bits + 4*size);
	fds.res_ex  = (unsigned long *) (bits + 5*size);

	/* 初始化bitmap,一部分作为输入,一部分作为返回值 */
	if ((ret = get_fd_set(n, inp, fds.in)) ||
	    (ret = get_fd_set(n, outp, fds.out)) ||
	    (ret = get_fd_set(n, exp, fds.ex)))
		goto out;
	zero_fd_set(n, fds.res_in);
	zero_fd_set(n, fds.res_out);
	zero_fd_set(n, fds.res_ex);

	/* 超时时间内调用do_select */
	ret = do_select(n, &fds, &timeout);

	/* 更新timeval */
	if (tvp && !(current->personality & STICKY_TIMEOUTS)) {
		time_t sec = 0, usec = 0;
		if (timeout) {
			sec = timeout / HZ;
			usec = timeout % HZ;
			usec *= (1000000/HZ);
		}
		put_user(sec, &tvp->tv_sec);
		put_user(usec, &tvp->tv_usec);
	}

	if (ret < 0)
		goto out;
	if (!ret) {
		ret = -ERESTARTNOHAND;
		if (signal_pending(current))	/* 如果有延迟信号需要处理,直接返回,这次select算作失败,指示调用着重新seelct */
			goto out;
		ret = 0;
	}

	/* 如果调用成功了,传入的fdset会被适当修改;否则,传入的fdset为全0 */	
	if (set_fd_set(n, inp, fds.res_in) ||
	    set_fd_set(n, outp, fds.res_out) ||
	    set_fd_set(n, exp, fds.res_ex))
		ret = -EFAULT;

out:
	select_bits_free(bits, size);/* 释放内存 */
out_nofds:
	return ret;
}
```


#### do_select

do_select其实没有什么复杂的操作，主要工作如下：
* 把进程的状态设置为wait
* 陷入一个循环,在循环里检查socket状态;
	* 设置当前进程的状态为可中断
	* 取出bitmap指针
	* 陷入一个循环:每次循环都检测一定数量的socket fd子集(`unsigned long *bitmap`的一部分,即是`unsigned long`).
		* 检测方法是调用socket fd对应的文件对象里的poll函数,这个函数返回文件被操作的情况.比如是不是发生了输入,是不是发生了输出等.
		* 如果在检测到了可操作的socket,就添加到返回值里
	* 跳出循环.如果没有可操作的socket,就继续循环,知道超时
* 跳出了循环,改变进程状态为running,并把进程从等待队列里移除,即更新`poll_table`
* 返回


do_select其实就是简单的轮询检查,比较好理解.

```c
/* select的真正细节 */
int do_select(int n, fd_set_bits *fds, long *timeout)
{
	struct poll_wqueues table;
	poll_table *wait;
	int retval, i;/* i是当前已经检测了的socket fd的最大值 */
	long __timeout = *timeout;

 	spin_lock(&current->files->file_lock);
	retval = max_select_fd(n, fds);/* 返回max(n,进程打开的最大描述符),因为设计到操作读文件表,所以需要加锁 */
	spin_unlock(&current->files->file_lock);

	if (retval < 0)
		return retval;
	n = retval;

	poll_initwait(&table);/* 把进程状态设置为wait */
	wait = &table.pt;
	if (!__timeout)
		wait = NULL;
	retval = 0;
	for (;;) {
		unsigned long *rinp, *routp, *rexp, *inp, *outp, *exp;

		set_current_state(TASK_INTERRUPTIBLE);/* 设置当前(内核)进程状态:可中断 */

		/* 取出待检测socket的bitmap的指针 */
		inp = fds->in; outp = fds->out; exp = fds->ex;
		rinp = fds->res_in; routp = fds->res_out; rexp = fds->res_ex;

		for (i = 0; i < n; ++rinp, ++routp, ++rexp) {
			unsigned long in, out, ex, all_bits, bit = 1, mask, j;//bit代表当前bitmap块里，正被检测的socket fd
			unsigned long res_in = 0, res_out = 0, res_ex = 0;
			struct file_operations *f_op = NULL;
			struct file *file = NULL;

			// 取出要检测的socket,因为一个socket有可能同时出现在3个或2个或1个bitmap里,所以要合并在一起,保证没有遗漏
			// 把指针移动到下一块要检测的bitmap
			in = *inp++; out = *outp++; ex = *exp++;
			all_bits = in | out | ex;

			// 如果当前bitmpa块没有要检测的socket,则跳过
			if (all_bits == 0) {
				i += __NFDBITS;
				continue;
			}

			// 遍历socket fd的块;按照其他代码,这个块有8*sizeof(usigned long)个socket
			for (j = 0; j < __NFDBITS; ++j, ++i, bit <<= 1) {
				if (i >= n) //如果检测完成
					break;
				if (!(bit & all_bits))
					continue;
				file = fget(i);//根据fd获取文件对象引用
				if (file) {
					f_op = file->f_op;
					mask = DEFAULT_POLLMASK;
					if (f_op && f_op->poll)
						//使用文件操作表里的poll函数，来取出状态码;这个状态码可以指示文件发生了什么操作
						mask = (*f_op->poll)(file, retval ? NULL : wait);
					fput(file);//解引用
					if ((mask & POLLIN_SET) && (in & bit)) {
						//如果当前socket对应的文件 发生了输入操作
						res_in |= bit;
						retval++;
					}
					if ((mask & POLLOUT_SET) && (out & bit)) {
						//如果当前socket对应的文件 发生了输出操作
						res_out |= bit;
						retval++;
					}
					if ((mask & POLLEX_SET) && (ex & bit)) {
						//如果当前socket对应的文件 发生了错误
						res_ex |= bit;
						retval++;
					}
				}
				cond_resched();
			}
			// 更新返回值
			if (res_in)
				*rinp = res_in;
			if (res_out)
				*routp = res_out;
			if (res_ex)
				*rexp = res_ex;
		}
		wait = NULL;
		// 如果检测到有可操作的socket，或者超时了，或者当前进程需要处理信号，就推出
		if (retval || !__timeout || signal_pending(current))
			break;
		if(table.error) {
			retval = table.error;
			break;
		}
		__timeout = schedule_timeout(__timeout);
	}

	// 改变当前进程状态
	__set_current_state(TASK_RUNNING);

	// 唤醒正在等待select返回的用户进程
	poll_freewait(&table);

	/*
	 * Up-to-date the caller timeout.
	 */
	*timeout = __timeout;
	return retval;
}
```


## poll

### 使用


### 实现原理

## epoll

### 使用


### 实现原理


## 注意事项

* select  
	* 几个指针指向的参数会被修改
	* n代表:要检查的最大socket fd+1
	* 如果有signal发给了进程,select会直接返回,并提示调用者重新select一次
