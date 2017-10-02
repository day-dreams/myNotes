Operating Systems: Three Easy Pieces
=====================================

这是OSTEP的并发部分章节的笔记.

<!-- TOC -->

- [1. chapter 26, Concurrency: An Introduction](#1-chapter-26-concurrency-an-introduction)
    - [1.1. threads](#11-threads)
    - [1.2. Shared Data](#12-shared-data)
    - [1.3. Atomicity](#13-atomicity)
- [2. Interlude: Thraed API](#2-interlude-thraed-api)
- [3. Locks](#3-locks)
    - [3.1. Locks:The Basic Idea](#31-locksthe-basic-idea)
    - [3.2. Evaluating Locks](#32-evaluating-locks)
    - [3.3. 第一种实现:Contorlling Interrupts](#33-第一种实现contorlling-interrupts)
    - [3.4. 第二种实现:SpinLock,Test and Set(Atomic Exchange)](#34-第二种实现spinlocktest-and-setatomic-exchange)
    - [3.5. 第三种实现:SpinLock,Compare and Swap](#35-第三种实现spinlockcompare-and-swap)
    - [3.6. 第四种实现:TicketLock,Fetch and Add](#36-第四种实现ticketlockfetch-and-add)
    - [3.7. 第五种实现:Using Queues,Sleeping Instead Of Spinning(Solaris,Linux)](#37-第五种实现using-queuessleeping-instead-of-spinningsolarislinux)
    - [3.8. 第六种实现:Two Phase Locks(Linux)](#38-第六种实现two-phase-lockslinux)

<!-- /TOC -->

# 1. chapter 26, Concurrency: An Introduction

## 1.1. threads

简单介绍下线程.

线程与进程十分相似,都是一种对CPU的抽象.相比与进程,线程更加轻盈.比如,同一个进程内的线程发生切换时,不必切换PageTable,也不必切换PageLookasideBuffer.线程也是调度器的工作对象.同一个进程内,所有的线程共享同一个code,stack段,但各自拥有独立的stack.与process的PCB相似,thread具有TCB.

需要强调的是,线程的运行顺序没有任何规定,完全取决与调度器的调度算法及当时的工作状态.这里有一个[例子](./examples/thread-schedule).

## 1.2. Shared Data

thread 虽然带来了一种更轻便的CPU虚拟化,但也带来了一些问题.最明显的是Shared Data问题.

由于调度的先后顺序未知,如果几个线程同时对一个变量进行裸露的读写操作,很可能由于读写顺序的打乱而造成结果错误的情况.

读写顺序的打乱是由于线程的读写操作未完成就用玩了时间片,丢失cpu控制权.以简单的 ```count=count+1``` 为例子,汇编如下:
```as
  40070e:       8b 05 40 09 20 00       mov    0x200940(%rip),%eax        # 601054 <x>
  400714:       83 c0 01                add    $0x1,%eax
  400717:       89 05 37 09 20 00       mov    %eax,0x200937(%rip)        # 601054 <x>
```

## 1.3. Atomicity

如果每一次读写都不被打断,shared data带来的问题就可以避免.我们需要一种保证操作能够完全完成,或根本就不执行的属性:原子性.下面在介绍几个重要的名词,他们都是由Dijkstra提出来的,与原子性有关.

* critical section  
    临界区.临界去区是一段代码/指令,这段代码试图访问共享变量(shared variable).显然,这样的代码不能并发执行.
* mutual exclusion  
    互斥.互斥是一种性质,它保证统一时间,无论有多少个线程正在执行,都只能有一个线程处于临界区.
* race condition  
    竞态条件.当多个线程试图同时进入临界区,就发生了竞态条件.一般会导致操作失败,结果不正确.
* synchronization primitives  
    同步原语.由硬件和操作系统共同提供的机制,确保代码的原子性.


# 2. Interlude: Thraed API

全是基本的api使用,跳过本章大部分内容.只留下一个Thread API Guidelines,这里摘录几条.

* Keep It Simple and Stupid.
* 减少线程间的交互!
* 尽量减少共享变量的使用!

# 3. Locks

这一章主要是关于同步锁及其实现.

## 3.1. Locks:The Basic Idea

到目前,我们没有任何用来干预进程或线程调度(以下统一称为线程调度)的工具,也就是说,线程调度完全取决与调度器(scheduler).谁先执行,执行多就,全部又scheduler根据历史数据和当前环境来选择.

这样一来,就有了上一章中的data race的问题,也就是涉及到同步和互斥的问题.我们认为,线程之间有三种关系:
* 同步  
    线程之间存在依赖关系,如输入线程和计算线程和输出线程.
* 互斥  
    多个线程试图进入同一个临界区,会发生data race.
* 没关系
    完全不存在关系.

最后一种忽略不计,我们需要一种来控制线程间同步互斥关系的工具,也就是**锁**.当线程要进入临界区,必须先打开一把(也可能是多把)锁,如果锁打不开,就不能进入.当然,锁只能被打开一次,直到打开锁的线程重新施放了锁,这把锁才能被重新打开.锁(Lock)只是借用了我们生活中常说的锁的概念而已,工作原理则不同.Lock一般具有两种操作:acquire/release,也叫做lock/unlock.

这里也不多说,锁用的已经够多了,记下这么多就够了.

## 3.2. Evaluating Locks

从现在开始,我们主要集中与如何实现锁这个特别的数据结构.先介绍三种衡量锁的角度:

* Can provide mutaul exclusion or not?  
    是否能保证所有线程只能有一个进入临界区?
* Can promise for fairness or not?  
    是否可以保证所有线程都能进入临界区,而不造成某些线程一直处在starving的状态?
* How about performence?  
    lock和unlock的操作大概要占多长时间?

## 3.3. 第一种实现:Contorlling Interrupts

如果是在单核CPU环境下,要想保证mutual exclusion,就必须保证其他线程不会抢走cpu.我们知道,线程调度是通过timeout的形式触发中断,将控制权还给OS,OS再选择下个线程来实现的.如果timeout中断不被处理,就不会发生线程切换了.所以,我们只需要在线程进入临界区前,屏蔽终端.核心思想如下图:

![](./lock-by-mask-interrupts.png)

这种方式的缺点很多.首先,只能在单CPU下工作,显然在如今是不合适的;其次,关中断的操作需要更高的权限,所以需要一种特别的提权机制,来保证关终端开中断的顺利进行;最后,据说对于开关中断的操作,执行起来会比较慢.

## 3.4. 第二种实现:SpinLock,Test and Set(Atomic Exchange)

现在,我们添加一个标志位,来反映锁的工作状态.首先看看初步的Test ans Set算法.
```c
typedef struct __lock_t {
    int flag
} lock_t;

void init(lock_t *mutex){
    mutex->flag=0;
}

void lock(lock_t *mutex){
    while(mutex->flag==1) 
        ;
    mutex->flag=1;
}

void unlock(lock_t *mutex){
    mutex->flag=0;
}

``` 

可以看到,在lock的时候,如果锁已经被占用,lock()就会陷入循环,直到有人释放了这个锁.这种循环状态成为**spin-wait**,尽管线程在这个地方循环,相当与一种阻塞,但线程并没有让出CPU,而是占用CPU不断检查flag状态.

同时也有一个很大的问题:两个线程可能同时跳出循环(初始状态,两个线程通过了test,要进入临界区).这还是由于那个根本问题,CPU被抢占,指令操作未完成.所以,我们还需要一种硬件支持,来保证这个lock操作是原子性的.

很多硬件提供了这样的原子性指令,如x86下的xchg,SPARC下的ldstub,都统一叫做**test-and-set**,工作细节如下:

```c
int TestAndSet(int *ptr,int new){//这个函数是原子性的,不可被中断
    int old=*ptr;
    *ptr=new;
    return old;
}
```
test-and-set指令完成了test和set的功能,如果锁已经被占用,set仍然成功,但test得到的值却依然是旧值.使用test-and-set的lock()大致如下:
```c
void lock(lock_t *mutex){
    while(TestAndSet(&mutex->flag,1)==1)
        ;
}
```

这种锁称为自旋锁(**spin lock**).显然,这个lock可以保证mutual exclusion,但是却不能保证fairness和performence.

## 3.5. 第三种实现:SpinLock,Compare and Swap

一些硬件还提供另外的支持,也就是compare-and-swap.工作细节如下:

```c
int CompareAndSwap(int *ptr,int expected,int new){//这个函数同样是原子性的
    int actual=*ptr;
    if (actual==expected)
        *ptr=new;
    return actual;
}
```
## 3.6. 第四种实现:TicketLock,Fetch and Add

硬件提供这样的指令支持:
```c
int FetchAndAdd(int *ptr){//原子性
    int old=*ptr;
    *ptr=old+1;
    return old;
}
```

锁的机制变了很多,lock内有一对turn和ticket,分别表示当前轮到的号码,以及当前发放到的门票号码.当轮到的号码经过unlock增长后,ticket与之相等的lock()即可获得锁.

```
typedef struct __lock_t {
    int turn;
    int ticket;
} lock_t;

void lock(lock_t* mutex){
    int myturn=FetchAndAdd(&mutex->ticket);//领取门票,门票记录自动增长
    while(myturn!=mutex->turn)
        ;
    return
}

void unlock(lock_t* mutex){
    FetchAndAdd(&mutex->turn);
}

```

这样的算法好在,保证fairness:**每个试图获得锁的线程,最终都能够获得之**.但性能方面并没有什么改变:当一个线程获得了锁,其他线程试图lock()时,会陷入到spin-wait中.

## 3.7. 第五种实现:Using Queues,Sleeping Instead Of Spinning(Solaris,Linux)

为了避免spin-wait,线程应该自己申请进入等待状态(Block),获得锁的线程释放锁时,再将它唤醒.当然,不能采用简单的sleep(n),因为等待的时间不可控,并且太多线程一起醒来还是会造成类似的spin-wait.所以需要一种系统调用,能让线程主动进入block,而且能够被动唤醒.

Solaris有一对系统调用:park(),unpark(threadID).使用这对API(保证performance),在加上队列(保证fairness),以及原子性指令(TestAndSet,保证互斥),就可以完成一个比较好的lock实现方式.具体细节如下:

```c
typedef struct __lock{
    int flag;
    int guard;
    queue *q;
}lock_t;

void init(lock_t *mutex){
    mutex->flag=0;
    mutex->guard=0;
    queue_init(mutex->q);
}

void lock(lock_t *mutex){
    while(TestAndSet(&mutex->guard,1)==1) //如果不能获的guard的锁,即有线程正在修改队列q,应该自旋等待
        ;

    //获得了guard锁,可以修改队列了
    if(mutex->flag==0){//锁空闲,可直接获得
        mutex->flag=1;
        mutex->q=0; //释放队列锁
        return;
    }else{//锁被其他线程获得,应该陷入block,等待唤醒
        queue_add(mutex->q,gettid());
        mutex->q=0;//释放队列锁
        pack();
    }
}

void unlock(lock_t *mutex){
    while(TestAndSet(&mutex->guard,1)==1)
        ;
    if(queue_empty(mutex->q)){//没有其他线程试图获得这个锁
        mutex->flag=0;
    }else{//有其他线程因为或的这个锁而被pack,需要唤醒
        unpack(queue_remove(mutex->q));
    }
    mutex->guard=0;

}

```

这里分别解释下lock和unlock的细节.  
    guard是队列锁,只有获得了队列锁,才能进入下一步lock,否则陷入spin-wait.好在对于队列的操作耗时较短,不会产生大量的spin-wait现象.  
    
有个致命的地方:lock中,pack()之前可能产生race condition.比如,当前线程要pack了,队列锁也已经施放,却用玩了time slice,CPU被其他线程抢占.如果这个新上来的线程,执行了unlock操作,而队列中又只有上个线程,那么上个线程被unpack,同时锁被标记为realease,队列清空.然后等上个线程获得了CPU,继续从pack()开始执行,一进入pack,就不可能再被唤醒了(锁已经被标记为realease,下个试图获得锁的线程将直接获得锁),与是永远陷入pack.

当然,Soloris也有相应的解决办法.它用setpack代替pack,使线程pack之前先看看,是不是已经没有其他线程占用锁,如果是,就直接恢复正常执行状态,重新调用lock,获得锁.

Linux也有类似的机制,比如futex_wait,futex_wake.这个机制在```lowlevellock.h```中有体现.值得一提的是,futex.h等相关文件中,大量使用汇编来实现锁,为的就是减小中断的影响.Linux源代码中也有spinlock.c,有空可以看看[kernel/locking/spinlock.c](https://github.com/torvalds/linux/blob/master/kernel/locking/spinlock.c),里面太多的宏定义,还涉及到多核CPU的情形.

## 3.8. 第六种实现:Two Phase Locks(Linux)

这种锁对于spin-wait的理解不同,它认为spin-wait是有用的.直接避免spin-wait是不对的,相反,应该先尝试spin-wait一段时间,再进入block状态.

Linux的锁将loxk分为两段时期.第一段只是进入spin-wait,预定时间内还没获得锁就退出第一段,再次检查是否可以或的锁,如果可以就返回,否则进入第二段,陷入pack,等待被唤醒.