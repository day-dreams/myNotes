Operating Systems: Three Easy Pieces
=====================================
<!-- TOC -->

- [1. CPU Virtualization](#1-cpu-virtualization)
    - [1.1. chapter 4, The Abstraction: The Process](#11-chapter-4-the-abstraction-the-process)
        - [1.1.1. 什么是进程](#111-什么是进程)
        - [1.1.2. 用于控制进程的API](#112-用于控制进程的api)
        - [1.1.3. 创建一个进程需要哪些工作](#113-创建一个进程需要哪些工作)
            - [1.1.3.1. 将程序装载到内存](#1131-将程序装载到内存)
            - [1.1.3.2. 分配堆栈内存](#1132-分配堆栈内存)
            - [1.1.3.3. 其他初始化工作](#1133-其他初始化工作)
        - [1.1.4. 进程状态](#114-进程状态)
        - [1.1.5. 进程需要的数据结构](#115-进程需要的数据结构)
    - [1.2. chapter 5, Interlude: Process API](#12-chapter-5-interlude-process-api)
        - [1.2.1. fork()](#121-fork)
        - [1.2.2. wait()](#122-wait)
        - [1.2.3. exec()](#123-exec)
        - [1.2.4. 神奇的组合效果](#124-神奇的组合效果)
    - [1.3. chapter 6, Mechanism: Limited Direct Execution](#13-chapter-6-mechanism-limited-direct-execution)
        - [1.3.1. Direct Execution Protocol](#131-direct-execution-protocol)
        - [1.3.2. Limited Direction Execution Protocol](#132-limited-direction-execution-protocol)
        - [1.3.3. Switching Between Processes](#133-switching-between-processes)
        - [1.3.4. 为什么系统调用开销大](#134-为什么系统调用开销大)
    - [1.4. chapter 7, Scheduling:Introduction](#14-chapter-7-schedulingintroduction)
        - [1.4.1. 一些衡量标准](#141-一些衡量标准)
        - [1.4.2. FIFO](#142-fifo)
        - [1.4.3. Shortest Job First(SJF)](#143-shortest-job-firstsjf)
        - [1.4.4. Shortest Time-to-Completion First(STCF)](#144-shortest-time-to-completion-firststcf)
        - [1.4.5. Round Robin](#145-round-robin)
        - [1.4.6. 考虑IO](#146-考虑io)
    - [1.5. chapter 8, Scheduling: The Multi-Level Feedback Queue](#15-chapter-8-scheduling-the-multi-level-feedback-queue)
    - [1.6. chapter 9, Scheduling: Proportional Share](#16-chapter-9-scheduling-proportional-share)
    - [1.7. chapter 10, Multiprocessor Scheduling](#17-chapter-10-multiprocessor-scheduling)
- [2. Memory VIrtualization](#2-memory-virtualization)
    - [2.1. chapter 13, The Abstraction:Address Spaces](#21-chapter-13-the-abstractionaddress-spaces)
    - [2.2. chapter 14, Interlude: Memory APU](#22-chapter-14-interlude-memory-apu)
        - [2.2.1. malloc和free](#221-malloc和free)
        - [2.2.2. mmap(占坑)](#222-mmap占坑)
    - [2.3. chapter 15, Mechanism: Address Translation](#23-chapter-15-mechanism-address-translation)
        - [2.3.1. base and bounds](#231-base-and-bounds)

<!-- /TOC -->

# 1. CPU Virtualization

CPU虚拟化是为了营造一种假象:**用户交给操作系统的任务能够同时运行**.为了营造这种假象,我们不得不引入程序运行的基本单位:进程,以及进程的调度问题.

## 1.1. chapter 4, The Abstraction: The Process

### 1.1.1. 什么是进程
首先我们要明白什么是进程.进程操作系统对运行中的程序的一种抽象.

那么,对于进程执行来说,有那些重要的操作系统部分?

* **内存**:用于存放指令和部分数据
* **寄存器**:用于存放进程的另一部分数据,特殊的有,PC,StackPointer等
* **存储设备**:一些进程正在访问的I/O文件

### 1.1.2. 用于控制进程的API

既然是一种抽象,就因该有接口来管理这层抽象.我们一般需要这些接口:
* **Create**:创建一个进程
* **Destroy**:销毁\撤销一个进程
* **Wait**:等待一个进程结束
* **Miscellaneous Control**:各种控制方法,如挂起和激活
* **Status**:查看一个进程的状态

### 1.1.3. 创建一个进程需要哪些工作

#### 1.1.3.1. 将程序装载到内存

装载包括指令和数据两个部分.程序首先以某种可执行文件(ELF,EXE)的形式存在与磁盘上,操作系统可以读取它们并写入内存。

这里的装载还包括两种形式:lazy装载,eagerly装载．

eagerly装载就简单的把所有程序指令都装入内存,无论是否真的需要所有的指令.这往往是早期操作系统.

lazy装载则不同,只放入一些重要片段.这往往与分页机制有关,暂不研究.

#### 1.1.3.2. 分配堆栈内存

进程必须具有一个独立的栈.这个栈往往用来存放临时变量,同时具有最大空间限制(例如8M).

还有堆内存.例如C程序,有静态全局变量这样的东西.同时还有一些动态分配的内存需要堆来负责.所以,这个堆一开始往往较小,随着程序运行而增长.

#### 1.1.3.3. 其他初始化工作

还需要做一些IO方面的初始化.

如Linux中,每个进程都默认关联stderr,stdout,stdin三个文件描述符.

当一些准备好了,操作系统需要将进程的执行起点调至main函数,进入就需状态,等待合适的CPU时间片分配给进程.

### 1.1.4. 进程状态

共同的状态有：

* Ready
* Running
* Block

还有一些OS提供更多的状态：

* 创建
* 结束
* 挂起
* 僵死
* 跟踪：这个状态存在与一些调试情景，debugger将进程暂停，用于调试。

### 1.1.5. 进程需要的数据结构

为了管理,调度,跟踪进程,操作系统需要一些特定的数据结构来记录进程的信息.一般包括Processlist和PCB.

对于一个进程,操作系统需要记录它的信息,即PCB的组织:

* 寄存器上下文:当进程失去时间片,寄存器上下文会被保存到内存中,下次获取时间片会重新加载
* 进程状态: UNUSED, EMBRYO, SLEEPING,
RUNNABLE, RUNNING, ZOMBIE
* 内存分布情况
* 进程标识
* 父进程标识

这里列举一下xv6的PCB结构,很具有教育意义。
```c
// the registers xv6 will save and restore
// to stop and subsequently restart a process
struct context {
int eip;
int esp;
int ebx;
int ecx;
int edx;
int esi;
int edi;
int ebp;
};
// the different states a process can be in
enum proc_state { UNUSED, EMBRYO, SLEEPING,
RUNNABLE, RUNNING, ZOMBIE };
// the information xv6 tracks about each process
// including its register context and state
struct proc {
char *mem;
// Start of process memory
uint sz;
// Size of process memory
char *kstack;
// Bottom of kernel stack
// for this process
enum proc_state state;
// Process state
int pid;
// Process ID
struct proc *parent;
// Parent process
void *chan;
// If non-zero, sleeping on chan
int killed;
// If non-zero, have been killed
struct file *ofile[NOFILE]; // Open files
struct inode *cwd;
// Current directory
struct context context;
// Switch here to run process
struct trapframe *tf;
// Trap frame for the
// current interrupt
};
```


## 1.2. chapter 5, Interlude: Process API

这一章主要是Unix中的三个控制进程的接口，fork,exec,wait三个系统调用，顺便实现了一下重定向。

### 1.2.1. fork()

fork将进程复制了一份，包括：内存，寄存器等。这可能带来一个不必要的复制动作，但是现在也有了CopyOnWrite技术，没什么好纠结的。

### 1.2.2. wait()

wait被父进程用来等待其子进程结束，可以一定程度克服父子进程运行先后顺序不确定的问题。

### 1.2.3. exec()

exec完全就像是重新创建进程。它首先将当前进程的各种信息、数据全部抹去，再换上新程序的代码段、数据段，同时初始化寄存器等。

可以感觉到，这个系统调用还是比较重的。它做的事情太多了，我们确实需要一种更轻便的系统调用。

### 1.2.4. 神奇的组合效果

确实，上面三个API都不是纯粹的创建进程，都或多或少的做了点其他事情。但他们的确可以完成创建进程的任务。

特别的，fork和exec的组合允许一些额外的操作。比如shell执行一行命令并将输出重定向到文件。过程大致如下：

* shell接受命令，执行fork，进程创建完毕
* shell做一些文件描述符的调整工作，将当前进程的stdin,stdout,stderr重定向到其他文件
* shell执行exec，执行新的程序

新的程序会将输出输出到第二步设定好的文件中。

## 1.3. chapter 6, Mechanism: Limited Direct Execution

这一章主要是介绍进程运行的机制:限制型直接执行.

为了对CPU进行抽象,操作系统需要营造出有好几个CPU同时工作的假象,让每个进程分时进行,对CPU进行**time sharing**.

但是出现了这样的问题:
* 我们减小额外的性能开销
* 如何在允许进程占用CPU的同时,保证对CPU的控制.

### 1.3.1. Direct Execution Protocol

这里先介绍没有限制的一种运行协议:Direct Execution Protocol.这个协议清晰的反映出一个进程的执行需要哪些支持?

下面用这个表格来表现协议的具体内容.

|OS|Program|
|-|-|
|在process list里,为要开启的新进程建立一个entry||
|为进程分配内存页||
|加载进程的程序代码到内存||
|初始化进程堆栈||
|清除cpu中的寄存器||
|调用进程的main()函数,开始进程执行||
||执行main()的代码|
||main()执行完毕,return返回|
|释放进程的内存页||
|将进程从process list中移除||

但是这还不够,有这样的问题没有解决:操作系统如何防止进程做一些不必要的**恶意操作**?比如清空磁盘,更改中断向量表,滥用网络流量.

### 1.3.2. Limited Direction Execution Protocol

现在,我们首先解决恶意操作的问题.我们要对一些操作设置权限,让进程难以进行恶意操作.

这就引入了进程的用户态(user mode)和内核态(kernel mode).

我们这样限制:用户态进程不能随进行危险操作,但可以通过**trap**来进入内核态,完成必要且合法的操作,再通过**return-from-trap**来返回用户态.

注意,这里的trap和return-from-trap都是系统调用,需要向操作系统发起申请.不同的危险操作对应下来的具体操作不同,我们还要引入一个trap table,来帮助操作操作系统完全控制危险操作:操作系统限制了trap可以申请的操作,清楚的**知道**这个trap将会带来什么,从而做到完全的掌控.

这里再引入另一个协议:Limited Direction Execution Protocol.同样用表格来反映它.

|OS|Hardware|Program(user code)|
|-|-|-|
|OS开始boot,同时初始模式为kernel mode|||
|初始化 trap table(类似与中断向量表)|||
||记录好各个系统调用对应程序的入口地址||
|OS完成boot,开始run,同样处于kernel mode|||
|在process list中创建process entry|||
|分配内存页;加载程序代码;分配堆栈并初始化|||
|**将cpu的寄存器状态存入kernel statck**,以供进程切换|||
|**return-from-trap**|||
||恢复kernel mode之前的寄存器状态||
||移动到用户代码,跳转至main()||
|||执行main|
|||执行过程中发生system call,进程陷入内核态(**trap**)|
||保存当前user mode的寄存器状态||
||进入内核态,掉转到trap对应的代码入口||
|调用代码,处理trap要求的操作|||
|return-from-trap|||
||恢复之前user mode的寄存器状态||
||进入user mode,执行用户程序代码||
|||继续执行|
|||return from main|
|||trap|
|释放进程内存|||
|从process list中删除process entry|||

这样,我们解决了危险操作的问题.

但依然剩下一个问题:没有实现进程切换(switch)

### 1.3.3. Switching Between Processes

这里我们先明确什么是running.只有拥有cpu时间片才叫做running.所以,当用户进程running的时候,操作系统没有获得cpu时间片,没有running.

那么,OS如何在自己不running的时候,去strop a process并switch a process呢?实际上,OS需要**regain control of CPU**.

最初的一些OS假设,所有用户进程都会适时地归还控制权,一般是通过trap来实现的.特别是在用户进程发生了一些一场:devided by zero等,就会发生trap,OS借机regain control.但这并不合适,比如,用户进程陷入了死循环,但又不trap,那么OS就没办法regain control.

那好,既然用户进程不一定trap,我们就设置一个一定会引起trap的机制,来帮助操作系统regain control.这个机制就是:**timer interrupt**.

我们用硬件实现一个timer,没当timer到期,就发起一个中断请求.这个中断出发的代码会保存当前用户进程的各种状态,再将控制权交给OS,**由OS来决定,是不是继续执行这个进程**,这就是**调度机制(scheduler)**.

如果OS在schedule的时候,或者在进行系统调用的时候,timer到期发起interuppt呢?我们说,hardware一旦接受了timer的中断请求,就控制权返回给OS,就会**关闭中断(MASK)**,从而使timer不能中断OS.

我们再提一下,timer触发interuppt后,OS夺权是如何**保存上下文**,同时进入下一个进程时如何**切换上下文**.其实就是寄存器的保存,我们仅仅列举一下先xv6的上下文切换汇编代码.

```assembly

# void swtch(struct context **old, struct context *new);
#
# Save current register context in old
# and then load register context from new.

.globl swtch
swtch:
    # Save old registers
    movl 4(%esp), %eax # put old ptr into eax
    popl 0(%eax)       # save the old IP
    movl %esp, 4(%eax) # and stack
    movl %ebx, 8(%eax) # and other registers
    movl %ecx, 12(%eax)
    movl %edx, 16(%eax)
    movl %esi, 20(%eax)
    movl %edi, 24(%eax)
    movl %ebp, 28(%eax)

    # Load new registers
    movl 4(%esp), %eax  # put new ptr into eax
    movl 28(%eax), %ebp # restore other registers
    movl 24(%eax), %edi
    movl 20(%eax), %esi
    movl 16(%eax), %edx
    movl 12(%eax), %ecx
    movl 8(%eax), %ebx
    movl 4(%eax), %esp  # stack is switched here
    pushl 0(%eax)       # return addr put in place
    ret                 # finally return into new ctxt

```

### 1.3.4. 为什么系统调用开销大

现在我们已经明白,系统调用意味着trap into kernel mode.系统调用实际上是OS在运行.系统调用的开销主要在这里:

* 上下文切换:trap一次,return-from-trap又一次
* IO慢速:比如网络,磁盘等IO,很可能带来等待时间


## 1.4. chapter 7, Scheduling:Introduction

这一章是为了延续上一章的调度问题,我们将引入一些调度算法,来选择一些进程去执行.

### 1.4.1. 一些衡量标准

首先,我们需要一些衡量标准来衡量一个调度算法.

|衡量标准|侧重点|计算方法|说明|
|-|-|-|-|
|Turnaround Time|performance|作业完成时间-作业发起时间|一般取一系列作业的均值|
|Response Time|fairness|作业第一次开始时间-作业发起时间||

### 1.4.2. FIFO

FIFO作为一种调度算法,十分的差劲.

它只能选择最先发起的作业去完成.完全没有考虑到IO,也没有考虑到作业本身要占用CPU时间的不同.很容易出现一个长时间任务长期占用CPU,后续作业排队的情况.

### 1.4.3. Shortest Job First(SJF)

这个算法建立在一个假设上:OS知道每个作业做需要占用的CPU时间.它在当前排队的作业中,选择耗时最少的,分配CPU时间.

同样的具有一个问题:一个耗时长的作业最先到来,于是SJF分配CPU给它,但是马上又来了一批耗时相对非常短的作业.这导致平均turnaround时间被拉长.

### 1.4.4. Shortest Time-to-Completion First(STCF)

在SJF中,无法解决轻量级作业后到来的问题.所以,我们引入一种抢占(preempt)的概念.

现在,来了一个作业B.如果B立即开始抢占CPU,会比当前正在占用CPU的作业A继续占用先完成,那么STCF选择分配CPU给B.

依然有个问题,STCF可能会造成,某个作业等待被调度的时间过长,从而拉高了平均相应时间.

### 1.4.5. Round Robin

现在采取一种新的思路:将CPU时间分片.每个作业占用CPU的基本时间单位为time slice.RoundRobin简单的以time sclice为单位,依次执行队列中的作业,直到所有作业都完成.

如果time slice选择的合适,可以显著的减小平均相应时间,但是也可能因为频繁切换进程,导致上下文保护和回复所带来的开销过大.

### 1.4.6. 考虑IO

现在,我们还没有处理IO.一个作业很可能会申请IO操作,如果作业发起IO后,一边占用CPU,一边等待IO完成,未免太浪费.

我们可以将带有IO的作业拆分为子作业,IO事件单独在一个子作业中,非IO事件则被IO事件切分成其他子作业.注意,这只是一个概念上的不同.方便我们理解这样的策略:遇到IO事件时,将CPU分配给其他作业,同时等待IO事件完成.这就造成了一个作业的子作业和其他作业重叠(overlap)进行的错觉.

当IO事件完成,会发起一个中断申请,从而让CPU可以继续执行之前的作业.

到了这里,我们还剩下一个问题:**OS不知道一个作业需要占用CPU多久,也不知道这个作业里有没有IO事件**.我们需要对这两个问题进行预测,预测的基础是最近一段时间各个作业的表现情况.

## 1.5. chapter 8, Scheduling: The Multi-Level Feedback Queue

这里,不再按照书本的顺序来介绍,毕竟循序渐进的章节安排太费文字了.而且,我并不是操作系统设计者,我只是想了解操作系统.

MultiyLevelFeedbackQueue的设计思想主要是:
* 提高作业的turnaround time
* 重视OS与用户的交互

MLFQ比之前介绍的调度算法要复杂一些,它设置了多个不同优先级的队列,作业的优先级越高,越可能获得CPU时间.同时,所有作业的优先级都有下降的趋势,会随着作业的行为(使用CPU情况,使用IO情况)来调整.IO密集型的作业优先级较高,CPU密集型较低,但高优先级获得的时间片比低优先级短.

再简单列举下MLFQ的basic rules:
* 如果作业A的优先级高于作业B,那么A获得CPU,B不能获得CPU.
* 如果A和B的优先级相同,那么A和B采用Round Robin方法来调度,即依次执行.
* 当一个作业刚刚被提交到操作系统,那么它具有最高优先级.
* 如果一个作业在当前优先级运行的时间超过该优先级允许的上限,那么无论这个作业最近的行为如何,都会下降一个优先级(防止程序性行为欺骗).
* 每过一段时间,所有作业的优先级全部置为最高(重新洗牌,避免"  阶级固化",低优先级任务一直starving)

## 1.6. chapter 9, Scheduling: Proportional Share

ProportionalShare 的设计思想是**注重fiarness**,即**保证每个作业能够获得一定百分比的CPU使用机会**.

ProportionalShare引进了一个tickets
的概念.每个作业持有一定量的tickets,tickets越多,获得CPU使用权的**机会越大**(不是说一定会获得CPU,只是概率大).ticekts的维护机制很像货币,主要有三个性质:
* ticket currency  
    每个用户持有一定通用的tickets,但却可以分配给自己的作业任意数量的tickets(非通用).这样,用户的每个作业都可以按比例兑换非荣用tickets和通用tickets,最后根据通用tickets来决定谁获得CPU使用权.
* ticket transfer
    作业持有一定的ticket,并且可以转移ticket给其他作业.这在某些情况下非常有用:C/S架构中,Client向Server发起一个请求,要求一些操作,为了提高Server运行效率,Client可以把自己的tickets转交给Server,请求执行完毕后再拿回tickets.
* ticket inflation
    某些情形下,如果作业之间互相**信任**,允许一些作业在需要时,临时提高自己的ticket持有量,从而获取更多的CPU占用机会.

这种类似货币发行的调度策略,给了调度很大的灵活性.一个job可以改变自己获取CPU的机会;同时,这种依赖随机性的调度只需要一个合格的随机数生成器,以及一个好的数据结构,就很容易实现出来.

## 1.7. chapter 10, Multiprocessor Scheduling

这部分延后在看,先看看内存虚拟化和并发相关章节.

# 2. Memory VIrtualization

在CPU虚拟化成功后,我们可以让多个进程轮流执行,只要我们能够做好上下文切换.但是,保存恢复CPU状态很快,内存的保存恢复却非常慢.所以我们有了这样的需求:能不能不保存恢复进程内存,不把他们保存到磁盘又重新加载到内存?如果条件允许,内存够大,就可以避免相当多的保存恢复工作.当然,这一切还需要对进程内存的保护:保护它不被其他进程篡改.

所以,我们需要一种能够不保存恢复进程内存,同时可以对进程内存提供保护的机制.

## 2.1. chapter 13, The Abstraction:Address Spaces

地址空间是OS抽象出的一层概念,它隐藏了硬件地址的丑陋细节,只留给程序美丽的假象.这种抽象让程序误以为自己**独享**一个**完整**的物理地址.程序将基于这个地址空间安排自己的内存结构:stack,heap,code等.

程序只能接触到虚拟地址,而**虚拟地址到物理地址的转换**由操作系统和硬件共同完成.

这种抽象机制的目标有三点:
* 对进程透明  
    进程并不知道自己所谓的地址空间是假的.实际上,操作系统和硬件一起完成了底层的工作.
* 高效  
    这种机制必须尽可能的高效,所以某些工作交由硬件来完成.
* 保护  
    操作系统必须对每个进程的地址空间进行保护,让每个进程无法接触到不属于自己的物理地址.这既是保护用户进程互相干扰,也是保护操作系统的代码和数据不被篡改.

## 2.2. chapter 14, Interlude: Memory APU

显然,程序可以操作的内存大致分为stack和heap.所以需要有一些API来完成对stack和heap的操作.

### 2.2.1. malloc和free

这里没什么好说的,C/C++已经学的够多了.只想着重记录两个点
* malloc需要指定内存大小,free却不需要  
    这是因为操作系统在底层记录了每个malloc分配的内存块大小,所以free的时候不需要接受大小参数
* malloc和free不是系统调用  
    这一点可以从man page反映出来.malloc和free等内存分配函数在page 3(library calls),而不是在page 2(system calls).但是,malloc等内存管理函数依赖于系统调用brk和sbrk.

### 2.2.2. mmap(占坑)

这是个神奇的函数,可以用来实现进程间内存共享,也可以用来操作文件(特别是文件比内存还大的时候).具体细节现在不研究,先占坑吧.

## 2.3. chapter 15, Mechanism: Address Translation

在chapter13,我们提到了进程地址空间到物理地址的转换.这个转换由操作系统和硬件共同完成.这个转换机制的设计强调三点:
* 效率
* 掌控能力
* 灵活性

我们先来看看一种地址转换机制:base and bounds

### 2.3.1. base and bounds

base and bounds一般叫做Dynamic Relocation.Dynamic Relocation基于一对硬件设备:base,bounds.

进程的地址空间要映射到硬件地址,必须有某种计算方法,Dynamic Relocation采取的计算方式是:**PhysicalAddr=BaseAddr+VirtualAddr**.

每个进程控制块PCB都会维护属于自己的base和bounds,base用于记录BaseAddr,bounds用来记录该进程地址空间的大小.如果进程访问一个地址时,给出的虚拟地址超出了bounds(试图访问不属于自己的物理地址),就会触发一些异常,操作系统会对这个进程进行处理(一般是kill掉).

对于base and bounds机制,每个进程拥有相同大小的地址空间,无论是否被进程使用,都会被映射到物理地址.这也带来了一个问题:internal fragment,即stack和heap反向生长,二者之间会存在较大的空闲内存块.

base和bounds实际是CPU中的一对寄存器,每个CPU都有一对.为了进行地址转换,每个CPU也需要一个MMU(Memory Management Unit),来负责具体的地址计算.