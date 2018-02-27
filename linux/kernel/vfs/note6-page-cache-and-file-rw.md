页高速缓存和文件读写
===


## 页高速缓存

页高速缓存(page cache)的主要是为提高磁盘读写速度而设立的.磁盘读写通常很慢,根据计算机领域广泛采用的缓存策略,也可以在操作系统实现的文件概念和磁盘之间添加一层缓存.用户和内核读写文件时,都是直接操作缓存,只有必要时,才把缓存回写到磁盘,或者从磁盘重新读取从而更新缓存.

优化磁盘读写并不是page cache的唯一目的,page chage也可以为进程间通信提供服务.通常,进程间共享一块页高速缓存,映射到各自地址空间的不同部分,有的还会抽象出FIFO,管道等概念,进而实现IPC.

### address_space


linux内核在inode基础上构建页缓存.由于inode主要反映物理磁盘上的目录和文件节点,以及一些内存映射文件,和page cache的设立意图温和,所以直接把访问页高速缓存的功能添加给inode,让进程通过inode来操作缓存.

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
* 关于快设备的信息(不讨论)
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

## 文件读写

### open()函数

### read()函数

### write()函数