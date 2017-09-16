Operating Systems: Three Easy Pieces
=====================================
<!-- TOC -->

- [1. chapter 4, The Abstraction: The Process](#1-chapter-4-the-abstraction-the-process)
    - [1.1. 什么是进程](#11-什么是进程)
    - [1.2. 用于控制进程的API](#12-用于控制进程的api)
    - [1.3. 创建一个进程需要哪些工作](#13-创建一个进程需要哪些工作)
        - [1.3.1. 将程序装载到内存](#131-将程序装载到内存)
        - [1.3.2. 分配堆栈内存](#132-分配堆栈内存)
        - [1.3.3. 其他初始化工作](#133-其他初始化工作)
    - [1.4. 进程状态](#14-进程状态)
    - [1.5. 进程需要的数据结构](#15-进程需要的数据结构)
- [2. chapter 5, Interlude: Process API](#2-chapter-5-interlude-process-api)
    - [2.1. fork()](#21-fork)
    - [2.2. wait()](#22-wait)
    - [2.3. exec()](#23-exec)
    - [2.4. 神奇的组合效果](#24-神奇的组合效果)
- [3. chapter 6, Mechanism: Limited Direct Execution](#3-chapter-6-mechanism-limited-direct-execution)
    - [3.1. Direct Execution Protocol](#31-direct-execution-protocol)
    - [3.2. Limited Direction Execution Protocol](#32-limited-direction-execution-protocol)
    - [3.3. Switching Between Processes](#33-switching-between-processes)

<!-- /TOC -->

# 1. chapter 4, The Abstraction: The Process

## 1.1. 什么是进程
首先我们要明白什么是进程.进程操作系统对运行中的程序的一种抽象.

那么,对于进程执行来说,有那些重要的操作系统部分?

* **内存**:用于存放指令和部分数据
* **寄存器**:用于存放进程的另一部分数据,特殊的有,PC,StackPointer等
* **存储设备**:一些进程正在访问的I/O文件

## 1.2. 用于控制进程的API

既然是一种抽象,就因该有接口来管理这层抽象.我们一般需要这些接口:
* **Create**:创建一个进程
* **Destroy**:销毁\撤销一个进程
* **Wait**:等待一个进程结束
* **Miscellaneous Control**:各种控制方法,如挂起和激活
* **Status**:查看一个进程的状态

## 1.3. 创建一个进程需要哪些工作

### 1.3.1. 将程序装载到内存

装载包括指令和数据两个部分.程序首先以某种可执行文件(ELF,EXE)的形式存在与磁盘上,操作系统可以读取它们并写入内存。

这里的装载还包括两种形式:lazy装载,eagerly装载．

eagerly装载就简单的把所有程序指令都装入内存,无论是否真的需要所有的指令.这往往是早期操作系统.

lazy装载则不同,只放入一些重要片段.这往往与分页机制有关,暂不研究.

### 1.3.2. 分配堆栈内存

进程必须具有一个独立的栈.这个栈往往用来存放临时变量,同时具有最大空间限制(例如8M).

还有堆内存.例如C程序,有静态全局变量这样的东西.同时还有一些动态分配的内存需要堆来负责.所以,这个堆一开始往往较小,随着程序运行而增长.

### 1.3.3. 其他初始化工作

还需要做一些IO方面的初始化.

如Linux中,每个进程都默认关联stderr,stdout,stdin三个文件描述符.

当一些准备好了,操作系统需要将进程的执行起点调至main函数,进入就需状态,等待合适的CPU时间片分配给进程.

## 1.4. 进程状态

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

## 1.5. 进程需要的数据结构

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


# 2. chapter 5, Interlude: Process API

这一章主要是Unix中的三个控制进程的接口，fork,exec,wait三个系统调用，顺便实现了一下重定向。

## 2.1. fork()

fork将进程复制了一份，包括：内存，寄存器等。这可能带来一个不必要的复制动作，但是现在也有了CopyOnWrite技术，没什么好纠结的。

## 2.2. wait()

wait被父进程用来等待其子进程结束，可以一定程度克服父子进程运行先后顺序不确定的问题。

## 2.3. exec()

exec完全就像是重新创建进程。它首先将当前进程的各种信息、数据全部抹去，再换上新程序的代码段、数据段，同时初始化寄存器等。

可以感觉到，这个系统调用还是比较重的。它做的事情太多了，我们确实需要一种更轻便的系统调用。

## 2.4. 神奇的组合效果

确实，上面三个API都不是纯粹的创建进程，都或多或少的做了点其他事情。但他们的确可以完成创建进程的任务。

特别的，fork和exec的组合允许一些额外的操作。比如shell执行一行命令并将输出重定向到文件。过程大致如下：

* shell接受命令，执行fork，进程创建完毕
* shell做一些文件描述符的调整工作，将当前进程的stdin,stdout,stderr重定向到其他文件
* shell执行exec，执行新的程序

新的程序会将输出输出到第二步设定好的文件中。

# 3. chapter 6, Mechanism: Limited Direct Execution

这一章主要是介绍进程运行的机制:限制型直接执行.

为了对CPU进行抽象,操作系统需要营造出有好几个CPU同时工作的假象,让每个进程分时进行,对CPU进行**time sharing**.

但是出现了这样的问题:
* 我们减小额外的性能开销
* 如何在允许进程占用CPU的同时,保证对CPU的控制.

## 3.1. Direct Execution Protocol

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

## 3.2. Limited Direction Execution Protocol

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

## 3.3. Switching Between Processes

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




