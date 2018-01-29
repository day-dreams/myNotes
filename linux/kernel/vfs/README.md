Linux 文件系统子系统

<!-- TOC -->

- [VFS对象及其数据结构](#vfs对象及其数据结构)
    - [文件对象(struct file)](#文件对象struct-file)
        - [结构定义](#结构定义)
        - [超级块操作](#超级块操作)
    - [目录项对象(struct dentry)](#目录项对象struct-dentry)
        - [结构定义](#结构定义-1)
        - [目录对象操作](#目录对象操作)
    - [文件对象(struct file)](#文件对象struct-file-1)
        - [结构定义](#结构定义-2)
        - [超级块操作](#超级块操作-1)
    - [超级块对象(struct super_block)](#超级块对象struct-super_block)
        - [结构定义](#结构定义-3)
        - [超级块操作](#超级块操作-2)

<!-- /TOC -->

# VFS对象及其数据结构

这里我认为应该自底向上学习,所以把顺序点倒过来看.

## 文件对象(struct file)

### 结构定义


### 超级块操作

## 目录项对象(struct dentry)

struct dentry的设计主要是为了路径查找.一个合法路径的每一个部分都对应一个目录项对象,每个目录项对象记录好自己的父目录对象,子目录对象列表,

### 结构定义

```c
struct dentry {
	atomic_t d_count;
	unsigned int d_flags;		/* protected by d_lock */
	spinlock_t d_lock;		/* 每个目录项目都有一个锁 */
	struct inode *d_inode;		/* Where the name belongs to - NULL is
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

### 目录对象操作

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
## 文件对象(struct file)

### 结构定义
### 超级块操作

## 超级块对象(struct super_block)

super_block代表一个具体的文件系统,它描述了一个被挂载到系统的文件系统的各种状态,操作方法,类型等.当系统挂载了一个文件系统,就必须为之创建一个super_block对象.

### 结构定义

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

### 超级块操作

```c


```