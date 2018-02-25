linux虚拟文件系统之层次结构
===

<!-- TOC -->

- [从内核角度看](#从内核角度看)
	- [super_block](#super_block)
	- [dentry](#dentry)
		- [目录项的组织形式](#目录项的组织形式)
	- [inode](#inode)
		- [索引节点的组织](#索引节点的组织)
	- [file](#file)
- [从进程角度看](#从进程角度看)
	- [files_struct](#files_struct)

<!-- /TOC -->

## 从内核角度看

### super_block

超级块代表linux挂载的一个文件系统,每个超级块都有唯一的根目录.比如我的笔记本里,双系统下能看到很多磁盘,如果都挂载上,就会产生好几个超级块.

超级块主要用来描述一个文件系统的细节,比如文件系统上每个数据块的大小,支持的最大文件大小,挂载目录等.

linux支持的文件系统,都必须设置好一个超级块对象,放在物理存储上.内核使用do_kern_mount()函数从磁盘上读取超级块,并实例化到内存,最终完成挂载操作.所以,do_kern_mount并不是平台相关的,不管是什么文件系统,内核都使用这个函数来读取超级块信息.


```c
extern struct vfsmount *do_kern_mount(const char *fstype, int flags,
				      const char *name, void *data);
```

```c
struct super_block {
	struct list_head			s_list;			/* 链表头,链表内是所有超级块对象 */
	dev_t					s_dev;			/* 设备标识号 */
	unsigned long				s_blocksize;		/* 块大小(byte) */
    	unsigned long 				s_old_blocksize;      	/* 块大小(bit) */
    	unsigned char				s_blocksize_bits;
	unsigned char				s_dirt;
	unsigned long long			s_maxbytes;		/* Max file size */
	struct file_system_type			*s_type;
	struct super_operations			*s_op;			/* 超级块方法 */
	struct dquot_operations			*dq_op;
 	struct quotactl_ops	*s_qcop;
	struct export_operations 		*s_export_op;
	unsigned long				s_flags;
	unsigned long				s_magic;
	struct dentry				*s_root;		/* 目录挂载点 */
	struct rw_semaphore			s_umount;
	struct semaphore			s_lock;
	int					s_count;
	int					s_syncing;
	int					s_need_sync_fs;
	atomic_t				s_active;
	void                    		*s_security;
	struct xattr_handler			**s_xattr;

	struct list_head			s_inodes;		/* 这个超级块下所有的inodes链表 */
	struct list_head			s_dirty;		/* 被修改过了的inodes链表 */
	struct list_head			s_io;			/* 需要回写的inodes链表 */
	struct hlist_head			s_anon;			/* anonymous dentries for (nfs) exporting */
	struct list_head			s_files;

	struct block_device			*s_bdev;
	struct list_head			s_instances;
	struct quota_info			s_dquot;	/* Diskquota specific options */

	int					s_frozen;
	wait_queue_head_t			s_wait_unfrozen;

	char s_id[32];				/* Informational name */

	void 			*s_fs_info;	/* Filesystem private info */

	/*
	 * The next field is for VFS *only*. No filesystems have any business
	 * even looking at it. You had been warned.
	 */
	struct semaphore s_vfs_rename_sem;	/* Kludge */

	/* Granuality of c/m/atime in ns.
	   Cannot be worse than a second */
	u32		   s_time_gran;
};

```


超级块支持的操作如下,主要设计到inode的分配,销毁,读取等.
```c

struct super_operations {
   	struct inode *(*alloc_inode)(struct super_block *sb);
	void (*destroy_inode)(struct inode *);

	void (*read_inode) (struct inode *);/* 读是指,用磁盘上inode的数据填充内存里的inode对象.  */
  
   	void (*dirty_inode) (struct inode *);
	int (*write_inode) (struct inode *, int);
	void (*put_inode) (struct inode *);
	void (*drop_inode) (struct inode *);
	void (*delete_inode) (struct inode *);
	void (*put_super) (struct super_block *);
	void (*write_super) (struct super_block *);
	int (*sync_fs)(struct super_block *sb, int wait);
	void (*write_super_lockfs) (struct super_block *);
	void (*unlockfs) (struct super_block *);
	int (*statfs) (struct super_block *, struct kstatfs *);
	int (*remount_fs) (struct super_block *, int *, char *);
	void (*clear_inode) (struct inode *);
	void (*umount_begin) (struct super_block *);

	int (*show_options)(struct seq_file *, struct vfsmount *);

	ssize_t (*quota_read)(struct super_block *, int, char *, size_t, loff_t);
	ssize_t (*quota_write)(struct super_block *, int, const char *, size_t, loff_t);
};

```

### dentry

由于文件系统抽象到用户曾都是目录分级的.内核需要使用一个对象来表示一个目录,也就是dentry.每个目录都有自己的父dentry(根目录除外),并且可以有自己的子目录.如果dentry对象已经到了最底层,也就是指向文件,那么会用一个内部数据域来指向inode对象.

如果dentry是有效的,即正被内核使用,那么数据域d_inode指向相关联的索引节点,并且其他数据域都是有效的.

磁盘上也存在目录这个概念,当磁盘被挂载,相应的目录数据需要被填充到一系列dentry对象里.

```c
struct dentry {
	atomic_t d_count;		/* 引用计数  */
	unsigned int d_flags;		/* protected by d_lock */
	spinlock_t d_lock;		/* 每个目录项目都有一个锁 */
	struct inode *d_inode;		/* 相关联的索引节点 */
					/* Where the name belongs to - NULL is
					 * negative */
	/*
	 * The next three fields are touched by __d_lookup.  Place them here
	 * so they all fit in a 16-byte range, with 16-byte alignment.
	 */
	struct dentry *d_parent;	/* 父目录项 */
	struct qstr d_name;

	struct list_head d_lru;		/* LRU list */
	struct list_head d_child;	/* child of parent list */
	struct list_head d_subdirs;	/* 子目录项的链表*/
	struct list_head d_alias;	/* inode alias list */
	unsigned long d_time;		/* used by d_revalidate */
	struct dentry_operations *d_op;
	struct super_block *d_sb;	/* 这个目录项所在的超级块对象,即是这个目录项被挂载的那个文件系统 */
	void *d_fsdata;			/* 文件系统特有数据 */
 	struct rcu_head d_rcu;
	struct dcookie_struct *d_cookie; /* cookie, if any */
	struct hlist_node d_hash;	/* lookup hash list */	
	int d_mounted;
	unsigned char d_iname[DNAME_INLINE_LEN_MIN];	/* small names */
};

```

#### 目录项的组织形式

由于目录树的具体信息在磁盘上,每次查找目录都需要从磁盘读入dentry,再读入子dentry,完成搜索.内核设立了一个目录项缓存,缓存dcache目录使用过的目录项,完成快速查找.dcache目录就是系统已经使用过的目录项缓存,与磁盘上的数据基本一致,除非发生了更新.

在dcache缓存中,一个dentry对象会被加入到四个链表里,分别是:用于快速查找的hash链表,属于同一inode的别名链,暂不使用的链,统一父目录的子目录链.

目录项的操作中,含有一个hash值生成函数,就是用于快速查找的.

```c
struct dentry_operations {
	int (*d_revalidate)(struct dentry *, struct nameidata *);
	int (*d_hash) (struct dentry *, struct qstr *);
	int (*d_compare) (struct dentry *, struct qstr *, struct qstr *);
	int (*d_delete)(struct dentry *);
	void (*d_release)(struct dentry *);
	void (*d_iput)(struct dentry *, struct inode *);
};
```

### inode

索引节点是一个唯一代表文件的数据结构.文件系统会在磁盘上,维护每个文件的索引节点,来描述文件的大小,位置等信息.linux访问文件时,必须把inode读入到内存.

inode的唯一性由索引节点号体现.一个文件系统中的所有inode都有一个编号,这个编号在同一个块设备中是唯一的.这个编号可以通过```ls -i```查看:

```
moon@moon-Think:~/workplace/myNotes$ ls -i
10619095 alg  10624202 database  10624204 plan    10487876 README.md
10617495 cpp  10619080 linux     11537364 python
```

inode虽然代表一个文件,但内存中的inode并不包含文件名信息,文件名信息被存储在dentry中.

inode还可以代表其他东西,不只是文件,还可以是目录,FIFO等.

索引节点的结构很复杂,涉及到索引节点的索引号,块大小,所有着情况,权限,大小等:
```c
struct inode {
	struct hlist_node	i_hash;
	struct list_head	i_list;
	struct list_head	i_sb_list;
	struct list_head	i_dentry;
	unsigned long		i_ino;	/* 索引节点号，在块设备内是唯一的 */
	atomic_t		i_count;
	umode_t			i_mode;/* 文件类型与访问权限 */
	unsigned int		i_nlink;
	uid_t			i_uid;	/* 所有者标识符 */
	gid_t			i_gid;	/* 组标识符 */
	dev_t			i_rdev;	
	loff_t			i_size;	/* 文件大小(bytes) */
	struct timespec		i_atime;
	struct timespec		i_mtime;
	struct timespec		i_ctime;
	unsigned int		i_blkbits;
	unsigned long		i_blksize;
	unsigned long		i_version;/* 版本号,文件每被使用一次都会加1 */
	unsigned long		i_blocks;
	unsigned short          i_bytes;
	unsigned char		i_sock;
	spinlock_t		i_lock;	/* i_blocks, i_bytes, maybe i_size */
	struct semaphore	i_sem;
	struct rw_semaphore	i_alloc_sem;
	struct inode_operations	*i_op;
	struct file_operations	*i_fop;	/* former ->i_op->default_file_ops */
	struct super_block	*i_sb;
	struct file_lock	*i_flock;
	struct address_space	*i_mapping;
	struct address_space	i_data;
#ifdef CONFIG_QUOTA
	struct dquot		*i_dquot[MAXQUOTAS];
#endif
	/* These three should probably be a union */
	struct list_head	i_devices;
	struct pipe_inode_info	*i_pipe;
	struct block_device	*i_bdev;
	struct cdev		*i_cdev;
	int			i_cindex;

	__u32			i_generation;

#ifdef CONFIG_DNOTIFY
	unsigned long		i_dnotify_mask; /* Directory notify events */
	struct dnotify_struct	*i_dnotify; /* for directory notifications */
#endif

	unsigned long		i_state;
	unsigned long		dirtied_when;	/* jiffies of first dirtying */

	unsigned int		i_flags;

	atomic_t		i_writecount;
	void			*i_security;
	union {
		void		*generic_ip;
	} u;
#ifdef __NEED_I_SIZE_ORDERED
	seqcount_t		i_size_seqcount;
#endif
};

```

inode的操作如下:
```c
struct inode_operations {
	int (*create) (struct inode *,struct dentry *,int, struct nameidata *);
	struct dentry * (*lookup) (struct inode *,struct dentry *, struct nameidata *);
	int (*link) (struct dentry *,struct inode *,struct dentry *);
	int (*unlink) (struct inode *,struct dentry *);
	int (*symlink) (struct inode *,struct dentry *,const char *);
	int (*mkdir) (struct inode *,struct dentry *,int);
	int (*rmdir) (struct inode *,struct dentry *);
	int (*mknod) (struct inode *,struct dentry *,int,dev_t);
	int (*rename) (struct inode *, struct dentry *,
			struct inode *, struct dentry *);
	int (*readlink) (struct dentry *, char __user *,int);
	int (*follow_link) (struct dentry *, struct nameidata *);
	void (*put_link) (struct dentry *, struct nameidata *);
	void (*truncate) (struct inode *);
	int (*permission) (struct inode *, int, struct nameidata *);
	int (*setattr) (struct dentry *, struct iattr *);
	int (*getattr) (struct vfsmount *mnt, struct dentry *, struct kstat *);
	int (*setxattr) (struct dentry *, const char *,const void *,size_t,int);
	ssize_t (*getxattr) (struct dentry *, const char *, void *, size_t);
	ssize_t (*listxattr) (struct dentry *, char *, size_t);
	int (*removexattr) (struct dentry *, const char *);
};
```

主要说说lookup函数,这个函数用途是:在一个父目录中查找一个inode对象,.inode对应的文件名在dentry对象中.

####　索引节点的组织

内核也使用五个双向链表来管理索引节点:

* 超级块链
	每个分区的超级块通过这个链表来管理本分区上的所有索引节点,反映为s_inodes.
* 索引节点的hash表
	主要用来加速检索速度.
* 脏inode链
	需要回写到磁盘的索引节点.
* 空闲inode链
	上面的索引节点都是空闲的,不被任何进程使用.
* 正在使用的inode链
	正在被进程使用,并且不是脏inode

### file

文件对象是面向用户进程的.因为文件已经可以用之前的三个对象来管理了,super_block记录其所在的分区,dentry用来实现文件的搜索,别名和共享,inode负责调用驱动去完成磁盘读写操作.

文件对象主要维护这些信息:
* 文件是否被改动,即"脏"
* 文件指针,即是下一个读或写的位置

文件指针之所以在file中而不是在inode中,是因为不同进程可能同时访问一个文件,但是读的位置不同.

```c
struct file {
	struct list_head	f_list;
	struct dentry		*f_dentry;
	struct vfsmount         *f_vfsmnt;
	struct file_operations	*f_op;
	atomic_t		f_count;
	unsigned int 		f_flags;
	mode_t			f_mode;
	int			f_error;
	loff_t			f_pos;
	struct fown_struct	f_owner;
	unsigned int		f_uid, f_gid;
	struct file_ra_state	f_ra;

	size_t			f_maxcount;
	unsigned long		f_version;
	void			*f_security;

	/* needed for tty driver, and maybe others */
	void			*private_data;

#ifdef CONFIG_EPOLL
	/* Used by fs/eventpoll.c to link all the hooks to this file */
	struct list_head	f_ep_links;
	spinlock_t		f_ep_lock;
#endif /* #ifdef CONFIG_EPOLL */
	struct address_space	*f_mapping;
};
```

## 从进程角度看

### files_struct






