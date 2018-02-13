Linux内核的内存管理机制(二)之 地址空间和内存区域
===

本文属于内核学习系列中一段,主要描述Linux管理进程内存的机制.

本篇及上篇大致可以回答这个问题:**内核为用户创建进程时,如何分配内存?**.后续学习了malloc的实现机制,应该就可以全部回答了.

<!-- TOC -->

- [内核进程,用户进程访问内存的特点](#内核进程用户进程访问内存的特点)
- [地址空间](#地址空间)
- [进程地址空间,struct mm_struct](#进程地址空间struct-mm_struct)
	- [struct mm_struct](#struct-mm_struct)
	- [mm_struct如何被分配](#mm_struct如何被分配)
	- [mm_struct如何被销毁](#mm_struct如何被销毁)
- [内存区域,struct vm_area_struct](#内存区域struct-vm_area_struct)
	- [struct vm_area_struct](#struct-vm_area_struct)
	- [如何观察程序的内存区域分布](#如何观察程序的内存区域分布)
	- [相关基础函数](#相关基础函数)
	- [创建内存区域:内核函数do_mmap,系统调用mmap](#创建内存区域内核函数do_mmap系统调用mmap)
	- [释放内存区域:内核函数do_muumap,系统调用munmap](#释放内存区域内核函数do_muumap系统调用munmap)
- [reference](#reference)

<!-- /TOC -->
## 内核进程,用户进程访问内存的特点

Linux对于内核代码是充分信任的.

如果内核发起了对内存的申请,就必须马上回应,不应该有推迟.而用户进程访问内存的优先级较低,被认为是可以推迟的.

另外,内核访问内存的代码不设置任何防御机制,完全信任其正确性.而用户进程访问内存可能出现各种错误,比如缺页,越界等行为.

## 地址空间

地址空间(Address Space)这个词有很多歧义，必须结合语境来区分．

首先，**真实的地址空间**是连续平坦的，它指的是物理内存的地址分布．在4GB内存系统中,用户进程和内核各占据一定的内存.内核常驻在[0xC0000000,0xFFFFFFFF]部分,任何属于内核的内存页都会被映射到这部分,任何试图访问这部分内存的操作都必须在内核态中完成;剩下的[0x00000000,0xBFFFFFFF]由用户进程使用,用户进程的内存页一般被映射到这部分内存,用户进程不需要陷入内核,就可以直接访问.


**用户进程的地址空间**则特殊很多,它们虽然在逻辑上是连续平坦的,但映射到物理内存上,可能是分散在3GB的不同部分.用户进程的地址空间很大,甚至可以超过真实物理内存的地址空间.进程地址空间并不是全部有效的,部分地址区域可能并没有被使用,因而没有被映射到真实的物理内存.进程的地址空间也可以被

用户进程的地址空间含有多个内存区域.**内存区域**是用户进程可以合法访问的地址空间部分.合法访问有很多层意思:  

* 被访问的地址可能具有权限限制,比如代码段的地址区域是只读的,数据段的地址区域是不可执行的. 
* 被访问的地址可能不存在.比如用户进程访问0x00000000地址,会直接引起segment fault.

## 进程地址空间,struct mm_struct

### struct mm_struct

每个进程(struct task_struct)只有一个struct mm_struct,不同进程可能含有同一个struct mm_struct.比如一个程序开启的不同线程可能使用同一个mm_struct.

用户进程使用这个数据结构来表示和管理自己的地址空间．

内核进程则没有这个数据结构(task_struct->mm为空).**因为内核进程不访问用户进程的内存,没有用户上下文一说**.由于进程访问内存需要页表,内核进程又没有自己的地址空间,所以不得不使用上一个用户进程的地址空间,从而访问上个用户进程的页表.

```c
struct mm_struct {				
	struct vm_area_struct * mmap;		/* mmap指向内存区域结构体,每个结构体代表一段连续的属性相同的内存区域, */
	struct rb_root mm_rb;			/* mmap本身以链表的形式组织,但当内存区域过多时,链表效率低下,转而使用红黑树来组织 */
	struct vm_area_struct * mmap_cache;	/* last find_vma result */
	unsigned long (*get_unmapped_area) (struct file *filp,
				unsigned long addr, unsigned long len,
				unsigned long pgoff, unsigned long flags);
	void (*unmap_area) (struct vm_area_struct *area);
	unsigned long mmap_base;		/* base of mmap area */
	unsigned long free_area_cache;		/* first hole */
	pgd_t *pgd;				/* 用户进程自己的页表.内核进程虽然没有mm_struct,但会直接使用上一个用户进程的页表 */
	atomic_t mm_users;			/* 进程间可以共享一段内存空间,这个字段表示有多少进程使用了这个地址空间 */
	atomic_t mm_count;			
	int map_count;				/* 这个内存空间的内存区域个数 */
	struct rw_semaphore mmap_sem;
	spinlock_t page_table_lock;		
	struct list_head mmlist;	
	unsigned long start_code, end_code, start_data, end_data;
	unsigned long start_brk, brk, start_stack;
	unsigned long arg_start, arg_end, env_start, env_end;
	unsigned long rss, anon_rss, total_vm, locked_vm, shared_vm;
	unsigned long exec_vm, stack_vm, reserved_vm, def_flags, nr_ptes;
	unsigned long saved_auxv[42]; /* for /proc/PID/auxv */
	unsigned dumpable:1;
	cpumask_t cpu_vm_mask;
	mm_context_t context;
	unsigned long swap_token_time;
	char recent_pagein;
	int core_waiters;
	struct completion *core_startup_done, core_done;
	rwlock_t		ioctx_list_lock;
	struct kioctx		*ioctx_list;
	struct kioctx		default_kioctx;
	unsigned long hiwater_rss;	/* High-water RSS usage */
	unsigned long hiwater_vm;	/* High-water virtual memory usage */
};
```
### mm_struct如何被分配

新进程产生只通过类fork函数来完成.产生子进程时,分两种情况:
* 共享地址空间
	这样的子进程实际上是个线程,由于线程间共享地址空间,所以直接把current->mm赋值给子进程的task_struct->mm.这时,mm_struct->mm_users需要加一.
* 不共享地址空间
	这种才是一般意义上的子进程.内核通过调用allocate_mm()函数,从mm_cachep slab缓存(回忆上一篇提到的slab机制)中获得.

### mm_struct如何被销毁

当进程推出,一般会执行exit函数.exit函数中又含有exit_mm函数,这个函数会更新地址空间的引用计数(mm_struct->mm_users),当引用计数为0,就会真正销毁这个地址空间.

销毁的过程通过free_mm()宏完成,free_mm宏会从mm_cachep slab中回收这个mm_struct.

```c
#define free_mm(mm)	(kmem_cache_free(mm_cachep, (mm)))/* 销毁地址空间.只有地址空间的引用计数为0才调用 */
```

## 内存区域,struct vm_area_struct

### struct vm_area_struct

内核使用vm_area_struct表示一段连续的内存区域,每个区域内有若干个页,页具有相同的属性(各种权限相同,访问操作相同).有的用来表示用户内存栈,有的表示代码段,数据段等.

内存区域有两种组织方式:链表和红黑树.mm_struct中有相关的数据域.

```c
struct vm_area_struct {			/* 代表一个内存区域 */
	struct mm_struct * vm_mm;	/* The address space we belong to. */
	unsigned long vm_start;		/* 起始地址 */
					/* Our start address within vm_mm. */
	unsigned long vm_end;		/* 结束地址 */
					/* The first byte after our end address
					   within vm_mm. */

	/* linked list of VM areas per task, sorted by address */
	struct vm_area_struct *vm_next; /* 链表相关 */

	pgprot_t vm_page_prot;		/* 访问权限 */
					/* Access permissions of this VMA. */
	unsigned long vm_flags;		/* Flags, listed below. */

	struct rb_node vm_rb;		/* 红黑树相关 */

	/*
	 * For areas with an address space and backing store,
	 * linkage into the address_space->i_mmap prio tree, or
	 * linkage to the list of like vmas hanging off its node, or
	 * linkage of vma in the address_space->i_mmap_nonlinear list.
	 */
	union {
		struct {
			struct list_head list;
			void *parent;	/* aligns with prio_tree_node parent */
			struct vm_area_struct *head;
		} vm_set;

		struct raw_prio_tree_node prio_tree_node;
	} shared;

	/*
	 * A file's MAP_PRIVATE vma can be in both i_mmap tree and anon_vma
	 * list, after a COW of one of the file pages.  A MAP_SHARED vma
	 * can only be in the i_mmap tree.  An anonymous MAP_PRIVATE, stack
	 * or brk vma (with NULL file) can only be in an anon_vma list.
	 */
	struct list_head anon_vma_node;	/* Serialized by anon_vma->lock */
	struct anon_vma *anon_vma;	/* Serialized by page_table_lock */

	struct vm_operations_struct * vm_ops;	/* 相关操作表 */

	/* Information about our backing store: */
	unsigned long vm_pgoff;		/* Offset (within vm_file) in PAGE_SIZE
					   units, *not* PAGE_CACHE_SIZE */
	struct file * vm_file;		/* File we map to (can be NULL). */
	void * vm_private_data;		/* was vm_pte (shared mem) */
	unsigned long vm_truncate_count;/* truncate_count or restart_addr */

#ifndef CONFIG_MMU
	atomic_t vm_usage;		/* refcount (VMAs shared if !MMU) */
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *vm_policy;	/* NUMA policy for the VMA */
#endif
};
```

### 如何观察程序的内存区域分布

```bash
cat /proc/进程号/maps
```

```bash
$ cat /proc/4457/maps

00400000-00401000 r-xp 00000000 08:09 11144035 home/moon/workplace/myNotes/linux/kernel/mmu/test/a.out 
00600000-00601000 r--p 00000000 08:09 11144035 home/moon/workplace/myNotes/linux/kernel/mmu/test/a.out
00601000-00602000 rw-p 00001000 08:09 11144035  home/moon/workplace/myNotes/linux/kernel/mmu/test/a.out
7fb73aff4000-7fb73b1b4000 r-xp 00000000 08:0a 2228439                    /lib/x86_64-linux-gnu/libc-2.23.so
7fb73b1b4000-7fb73b3b4000 ---p 001c0000 08:0a 2228439                    /lib/x86_64-linux-gnu/libc-2.23.so
7fb73b3b4000-7fb73b3b8000 r--p 001c0000 08:0a 2228439                    /lib/x86_64-linux-gnu/libc-2.23.so
7fb73b3b8000-7fb73b3ba000 rw-p 001c4000 08:0a 2228439                    /lib/x86_64-linux-gnu/libc-2.23.so
7fb73b3ba000-7fb73b3be000 rw-p 00000000 00:00 0 
7fb73b3be000-7fb73b3e4000 r-xp 00000000 08:0a 2228435                    /lib/x86_64-linux-gnu/ld-2.23.so
7fb73b5c0000-7fb73b5c3000 rw-p 00000000 00:00 0 
7fb73b5e3000-7fb73b5e4000 r--p 00025000 08:0a 2228435                    /lib/x86_64-linux-gnu/ld-2.23.so
7fb73b5e4000-7fb73b5e5000 rw-p 00026000 08:0a 2228435                    /lib/x86_64-linux-gnu/ld-2.23.so
7fb73b5e5000-7fb73b5e6000 rw-p 00000000 00:00 0 
7ffd15b3b000-7ffd15b5c000 rw-p 00000000 00:00 0                          [stack]
7ffd15b76000-7ffd15b79000 r--p 00000000 00:00 0                          [vvar]
7ffd15b79000-7ffd15b7b000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]

```

### 相关基础函数

这部分函数用于实现do_map()和do_unmap().

```c
/* 查找并返回第一个终止地址大于addr的内存区域;如果没有则返回NULL */
extern struct vm_area_struct * find_vma(struct mm_struct * mm, unsigned long addr);

/* 查找并返回第一个终止地址小于addr的内存区域;如果没有则返回NULL */
extern struct vm_area_struct * find_vma_prev(struct mm_struct * mm, unsigned long addr,
					     struct vm_area_struct **pprev);

/* 查找并返回地一个与给定区间交叉的内存区域;如果没有则返回NULL */
static inline struct vm_area_struct * find_vma_intersection(struct mm_struct * mm, unsigned long start_addr, unsigned long end_addr)
{
	struct vm_area_struct * vma = find_vma(mm,start_addr);

	if (vma && end_addr <= vma->vm_start)
		vma = NULL;
	return vma;
}
```

### 创建内存区域:内核函数do_mmap,系统调用mmap

当新进程建立,内核要为止创建地址空间(struct mm_struct),并创建内存区域(struct vm_area_struct)来存放一些数据(比如代码段,数据段,bss段等),最终把内存区域填充到地址空间中.

do_mmap的实现细节很复杂,不考虑映射磁盘文件时,主要完成这些工作:
1. 检查参数  
2. 调用get_unmapped_area(),查找一个可以使用的内存区域  
3. 通过传入的参数来计算内存区域的状态.此时,**内存区域没有创建,而且没有为之分配内存页**  
4. 调用find_vma_prepare(),查找新的的内存区域在红黑树中的位置,以及前一个内存区域的位置  
5. 检查新的内存区域是否会引起内存超出进程限制.如果超出,撤销创建并返回错误码  .
6. 检查剩余的空闲内存页是否够用.新的内存区域是否包含私有可写的区域,并且没有足够的空闲页框为这些区域提供映射.即剩余内存不足以支撑私有可写区域.如果是,返回出错码.  
7. 如果新的内存区域是私有的,并且不映射到磁盘文件,检查这个新的内存区域是否可以合并到前一个内存区域.如果可以,直接调用vma_merge()合并之,并跳转到11.
8. 使用slab机制,为vma_area_struct分配一块内存,使用计算好的标志位来初始化之.此时,**内存区域创建成功**.
9. 如果新的内存区域是个匿名共享区,即用于进程通信,那么调用shmem_zero_setup()来初始化之.
10. 调用vma_link(),把新的vma_area_struct结构插入到链表和红黑树中.
11. 增加task_struct中的内存大小字段.
12. 如果新的内存区域设置了VM_LOCKED,即被锁在内存而不能被换出,那么直接为它分配内存页.
13. 创建完毕.

**注意,如果内存区域没有设置VM_LOCKED,内核并没有直接为之分配内存页.内存页的分配将推迟到进程访问这部分内存而引起的缺页中断处理中**.

缺页中断的处理留到其他部分再学习.

```c
/* 
 * 创建一个内存区域,这个内存区域可能要映射磁盘文件的某些区域(比如代码段,数据段)
 *
 * file:	需要映射的文件,如果有的话
 * addr:	从何处开始查找空闲的内存区域
 * len:		需要多大的内存区域
 * prot:	访问权限,分配的内存区域含有的所有页具有相同的权限性质
 * flag:	一些标志,是否有文件跟这个内存区域关联等
 * offset:	从文件何处开始映射
 * */
static inline unsigned long do_mmap(struct file *file, unsigned long addr,
	unsigned long len, unsigned long prot,
	unsigned long flag, unsigned long offset)
{
	unsigned long ret = -EINVAL;
	if ((offset + PAGE_ALIGN(len)) < offset)
		goto out;
	if (!(offset & ~PAGE_MASK))
		ret = do_mmap_pgoff(file, addr, len, prot, flag, offset >> PAGE_SHIFT);
out:
	return ret;
}
```

do_mmap()是个内核函数,只供内核使用.如果用户进程需要调用这样的功,需要使用mmap函数.mmap内部包装了do_mmap,是个系统调用.可以看到,mmap具有相似的函数,并使用文件描述符fd来表示文件.
```c
void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);

```


### 释放内存区域:内核函数do_muumap,系统调用munmap

```c
/* 销毁某地址空间中的某个内存区域,指定内存区域的起始地址和大小 */
extern int do_munmap(struct mm_struct *, unsigned long, size_t);

/* 相关系统调用 */
int munmap(void *addr, size_t length);

```

## reference


* [Process Address Space](https://www.kernel.org/doc/gorman/html/understand/understand007.html)　　
* linux-2.6.11.1

* Linux内核设计与实现,第三版  
