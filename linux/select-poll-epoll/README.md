select,poll,epoll详解
===


本片笔记重点在于分析三者的特性和实现细节.

<!-- TOC -->

- [解决问题](#解决问题)
- [select](#select)
	- [使用](#使用)
		- [fd_set](#fd_set)
		- [time_val](#time_val)
		- [例子](#例子)
	- [实现剖析](#实现剖析)
		- [do_select](#do_select)
- [poll](#poll)
	- [使用](#使用-1)
	- [实现原理](#实现原理)
- [epoll](#epoll)
	- [使用](#使用-2)
	- [实现原理](#实现原理-1)
		- [eventpoll数据结构](#eventpoll数据结构)
			- [eventpoll_mnt](#eventpoll_mnt)
			- [eventpoll](#eventpoll)
		- [epoll_create()](#epoll_create)
		- [ep_getfd()](#ep_getfd)
		- [ep_file_init()](#ep_file_init)
		- [epoll_ctl()](#epoll_ctl)
		- [epoll_wait()](#epoll_wait)
- [注意事项](#注意事项)

<!-- /TOC -->


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

poll内部机制和select很像,不同的是,poll不是通过bitmap来实现扫描,而是直接使用单向链表实现的.

poll在内部会扫描整个链表,每扫描一次都检查下`是不是超时`,`有没有信号待处理`.如果有一次扫描发现了1个或多个可用的sockfd,就直接返回.

poll检查文件是否可用,也是通过文件对象操作表里的poll()函数,和select是一样的.

poll相对select的优点在于:
* 没有socket数量限制
* 内存消耗相对小写

## epoll

### 使用

epoll有两种模式:edge-triggered (ET),level-triggered (LT).即是边沿触发和水平触发.
* ET
	只在fd变得可以进行I/O时,触发一次epoll_wait返回.返回后,不管进程有没有写入fd,或者有没有读取fd,或者有没有把所有数据都读取完,后续epoll_wait都不会被这个fd触发.**如果没有一次性把数据读出来,后续过程的epoll_wait就可能没有发现fd可读,导致数据看起来发生了延迟**
* LT
	只要fd可以IO,就不停的触发epoll_wait返回.

下面的例子尝试从stdin读取数据并打印到stdout:
```cpp
#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int pollfd = epoll_create(1);

    struct epoll_event e, revents[5];
    e.data.fd = 2;
    e.events  = EPOLLIN; /* default as Level Trigger */
    // e.events  = EPOLLIN | EPOLLET; /* set as Edge Trigger */

    epoll_ctl(pollfd, EPOLL_CTL_ADD, 2, &e);

    while (true)
    {
        int nfds = epoll_wait(pollfd, revents, 5, 1000);

        if (nfds == 0)
            continue;

        std::cout << "fd to handle:" << nfds << std::endl;

        for (int i = 0; i != nfds; ++i)
        {
            auto fd = revents[i].data.fd;

            static char buffer[1024] = "";

            read(fd, buffer, 1);
            std::cout << "from fd " << fd << ':' << std::string(buffer) << std::endl;
        }
    }

    return 0;
}
```


### 实现原理

#### eventpoll数据结构

我们先看看eventpoll的数据结构.实际上,我是写完了后面几个小节的笔记才回来继续写这里的.

##### eventpoll_mnt

eventpoll_mnt是eventpoll专用的虚拟文件系统.

epoll函数族的使用和文件描述符紧密相关.内核把epoll机制也虚拟成一个文件系统,用户可以向文件系统里添加“文件”(创建新的eventpoll),可以修改“文件”(在eventpoll里添加、修改、修改要监听的文件),访问“文件”(查看eventpoll中监听的文件是否发生了IO事件).

##### eventpoll

eventpoll就是我们用epoll_create创建的结构.epoll虚拟文件系统里,每个文件对象都对应一个eventpoll,通过file->private_data来访问.
 
eventpoll有自己的锁和信号量,并且维护**进程正在等待哪些文件发生IO事件**,**哪些文件已经发生了IO事件**.

调用epoll_ctl时,内核添加要监听的文件对象到wait_queue中.

当文件对象发生了IO事件,内核又会调用设置好的回调函数,把文件对象添加到readylist,即rdllist中.

等到用户调用了epoll_wait,epoll_wait就尝试把rdllist中的文件对象及其IO事件取出来,从内核空间复制到用户空间,从而完成epoll_wait.如果rdllist是空的,就陷入sleep,知道超时推出或者有就绪的文件被添加到rdllist.

可以看到,这里的开销远小于select和poll.因为select每次都要复制文件描述符到内核,如果fd太多,速度会非常慢;poll又是和select采用类似的轮询机制,要主动调用文件对象的poll方法,获取其状态,也非常慢;只有epoll,在文件对象发生了IO时,回调函数被执行,fd就已经到了rdllist,epoll_wait只需要来拿就行,要么没有数据可复制,要么epoll成功.很多人说这里的复杂度是O(1),我个人倒认为讨论复杂度不太好,毕竟我们已经看到了epoll为什么快,也有很多人验证了这一点.

```c
struct eventpoll {
	/* Protect the this structure access */
	rwlock_t lock;

	/*
	 * This semaphore is used to ensure that files are not removed
	 * while epoll is using them. This is read-held during the event
	 * collection loop and it is write-held during the file cleanup
	 * path, the epoll file exit code and the ctl operations.
	 */
	struct rw_semaphore sem;

	/* Wait queue used by sys_epoll_wait() */
	wait_queue_head_t wq;

	/* Wait queue used by file->poll() */
	wait_queue_head_t poll_wait;

	/* List of ready file descriptors */
	struct list_head rdllist;

	/* RB-Tree root used to store monitored fd structs */
	struct rb_root rbr;
};
```

#### epoll_create()

epoll_create的实现比较清晰,算上下级函数,也没有太复杂的东西.大部分原理是在eventpoll的实现里.

epoll_create主要做这些事情:
* 创建好eventpoll需要的inode,file和文件描述符
* 初始化eventpoll,返回文件描述符

细节如下,部分不相关内容已删除.:
```c
asmlinkage long sys_epoll_create(int size)
{
	int error, fd;
	struct inode *inode;
	struct file *file;

	/* size<0则退出 */
	error = -EINVAL;
	if (size <= 0)
		goto eexit_1;

	/* 创建eventpoll需要的所有结构:inode,file,和文件描述符 */
	error = ep_getfd(&fd, &inode, &file);
	if (error)
		goto eexit_1;

	/* 初始化好eventpoll */
	error = ep_file_init(file);
	if (error)
		goto eexit_2;

	return fd;

eexit_2:
	sys_close(fd);
eexit_1:
	DNPRINTK(3, (KERN_INFO "[%p] eventpoll: sys_epoll_create(%d) = %d\n",
		     current, size, error));
	return error;
}

```

#### ep_getfd()

ep_getfd用于创建eventpoll需要的数据结构.

ep_getfd首先申请一个可用的file对象,再申请一个eventpoll文件系统专用的inode节点,并且要从文件表中获取一个空闲的文件描述符.

接着,ep_getfd开始建立file对象和inode节点的关联.由于file和inode节点之间有一层dentry的抽象,所以需要申请一个dentry对象,dentry的路径名根据inode节点的节点号来生成.

file和inode节点关联后,继续设置file指向的超级块,即全局变量eventpoll_mnt指向的虚拟文件系统.并且要设置好file的文件操作表,完成后把file安装到文件表.

ep_getfd仅限于上述操作,没有设计到更多的细节.至于eventpoll文件系统,需要单独分析.

```c
static int ep_getfd(int *efd, struct inode **einode, struct file **efile)
{
	struct qstr this;
	char name[32];
	struct dentry *dentry;
	struct inode *inode;
	struct file *file;
	int error, fd;

	/* 获取一个可用的文件对象 */
	error = -ENFILE;
	file = get_empty_filp();
	if (!file)
		goto eexit_1;

	/* 从eventpoll文件系统里分配一个节点 */
	inode = ep_eventpoll_inode();
	error = PTR_ERR(inode);
	if (IS_ERR(inode))
		goto eexit_2;

	/* 从进程文件表里获取一个空闲fd,失败则退出 */
	error = get_unused_fd();
	if (error < 0)
		goto eexit_3;
	fd = error;

	/* 创建一个dentry,并把dentry关联到inode节点上,dentry的路径名根据inode的节点号来生成. */
	error = -ENOMEM;
	sprintf(name, "[%lu]", inode->i_ino);
	this.name = name;
	this.len = strlen(name);
	this.hash = inode->i_ino;
	dentry = d_alloc(eventpoll_mnt->mnt_sb->s_root, &this);
	if (!dentry)
		goto eexit_4;
	dentry->d_op = &eventpollfs_dentry_operations;
	d_add(dentry, inode);

	/* 设置文件对象指向的超级块,即全局变量eventpoll_mnt */
	file->f_vfsmnt = mntget(eventpoll_mnt);

	/* 设置其他文件属性 */
	file->f_dentry = dentry;
	file->f_mapping = inode->i_mapping;
	file->f_pos = 0;
	file->f_flags = O_RDONLY;
	/* 把文件操作表设置为eventpoll专有的操作表 */
	file->f_op = &eventpoll_fops;
	file->f_mode = FMODE_READ;
	file->f_version = 0;
	file->private_data = NULL;

	/* 把文件对象安装到进程文件表 */
	fd_install(fd, file);

	*efd = fd;
	*einode = inode;
	*efile = file;
	return 0;

eexit_4:
	put_unused_fd(fd);
eexit_3:
	iput(inode);
eexit_2:
	put_filp(file);
eexit_1:
	return error;
}
```

#### ep_file_init()

ep_file_init完成eventpoll fd代表的evenpoll的初始化工作.

ep_file_init会初始化好eventpoll的读写锁和等待队列,由于是新创建的,等待队列为空.file对象通过private_data字段来存放eventpoll.

static int ep_file_init(struct file *file)
{
	struct eventpoll *ep;

	/* 分配一个eventpoll */
	if (!(ep = kmalloc(sizeof(struct eventpoll), GFP_KERNEL)))
		return -ENOMEM;

	memset(ep, 0, sizeof(*ep));
	/* 初始化读写锁 */
	rwlock_init(&ep->lock);
	init_rwsem(&ep->sem);
	/* 初始化等待队列,其实就是把:wq->next=wq,wq->prev=wq */
	init_waitqueue_head(&ep->wq);
	init_waitqueue_head(&ep->poll_wait);
	INIT_LIST_HEAD(&ep->rdllist);

	/* 初始化红黑树,用于管理被监听的fd */
	ep->rbr = RB_ROOT;

	/* eventpoll直接被存储在file->private_data里 */
	file->private_data = ep;

	DNPRINTK(3, (KERN_INFO "[%p] eventpoll: ep_file_init() ep=%p\n",
		     current, ep));
	return 0;
}

#### epoll_ctl()

epoll_ctl处理event的添加、删除等操作.大概如下:
* 把event复制到内核空间
* 获取eventpoll对应的文件对象,以及待监听的文件对象,并检查它们是否支持epoll操作
* 在内部的hash表里查找要监听的文件对象.这决定申请的操作是否合法,比如不允许重复添加,不允许删除和修改不在eventpoll的fd等
* 根据操作类型,分别执行ep_insert,ep_remove,ep_modify操作

```c
asmlinkage long
sys_epoll_ctl(int epfd, int op, int fd, struct epoll_event __user *event)
{
	int error;
	struct file *file, *tfile;
	struct eventpoll *ep;
	struct epitem *epi;
	struct epoll_event epds;

	DNPRINTK(3, (KERN_INFO "[%p] eventpoll: sys_epoll_ctl(%d, %d, %d, %p)\n",
		     current, epfd, op, fd, event));

	/* 检查是否是申请删除event,并把event从用户空间复制到内核空间(可见每次) */
	error = -EFAULT;
	if (EP_OP_HASH_EVENT(op) &&
	    copy_from_user(&epds, event, sizeof(struct epoll_event)))
		goto eexit_1;

	/* 获取eventpoll对应的文件对象 */
	error = -EBADF;
	file = fget(epfd);
	if (!file)
		goto eexit_1;

	/* 获取目标监听文件对应的文件对象 */
	tfile = fget(fd);
	if (!tfile)
		goto eexit_2;

	/* 两部分检查,必须确认用户传进来的是个eventpoll对象,并且要监听的文件对象必须支持epoll相关操作 */
	error = -EPERM;
	if (!tfile->f_op || !tfile->f_op->poll)
		goto eexit_3;

	error = -EINVAL;
	if (file == tfile || !IS_FILE_EPOLL(file))
		goto eexit_3;

	/* 检查完毕,获取eventpoll */
	ep = file->private_data;

	down_write(&ep->sem);

	/* 在hash表中查找文件对象 */
	epi = ep_find(ep, tfile, fd);

	/* 增加,删除,修改event */
	error = -EINVAL;
	switch (op) {
	case EPOLL_CTL_ADD:
		if (!epi) {
			epds.events |= POLLERR | POLLHUP;

			error = ep_insert(ep, &epds, tfile, fd);
		} else
			error = -EEXIST;
		break;
	case EPOLL_CTL_DEL:
		if (epi)
			error = ep_remove(ep, epi);
		else
			error = -ENOENT;
		break;
	case EPOLL_CTL_MOD:
		if (epi) {
			epds.events |= POLLERR | POLLHUP;
			error = ep_modify(ep, epi, &epds);
		} else
			error = -ENOENT;
		break;
	}

	/*
	 * The function ep_find() increments the usage count of the structure
	 * so, if this is not NULL, we need to release it.
	 */
	if (epi)
		ep_release_epitem(epi);

	up_write(&ep->sem);

eexit_3:
	fput(tfile);
eexit_2:
	fput(file);
eexit_1:
	DNPRINTK(3, (KERN_INFO "[%p] eventpoll: sys_epoll_ctl(%d, %d, %d, %p) = %d\n",
		     current, epfd, op, fd, event, error));

	return error;
}
```

* ep_insert

ep_insert把要监听的文件对象安装到eventpoll,并且要设置回调函数.

当期待的文件对象事件发生了,回调函数会被调用,把监听这个文件对象的等待队列移动到文件对象需要唤醒的唤醒队列里.

* ep_remove

从eventpoll中删除要监听的文件对象,并删除文件对象的唤醒队列.

* ep_modify

做一些修改.

#### epoll_wait()

epoll_wait先做一些检查工作,再调用ep_poll完成真正的epoll_wait.

epoll_wait就是在一个timeout内,把内核监测到的满足IO条件的文件描述符返回给用户,内部需要从内核空间复制数据到用户空间.文件描述符只是被复制到了用户,但并没有从eventpoll中删除.

我觉的没有什么值得注意的细节,所以就不放代码了.

## 注意事项

* select  
	* 几个指针指向的参数会被修改
	* n代表:要检查的最大socket fd+1
	* 如果有signal发给了进程,select会直接返回,并提示调用者重新select一次
* poll
	* 非常像select,底层都依赖文件操作表的poll函数
	* 没有socket数量限制
	* 依然存在轮询的问题,轮询代表每次调用,都需要从用户空间复制数据到内核空间,再由内核去查询状态,还是非常低效的.