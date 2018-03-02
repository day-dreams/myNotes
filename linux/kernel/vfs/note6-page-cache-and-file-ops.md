linux虚拟文件系统之页高速缓存,文件的打开关闭和读写
===

<!-- TOC -->

- [页高速缓存](#页高速缓存)
	- [address_space](#address_space)
		- [inode节点关于address_space的字段](#inode节点关于address_space的字段)
		- [address_space](#address_space-1)
	- [页高速缓存的布局](#页高速缓存的布局)
	- [相关操作](#相关操作)
	- [缓冲区高速缓存](#缓冲区高速缓存)
- [文件操作](#文件操作)
	- [open()的实现](#open的实现)
		- [filp_open()](#filp_open)
			- [open_namei()](#open_namei)
			- [dentry_open()](#dentry_open)
		- [fd_install()](#fd_install)
	- [close()的实现](#close的实现)
		- [filp_close()](#filp_close)
- [文件读写](#文件读写)
	- [read()调用](#read调用)
		- [fget_light()](#fget_light)
		- [fpos_read()](#fpos_read)
		- [vfs_read()](#vfs_read)
		- [do_sync_read()与__generic_file_aio_read()](#do_sync_read与__generic_file_aio_read)
	- [write()调用](#write调用)
		- [fget_light()](#fget_light-1)
		- [file_pos_read()](#file_pos_read)
		- [vfs_write()](#vfs_write)
		- [do_sync_write()和__generic_file_aio_write_nolock()](#do_sync_write和__generic_file_aio_write_nolock)
- [需要注意的细节](#需要注意的细节)

<!-- /TOC -->

## 页高速缓存

页高速缓存(page cache)的主要是为提高磁盘读写速度而设立的.磁盘读写通常很慢,根据计算机领域广泛采用的缓存策略,也可以在操作系统实现的文件概念和磁盘之间添加一层缓存.用户和内核读写文件时,都是直接操作缓存,只有必要时,才把缓存回写到磁盘,或者从磁盘重新读取从而更新缓存.

优化磁盘读写并不是page cache的唯一目的,page chage也可以为进程间通信提供服务.通常,进程间共享一块页高速缓存,映射到各自地址空间的不同部分,有的还会抽象出FIFO,管道等概念,进而实现IPC.

### address_space


linux内核在inode基础上构建页缓存.由于inode主要反映物理磁盘上的目录和文件节点,以及一些内存映射文件,和page cache的设立意图吻合,所以直接把访问页高速缓存的功能添加给inode,让进程通过inode来操作缓存.

如果页是用于page cache,那么mapping和index字段会被启用.

```c
struct page {
	page_flags_t flags;		/* 页的状态,包括:脏页,正在被写回,激活状态等 
					 * 提供原子性的操作*/
	atomic_t _count;		/* 引用计数,有多少进程,用于page_count()函数. */
	atomic_t _mapcount;		
	unsigned long private;		
	struct address_space *mapping;	/* NOTE:不是代表属于哪个地址空间
					 * 多个地址空间可能会共享同一个物理页,所以这个说法是显然不成立的
					 * 当page属于交换分区,mapping指向交换分区的地址空间(swapper_space)
					 * 当page属于文件映射或者是页缓存,mapping指向文件的地址空间(address_space)
					 * 当page属于匿名映射,也就是用于进程间通信,mapping只想struct anon_vma对象.
					 */

	pgoff_t index;			/* 这个页在所属内存区域(vma_area_struct)中的偏移量;
					 * 如果这个页是映射到文件的某部分,那么index是page在这个部分里的偏移量,而不是在整个文件中的偏移量.
					 * */
	struct list_head lru;
#if defined(WANT_PAGE_VIRTUAL)
	void *virtual;			/* Kernel virtual address (NULL if
					   not kmapped, ie. highmem) */
#endif /* WANT_PAGE_VIRTUAL */
};

```

#### inode节点关于address_space的字段

* i_mapping  
	总指向索引节点的数据页所有者的页缓存.
* i_data  
	如果页缓存的所有者是文件,i_data即是文件的页高速缓存.此时,i_mapping和i_data引用的是同一个address_space.

如果inode节点的页数据来自与块设备文件,i_mapping和i_data的含义要复杂些,暂时不讨论.

#### address_space

```c
struct address_space {
	struct inode		*host;		/* 这个页缓存的所有者 */
						/* owner: inode, block_device */
	struct radix_tree_root	page_tree;	/* 页缓存中的所有页,以树的形式来组织,方便查找 */
						/* radix tree of all pages */
	spinlock_t		tree_lock;	/* and spinlock protecting it */
	unsigned int		i_mmap_writable;/* 页缓存被几个进程共享.如果没有被共享,则可以写入*/
						/* count VM_SHARED mappings */
	struct prio_tree_root	i_mmap;		/* tree of private and shared mappings */
	struct list_head	i_mmap_nonlinear;/*list VM_NONLINEAR mappings */
	spinlock_t		i_mmap_lock;	/* protect tree, count, list */
	unsigned int		truncate_count;	/* Cover race condition with truncate */
	unsigned long		nrpages;	/* 所有者的页总数 */
						/* number of total pages */
	pgoff_t			writeback_index;/* writeback starts here */
	struct address_space_operations *a_ops;	/* 页的操作方法 */
						/* methods */
	unsigned long		flags;		/* error bits/gfp mask */
	struct backing_dev_info *backing_dev_info; /* device readahead, etc */
	spinlock_t		private_lock;	/* for use by the address_space */
	struct list_head	private_list;	/* ditto */
	struct address_space	*assoc_mapping;	/* ditto */
} __attribute__((aligned(sizeof(long))));

```

可以对address_space进行如下操作:
```c
struct address_space_operations {
	int (*writepage)(struct page *page, struct writeback_control *wbc);
	int (*readpage)(struct file *, struct page *);
	int (*sync_page)(struct page *);

	/* Write back some dirty pages from this mapping. */
	int (*writepages)(struct address_space *, struct writeback_control *);

	/* Set a page dirty */
	int (*set_page_dirty)(struct page *page);

	int (*readpages)(struct file *filp, struct address_space *mapping,
			struct list_head *pages, unsigned nr_pages);

	/*
	 * ext3 requires that a successful prepare_write() call be followed
	 * by a commit_write() call - they must be balanced
	 */
	int (*prepare_write)(struct file *, struct page *, unsigned, unsigned);
	int (*commit_write)(struct file *, struct page *, unsigned, unsigned);
	/* Unfortunately this kludge is needed for FIBMAP. Don't use it */
	sector_t (*bmap)(struct address_space *, sector_t);
	int (*invalidatepage) (struct page *, unsigned long);
	int (*releasepage) (struct page *, int);
	ssize_t (*direct_IO)(int, struct kiocb *, const struct iovec *iov,
			loff_t offset, unsigned long nr_segs);
};

```

### 页高速缓存的布局

任何关于IO的读写操作都需要检查,页是否已经在page cache中,所以需要一个快速的方法来检索页.address_space结构主要通过一个基树来管理和查找缓存页.

基树实际上是个多叉树,每个节点可指向64个其它节点.节点定义如下:
```c

/* address_space中基树的节点 */
struct radix_tree_node {
	unsigned int	count;/* slots中非空指针数量 */
	void		*slots[RADIX_TREE_MAP_SIZE];/* 2^6次方个子节点 */
	unsigned long	tags[RADIX_TREE_TAGS][RADIX_TREE_TAG_LONGS];/* 每个子节点的标志,包括脏页标记等 */
};

struct radix_tree_root {
	unsigned int		height;/* 深度 */
	int			gfp_mask;	
	struct radix_tree_node	*rnode;
};
```

节点之间通过在文件中的偏移量排序,所以给出页偏移量后,很快就可以找到页.

基树的深度反映了文件的最大长度.当基树深度达到6,可以反映16TB大小的文件.通常的文件顶多几个G,3到4曾就可以满足要求,查找缓存页时也需要跳转4层左右,所以还是比较快的.


### 相关操作


主要介绍一个操作:查找页,find_get_page().

find_get_page()首先获取相关锁.再调用radix_tree_lookup来查找页节点,返回的是radix_tree_node中slots的一个元素.

再通过page_cache_get宏来获得struct page中的private字段,并增加struct page的引用计数.private字段指向缓冲区首部,即buffer_head.

```c
struct page * find_get_page(struct address_space *mapping, unsigned long offset)
{
	struct page *page;

	spin_lock_irq(&mapping->tree_lock);
	page = radix_tree_lookup(&mapping->page_tree, offset);
	if (page)
		page_cache_get(page);
	spin_unlock_irq(&mapping->tree_lock);
	return page;
}
```

buffer_head是一个链表,链表元素维护好几个信息:
* 关于块设备的信息(不讨论)
* 这个元素引用的struct page地址
* 下一个元素的指针

可以看出,缓冲区的页不一定是连续的.

```c
struct buffer_head {
	/* First cache line: */
	unsigned long b_state;		/* buffer state bitmap (see above) */
	struct buffer_head *b_this_page;/* circular list of page's buffers */
	struct page *b_page;		/* the page this bh is mapped to */
	atomic_t b_count;		/* users using this block */
	u32 b_size;			/* block size */

	sector_t b_blocknr;		/* block number */
	char *b_data;			/* pointer to data block */

	struct block_device *b_bdev;
	bh_end_io_t *b_end_io;		/* I/O completion */
 	void *b_private;		/* reserved for b_end_io */
	struct list_head b_assoc_buffers; /* associated with another mapping */
};
```


### 缓冲区高速缓存

缓冲区高速缓存服务于块设备.块设备就是磁盘这样的设备,它们的基本存储概念是扇区,与linux传输交互的是块.块就是一系列扇区,扇区一般是256Byte,块大小一般是页的整数倍.

在很老的linux版本里,缓冲区高速缓存用来缓存块设备上的块.磁盘上的文件实例化到内存后(即建立inode和file对象),直接使用缓冲区高速缓存来缓存文件,而不是页高速缓存.那个时候的页高速缓存,可能主要用于内存映射什么的.


在linux2.6以后,缓冲区高速缓存被取消了.内核直接使用页高速缓存来处理块设备的缓存问题.我认为,不需要再考虑缓冲区高速缓存了.

## 文件操作

了解VFS基本概念和页高速缓存之后,就可以正式剖析文件操作了.了解文件操作的底层实现后,就可以知道自己的程序到底干了什么,底层有那些性能瓶颈,对职业发展很有帮助.

### open()的实现

<!-- #### sys_open() -->

open()是glibc提供的编程接口,底层调用了操作系统提供的系统调用sys_open.sys_open才是linux内核层次上的文件打开操作.

sys_open主要完成这些工作:
* 从进程的文件表中搜索可用的文件描述符
* 调用filp_open(),根据文件名搜索inode节点后创建file对象,得到文件对象
* 根据文件对象来初始化进程文件表的对应项

```c
asmlinkage long sys_open(const char __user * filename, int flags, int mode)
{
	char * tmp;
	int fd, error;

#if BITS_PER_LONG != 32
	flags |= O_LARGEFILE;
#endif
	tmp = getname(filename);
	fd = PTR_ERR(tmp);
	if (!IS_ERR(tmp)) {
		fd = get_unused_fd();
		if (fd >= 0) {
			struct file *f = filp_open(tmp, flags, mode);	//根据文件名打开文件，并返回相应的文件对象
			error = PTR_ERR(f);
			if (IS_ERR(f))
				goto out_error;
			fd_install(fd, f);	//把文件对象安装到进程的文件描述副表里
		}
out:
		putname(tmp);
	}
	return fd;

out_error:
	put_unused_fd(fd);
	fd = error;
	goto out;
}
```

#### filp_open() 

filp_open()函数是个关键函数,是open的真正细节.

```c
struct file *filp_open(const char * filename, int flags, int mode)
{
	int namei_flags, error;
	struct nameidata nd;

	namei_flags = flags;
	if ((namei_flags+1) & O_ACCMODE)
		namei_flags++;
	if (namei_flags & O_TRUNC)
		namei_flags |= 2;

	error = open_namei(filename, namei_flags, mode, &nd);
	if (!error)
		return dentry_open(nd.dentry, nd.mnt, flags);

	return ERR_PTR(error);
}

```

filep_open主要调用两个函数来完成任务:open_namei和dentry_open.

##### open_namei()  

这个函数调用path_lookup(),完成路径搜索并返回相应的dentry.path_lookup()会处理一些繁琐的工作:
* 路径切换(dentry的搜索)  
* 链接处理(目录的跳转)  
* 文件系统跳转  
* 访问权限检查
* 磁盘dentry读取
* dcache搜索
* inode的创建

open_namei函数的返回值放在传入的nameidata结构体里,主要包括文件名对应的dentry对象,挂载的文件系统,dentry的深度等.

而dentry对象又指向inode对象,所以相当于一并创建了indoe.此时的inode的页高速缓存也已经创建完成.


```c
int open_namei(const char * pathname, int flag, int mode, struct nameidata *nd)
```

##### dentry_open()

使用open_namei()函数查找到相应的dentry后,filp_open()继续调用dentry_open(),创建file对象.

open_namei()的细节如下:

* 从**空闲的file对象链表**(回忆上一篇的file对象管理布局)中取出一个可用的file对象  
* 设置file对象的一些标志,比如打开方式
* 设置file对象指向的dentry对象,页高速缓存对象
* 把file对象从空闲链表移动到可用链表
* 设置file对象的操作函数表,即把inode节点操作函数表里的部分函数复制过来
* 返回文件对象

```c
struct file *dentry_open(struct dentry *dentry, struct vfsmount *mnt, int flags)
```

定位到代表文件的inode节点.如果inode已经存在,即文件被其他进程打开,那么直接使用这个inode,同时更新相关的引用计数;如果inode不存在,就使用slab机制创建一个inode对象,并为这个文件创建页高速缓存,同时初始化相关的address_space结构体.


至此,sys_open()已经创建好了进程可用的文件对象.

#### fd_install()

fd_install比较简单,就是设置task_struct->files,也就是进程打开的文件表.

```c
void fastcall fd_install(unsigned int fd, struct file * file)
{
	struct files_struct *files = current->files;
	spin_lock(&files->file_lock);
	if (unlikely(files->fd[fd] != NULL))
		BUG();
	files->fd[fd] = file;
	spin_unlock(&files->file_lock);
}
```

### close()的实现

题外话:学到现在,我已经可以自己分析open,close这种不涉及太多算法细节的内核函数了,我感到十分开心.


close()同样是glic实现的,glic封装了sys_close()系统调用.

sys_close()完成两件事情:
* 把给定的文件描述符从进程文件表里抹去
* 关闭文件对象

```c
asmlinkage long sys_close(unsigned int fd)
{
	struct file * filp;
	struct files_struct *files = current->files;

	spin_lock(&files->file_lock);
	if (fd >= files->max_fds)
		goto out_unlock;
	filp = files->fd[fd];
	if (!filp)
		goto out_unlock;
	files->fd[fd] = NULL;
	FD_CLR(fd, files->close_on_exec);
	__put_unused_fd(files, fd);
	spin_unlock(&files->file_lock);
	return filp_close(filp, files);

out_unlock:
	spin_unlock(&files->file_lock);
	return -EBADF;
}

```


#### filp_close()

filp_close()负责文件对象销毁的细节.

filp_close()首先检查file对象的f_count对象.如果f_count为0,说明已经被关闭.按道理是不会出现这种情况的,但实际上:由于进程某些不正确的文件操作,比如错误的更新了f_count,某些操作函数在错误的参数下导致了一些致命error.我认为这种检查主要是防范内核开发者和一些hacker的.

接着,filp_close**试图调用**函数操作表里的flush函数.flush函数应该是平台相关的,不同的文件系统有不同的驱动,实现细节不同.不过flush大体就是把缓冲区的数据回写到磁盘,也就是回写页高速缓存.**试图调用**意味着,文件对象可能是个socket,或者内存映射文件,不需要回写到磁盘.

之后,filp_close调用dnotify_flush,可能导致slab机制回收文件对象指向的inode节点内存.

到这里,完成了文件的关闭.

```c
int filp_close(struct file *filp, fl_owner_t id)
{
	int retval;

	/* Report and clear outstanding errors */
	retval = filp->f_error;
	if (retval)
		filp->f_error = 0;

	if (!file_count(filp)) {
		printk(KERN_ERR "VFS: Close: file count is 0\n");
		return retval;
	}

	if (filp->f_op && filp->f_op->flush) {
		int err = filp->f_op->flush(filp);
		if (!retval)
			retval = err;
	}

	dnotify_flush(filp, id);
	locks_remove_posix(filp, id);
	fput(filp);
	return retval;
}
```


## 文件读写

读写才是我最关心的部分!

### read()调用


read()有可能引起潜在的锁竞争,因为要使用进程文件表,而文件表可能被多个进程共享.

read主要调用这几个函数:
* `fget_light()`,从文件表获取文件对象.如果文件表给共享了,还需要增加文件对象的引用计数.
* `file_pos_read()`,获取文件对象中指针偏移量
* `vfs_read()`,检查相关限制,并决定是使用文件系统专有读操作,还是通用读操作(大部分是通用读).
* `do_sync_read()`,通用读操作的封装.
	* `__generic_file_aio_read()`,这个函数是所有文件系统通用的,及通用读操作.这个函数被`do_sync_read()`调用,内部逻辑涉及到:
		* 检查请求的数据是否已经在address_space中  
		* 获取文件对象的inode对象
		* 向磁盘发起读申请,把数据放到addrss_space  
		* 把数据从内核空间复制到用户空间  
* `file_pos_write()`,更新读写偏移量
* `fput_light()`,如果文件表给共享了,就需要减小文件对象的引用计数.
```c
asmlinkage ssize_t sys_read(unsigned int fd, char __user * buf, size_t count)
{
	struct file *file;
	ssize_t ret = -EBADF;
	int fput_needed;

	file = fget_light(fd, &fput_needed);
	if (file) {
		loff_t pos = file_pos_read(file);
		ret = vfs_read(file, buf, count, &pos);
		file_pos_write(file, pos);
		fput_light(file, fput_needed);
	}

	return ret;
}
```

#### fget_light()

fget_light()是轻量级的文件搜索,搜索发生在进程的文件表里,不涉及vfs.

* 检查进程文件表的引用计数,如果大于1就需要加锁;否则直接操作.
* 检查文件描述符是否在文件表中,即fd是否小于文件表的最大描述符```fd < files->max_fds```
* 如果fd在表中,直接返回对应的file对象;否则返回空指针

```c
struct file fastcall *fget_light(unsigned int fd, int *fput_needed)
{
	struct file *file;
	struct files_struct *files = current->files;

	*fput_needed = 0;
	if (likely((atomic_read(&files->count) == 1))) {
		file = fcheck_files(files, fd);
	} else {
		spin_lock(&files->file_lock);
		file = fcheck_files(files, fd);
		if (file) {
			get_file(file);
			*fput_needed = 1;
		}
		spin_unlock(&files->file_lock);
	}
	return file;
}
```

#### fpos_read()

返回文件内部的偏移量.

```c
static inline loff_t file_pos_read(struct file *file)
{
	return file->f_pos;
}
```
#### vfs_read()

vfs_read()做一些检查,再调用真正的读操作:
* 检查:文件打开模式,操作表,用户提供的数据区域是不是属于用户空间
* 检查偏移量是否合法:包括申请读取的偏移区域是否大于0,申请读取的字节数是否超过了文件支持的单次最大读写大小
* 如果文件本身有专用的读方法,直接使用;否则使用通用的读方法,向bio层发起读请求.事实上,大部分文件系统都没有提供专用的读方法,基本都是使用通用读,即是do_sync_read.
* 返回读取字节的数量


```c
ssize_t vfs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	ssize_t ret;

	// 检查:文件打开模式,操作表,用户提供的数据区域是不是属于用户空间
	if (!(file->f_mode & FMODE_READ))
		return -EBADF;
	if (!file->f_op || (!file->f_op->read && !file->f_op->aio_read))
		return -EINVAL;
	if (unlikely(!access_ok(VERIFY_WRITE, buf, count)))
		return -EFAULT;

	// 检查偏移量是否合法
	ret = rw_verify_area(READ, file, pos, count);
	if (!ret) {

		// 是否可读
		ret = security_file_permission (file, MAY_READ);
		if (!ret) {
			// 如果文件本身有专用的读方法,直接使用;否则使用bio层发起读请求
			if (file->f_op->read)
				ret = file->f_op->read(file, buf, count, pos);
			else
				ret = do_sync_read(file, buf, count, pos);
			if (ret > 0) {
				dnotify_parent(file->f_dentry, DN_ACCESS);
				current->rchar += ret;
			}
			current->syscr++;
		}
	}

	return ret;
}
```

#### do_sync_read()与__generic_file_aio_read()

do_sync_read把请求实例化为kiocb结构体,并调用一个复杂的函数来执行真正的读.这个函数是__generic_file_aio_read(),该函数可以作为所有文件系统的读例程.linux源码中有这样的注释:

>This is the "read()" routine for all filesystems that can use the page cache directly.

__generic_file_aio_read大致工作如下:
* 获取待读取文件的address_space
* 获取文件的索引节点
* 初始化读取位置:待读取页的索引(即地址空间中的页索引),第一个请求字节在页内的偏移量.
* 陷入一个循环,循环读取页.
* 检查待读取页是否已经在address_space中,如果不是,则需要向磁盘发起申请,把数据放到address_space中.
* **把页数据从内核空间拷贝到用户空间**
* 检查是否还有数据需要读.如果没有,则跳出循环;否则继续.

```c

/*
 * This is the "read()" routine for all filesystems
 * that can use the page cache directly.
 */
ssize_t
__generic_file_aio_read(struct kiocb *iocb, const struct iovec *iov,
		unsigned long nr_segs, loff_t *ppos)
{
	struct file *filp = iocb->ki_filp;
	ssize_t retval;
	unsigned long seg;
	size_t count;

	count = 0;
	for (seg = 0; seg < nr_segs; seg++) {
		const struct iovec *iv = &iov[seg];

		/*
		 * If any segment has a negative length, or the cumulative
		 * length ever wraps negative then return -EINVAL.
		 */
		count += iv->iov_len;
		if (unlikely((ssize_t)(count|iv->iov_len) < 0))
			return -EINVAL;
		if (access_ok(VERIFY_WRITE, iv->iov_base, iv->iov_len))
			continue;
		if (seg == 0)
			return -EFAULT;
		nr_segs = seg;
		count -= iv->iov_len;	/* This segment is no good */
		break;
	}

	/* coalesce the iovecs and go direct-to-BIO for O_DIRECT */
	if (filp->f_flags & O_DIRECT) {
		loff_t pos = *ppos, size;
		struct address_space *mapping;
		struct inode *inode;

		mapping = filp->f_mapping;
		inode = mapping->host;
		retval = 0;
		if (!count)
			goto out; /* skip atime */
		size = i_size_read(inode);
		if (pos < size) {
			retval = generic_file_direct_IO(READ, iocb,
						iov, pos, nr_segs);
			if (retval >= 0 && !is_sync_kiocb(iocb))
				retval = -EIOCBQUEUED;
			if (retval > 0)
				*ppos = pos + retval;
		}
		file_accessed(filp);
		goto out;
	}

	retval = 0;
	if (count) {
		for (seg = 0; seg < nr_segs; seg++) {
			read_descriptor_t desc;

			desc.written = 0;
			desc.arg.buf = iov[seg].iov_base;
			desc.count = iov[seg].iov_len;
			if (desc.count == 0)
				continue;
			desc.error = 0;
			do_generic_file_read(filp,ppos,&desc,file_read_actor);
			retval += desc.written;
			if (!retval) {
				retval = desc.error;
				break;
			}
		}
	}
out:
	return retval;
}


```

### write()调用


sys_write乍一看有点像sys_read,主要涉及到这几个函数:
* `fget_light()`,获取文件对象.如果文件表给共享了,就需要减小文件对象的引用计数.
* `file_pos_read()`,获取偏移量
* `vfs_write()`,做一些检查,并调用文件系统的专用写方法,或通用写方法:
	* `generic_file_direct_write`,通用的同步写方法,当文件打开时开启了O_DIRECT才调用.
	* `__generic_file_aio_write_nolock()`,通用的异步写方法,涉及到`请求磁盘分配更多的数据块提供给文件`,`向磁盘提交写请求`.
* `file_pos_write()`,更新偏移量
* `fput_light()`,如果文件表给共享了,就需要减小文件对象的引用计数.

```c
asmlinkage ssize_t sys_write(unsigned int fd, const char __user * buf, size_t count)
{
	struct file *file;
	ssize_t ret = -EBADF;
	int fput_needed;

	file = fget_light(fd, &fput_needed);
	if (file) {
		loff_t pos = file_pos_read(file);
		ret = vfs_write(file, buf, count, &pos);
		file_pos_write(file, pos);
		fput_light(file, fput_needed);
	}

	return ret;
}
```

#### fget_light()

获取文件对象,同sys_read().

#### file_pos_read()

获取文件对象的指针偏移量,同sys_read().

#### vfs_write()

检查工作同sys_read().

也会根据文件系统是否提供专用的写操作,来决定是否调用通用读例程.

```c
ssize_t vfs_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
	ssize_t ret;

	if (!(file->f_mode & FMODE_WRITE))
		return -EBADF;
	if (!file->f_op || (!file->f_op->write && !file->f_op->aio_write))
		return -EINVAL;
	if (unlikely(!access_ok(VERIFY_READ, buf, count)))
		return -EFAULT;

	ret = rw_verify_area(WRITE, file, pos, count);
	if (!ret) {
		ret = security_file_permission (file, MAY_WRITE);
		if (!ret) {
			if (file->f_op->write)
				ret = file->f_op->write(file, buf, count, pos);
			else
				ret = do_sync_write(file, buf, count, pos);
			if (ret > 0) {
				dnotify_parent(file->f_dentry, DN_MODIFY);
				current->wchar += ret;
			}
			current->syscw++;
		}
	}

	return ret;
}
```

#### do_sync_write()和__generic_file_aio_write_nolock()


和do_sysc_read()非常像,不同的是,它要间接调用一个__generic_file_aio_write_nolock(),该函数工作如下:
* 获取文件的address_space,以及inode节点
* 循环检查要写入的每一页,包括是否超出文件大小限制,用户提供的数据是否在合法的用户空间内
* 继续一些其他检查
* 如果文件打开时,开启了O_DIRECT,则采用同步的写操作,直接把数据页同步到磁盘上
* 否则采用异步写,即generic_file_buffered_write,其主要工作如下:
	* 循环处理每个要写的页
		* 调用pre_pare_write():
			- 做检查工作.如果涉及调整磁盘上的文件结构,需要等待磁盘进行额外的工作.比如:进程从文件的中部开始写,覆盖原内容,但写入的内容超出了原文件的大小,磁盘就要为文件分配新的块,内核必须等待这个步骤完成,才能继续处理下一个页的写入,所以内核线程有可能陷入阻塞.
		* 调用commit_write():
			- 真正向磁盘提交写操作.

```c

ssize_t
__generic_file_aio_write_nolock(struct kiocb *iocb, const struct iovec *iov,
				unsigned long nr_segs, loff_t *ppos)
{
	struct file *file = iocb->ki_filp;
	struct address_space * mapping = file->f_mapping;
	size_t ocount;		/* original count */
	size_t count;		/* after file limit checks */
	struct inode 	*inode = mapping->host;
	unsigned long	seg;
	loff_t		pos;
	ssize_t		written;
	ssize_t		err;

	// 循环检查
	ocount = 0;
	for (seg = 0; seg < nr_segs; seg++) {
		const struct iovec *iv = &iov[seg];

		/*
		 * If any segment has a negative length, or the cumulative
		 * length ever wraps negative then return -EINVAL.
		 */
		ocount += iv->iov_len;
		if (unlikely((ssize_t)(ocount|iv->iov_len) < 0))
			return -EINVAL;
		if (access_ok(VERIFY_READ, iv->iov_base, iv->iov_len))
			continue;
		if (seg == 0)
			return -EFAULT;
		nr_segs = seg;
		ocount -= iv->iov_len;	/* This segment is no good */
		break;
	}

	count = ocount;
	pos = *ppos;

	vfs_check_frozen(inode->i_sb, SB_FREEZE_WRITE);

	/* We can write back this queue in page reclaim */
	current->backing_dev_info = mapping->backing_dev_info;
	written = 0;

	err = generic_write_checks(file, &pos, &count, S_ISBLK(inode->i_mode));
	if (err)
		goto out;

	if (count == 0)
		goto out;

	err = remove_suid(file->f_dentry);
	if (err)
		goto out;

	inode_update_time(inode, 1);

	/* coalesce the iovecs and go direct-to-BIO for O_DIRECT */
	if (unlikely(file->f_flags & O_DIRECT)) {
		written = generic_file_direct_write(iocb, iov,
				&nr_segs, pos, ppos, count, ocount);
		if (written < 0 || written == count)
			goto out;
		/*
		 * direct-io write to a hole: fall through to buffered I/O
		 * for completing the rest of the request.
		 */
		pos += written;
		count -= written;
	}


	written = generic_file_buffered_write(iocb, iov, nr_segs,
			pos, ppos, count, written);
out:
	current->backing_dev_info = NULL;
	return written ? written : err;
}
```

## 需要注意的细节

写这篇笔记时,曾忘记一些细节,这里特别列出来.

* 文件的引用计数f_count贯穿与整个打开,读写,关闭流程.

* 只有文件表有同步控制,文件对象本身没有同步控制.如果多个进程共同读/写一个文件, 会造成dirtry
   write/read.