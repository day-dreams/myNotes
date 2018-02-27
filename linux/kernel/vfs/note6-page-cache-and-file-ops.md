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
	- [open()的实现.](#open的实现)
		- [filp_open()](#filp_open)
			- [open_namei()](#open_namei)
			- [dentry_open()](#dentry_open)
		- [fd_install()](#fd_install)
	- [close()的实现](#close的实现)
		- [filp_close()](#filp_close)
	- [读写操作](#读写操作)
		- [read()调用](#read调用)
		- [write()调用](#write调用)

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

缓冲区高速缓存服务于块设备.块设备就是磁盘这样的设备,它们的基本存储概念是扇区,与linux传输交互的是块.块就是一系列扇区,扇区一般是256Byte,块大小一般是页的整数倍.所

在很老的linux版本里,缓冲区高速缓存用来缓存块设备上的块.磁盘上的文件实例化到内存后(即建立inode和file对象),直接使用缓冲区高速缓存来缓存文件,而不是页高速缓存.那个时候的页高速缓存,可能主要用于内存映射什么的.


在linux2.6以后,缓冲区高速缓存被取消了.内核直接使用页高速缓存来处理块设备的缓存问题.我认为,不需要再考虑缓冲区高速缓存了.

## 文件操作

了解VFS基本概念和页高速缓存之后,就可以正式剖析文件操作了.了解文件操作的底层实现后,就可以知道自己的程序到底干了什么,底层有那些性能瓶颈,对职业发展很有帮助.

### open()的实现.

<!-- #### sys_open() -->

open()是glibc提供的编程接口,底层调用了操作系统提供的系统调用sys_open.sys_open才是linux内核层次上的文件打开操作.

sys_open主要完成两件工作:
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
			fd_install(fd, f);								//把文件对象安装到进程的文件描述副表里
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

###　close()的实现

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

filp_close()首先检查file对象的f_count对象.如果f_count为0,说明已经被关闭.按道理是不会出现这种情况的,但实际上是可能出现的:由于进程某些不正确的文件操作,比如错误的更新了f_count,某些操作函数在错误的参数下导致了一些致命error.我认为这种检查主要是防范内核开发者和一些hacker的.

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


###　读写操作

读写才是我最关心的部分!

#### read()调用



#### write()调用

