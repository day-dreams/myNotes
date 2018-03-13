mmap/munmap详解
===

之前的笔记里虽然提到了mmap(),但那实在学习内核的初期,效果不是很好,感觉还没有弄清楚,所以这次回头再来研究一下.

## mmap

mmap用于分配一个内存区域(memory region),这个内存区域可以映射到文件,起到随机读取文件的效果;也可以不映射到文件,而是供进程间共同使用,达到内存共享的效果.

```c
void *mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
```

还是先翻译翻译man page上的说明吧.

mmap在用户进程的虚拟内存空间上划出一块内存区域.

* `addr`用于指示内存区域的地址,linux会尽量分配出与addr接近的内存区域,如果没有指定addr,就任内核发挥了.
* `length`指定要映射的内存区域的大小.因为内存基本单位是页,所以需要页对齐.如果length不是4KB的整数倍,内核会自动扩充.不过,超出length的那部分内存是不可访问的.
* `prot`控制进程对内存区域的访问权限:可读,可写,可执行,不可访问.
* `flags`用于控制两个东西:如果多个进程同时映射同一片内存,进程对内存的修改是不是对其它进程可见;如果是映射到文件,进程对文件的修改是否需要反映到磁盘上的文件实体.这个标志还可以指示内核去创建一个匿名区域,不需要映射到文件.
* `fd`和`offset`主要在文件映射时起作用.如果是文件映射,新创建的内存区域使用fd指向文件的内容来初始化,offset必须是`sysconf(_SC_PAGE_SIZE)`的整数倍.

## munmap

```c
int munmap(void *addr, size_t length);
```

munmap用于解除进程内部分地址空间的映射.其实就是从`task_struct->active_mm->mmap`里删除一个`struct vm_area_struct`.解除之后,这部分内存区域可以用于其他用途,比如被malloc申请用于用户进程内存分配.

注意,关闭文件描述符并不会解除映射.


[这里](test/mmap)有使用例子.

## mmap实现原理

mmap()主要是调用了do_mmap2().

do_mmap2先做一点粗略的检查:如果用户不是创建匿名区,就需要关联到文件,所以会尝试在文件表里拿到文件对象.接下来调用do_mmap_pgoff,其他所有操作都在这个函数里.

我不是太想记录do_mmap_pgoff这个函数,因为内部涉及到很多vma相关的操作,比如合并,分解,查找,删除等.这里只记录do_mmap_pgoff的大概操作,细节可以看我在源码里的注释.

do_mmap_pgoff先做些检查,如:进程是不是有太多的mmap,可用区域是不是溢出到了内核空间.

随后从进程地址空间里搜索出一个可用的区域[addr,addr_offset),并开始根据用户传入的标志和是不是匿名文件映射来设置vma_flags.

接着就是检查这个内存区域是不是和其它有重叠,如果有,直接报错.

在然后又是一些检查,这里开始检查新的内存区域会不会导致进程使用过多的内存.

检查完毕后,从slba机制里申请一个vma_area_struct,并初始化.如果要映射到文件,就调用文件对象的mmap方法去处理.

最后,对两种情况分别进行处理:
* 是不是vm_locked,如果是,要立即分配物理页,否则推迟到页中断.
* 是不是map_populate,如果是,要采取预读措施,尽量减少阻塞延迟.


```c
static inline unsigned long
do_mmap2(unsigned long addr, unsigned long len, unsigned long prot,
        unsigned long flags, unsigned long fd, unsigned long pgoff);

		
unsigned long do_mmap_pgoff(struct file * file, unsigned long addr,
			unsigned long len, unsigned long prot,
			unsigned long flags, unsigned long pgoff);
```

### munmap()实现原理

## reference

[认真分析mmap：是什么 为什么 怎么用](https://www.cnblogs.com/huxiao-tee/p/4660352.html)