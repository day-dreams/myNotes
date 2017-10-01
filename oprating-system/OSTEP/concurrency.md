Operating Systems: Three Easy Pieces
=====================================

这是OSTEP的并发部分章节的笔记.

# chapter 26, Concurrency: An Introduction

## threads

简单介绍下线程.

线程与进程十分相似,都是一种对CPU的抽象.相比与进程,线程更加轻盈.比如,同一个进程内的线程发生切换时,不必切换PageTable,也不必切换PageLookasideBuffer.线程也是调度器的工作对象.同一个进程内,所有的线程共享同一个code,stack段,但各自拥有独立的stack.与process的PCB相似,thread具有TCB.

需要强调的是,线程的运行顺序没有任何规定,完全取决与调度器的调度算法及当时的工作状态.这里有一个[例子](./examples/thread-schedule).

## Shared Data

thread 虽然带来了一种更轻便的CPU虚拟化,但也带来了一些问题.最明显的是Shared Data问题.

由于调度的先后顺序未知,如果几个线程同时对一个变量进行裸露的读写操作,很可能由于读写顺序的打乱而造成结果错误的情况.

读写顺序的打乱是由于线程的读写操作未完成就用玩了时间片,丢失cpu控制权.以简单的 ```count=count+1``` 为例子,汇编如下:
```as
  40070e:       8b 05 40 09 20 00       mov    0x200940(%rip),%eax        # 601054 <x>
  400714:       83 c0 01                add    $0x1,%eax
  400717:       89 05 37 09 20 00       mov    %eax,0x200937(%rip)        # 601054 <x>
```

## Atomicity

如果每一次读写都不被打断,shared data带来的问题就可以避免.我们需要一种保证操作能够完全完成,或根本就不执行的属性:原子性.下面在介绍几个重要的名词,他们都是由Dijkstra提出来的,与原子性有关.

* critical section  
    临界区.临界去区是一段代码/指令,这段代码试图访问共享变量(shared variable).显然,这样的代码不能并发执行.
* mutual exclusion  
    互斥.互斥是一种性质,它保证统一时间,无论有多少个线程正在执行,都只能有一个线程处于临界区.
* race condition  
    竞态条件.当多个线程试图同时进入临界区,就发生了竞态条件.一般会导致操作失败,结果不正确.
* synchronization primitives  
    同步原语.由硬件和操作系统共同提供的机制,确保代码的原子性.


