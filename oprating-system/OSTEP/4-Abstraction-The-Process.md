Operating Systems: Three Easy Pieces
=====================================

chapter 4, The Abstraction: The Process
<!-- TOC -->

- [1. 什么是进程](#1-什么是进程)
- [2. 用于控制进程的API](#2-用于控制进程的api)
- [3. 创建一个进程需要哪些工作](#3-创建一个进程需要哪些工作)
    - [3.1. 将程序装载到内存](#31-将程序装载到内存)
    - [3.2. 分配堆栈内存](#32-分配堆栈内存)
    - [3.3. 其他初始化工作](#33-其他初始化工作)
- [4. 进程状态](#4-进程状态)
- [5. 进程需要的数据结构](#5-进程需要的数据结构)

<!-- /TOC -->
# 1. 什么是进程
首先我们要明白什么是进程.进程操作系统对运行中的程序的一种抽象.

那么,对于进程执行来说,有那些重要的操作系统部分?

* **内存**:用于存放指令和部分数据
* **寄存器**:用于存放进程的另一部分数据,特殊的有,PC,StackPointer等
* **存储设备**:一些进程正在访问的I/O文件

# 2. 用于控制进程的API

既然是一种抽象,就因该有接口来管理这层抽象.我们一般需要这些接口:
* **Create**:创建一个进程
* **Destroy**:销毁\撤销一个进程
* **Wait**:等待一个进程结束
* **Miscellaneous Control**:各种控制方法,如挂起和激活
* **Status**:查看一个进程的状态

# 3. 创建一个进程需要哪些工作

## 3.1. 将程序装载到内存

装载包括指令和数据两个部分.程序首先以某种可执行文件(ELF,EXE)的形式存在与磁盘上,操作系统可以读取它们并写入内存。

这里的装载还包括两种形式:lazy装载,eagerly装载．

eagerly装载就简单的把所有程序指令都装入内存,无论是否真的需要所有的指令.这往往是早期操作系统.

lazy装载则不同,只放入一些重要片段.这往往与分页机制有关,暂不研究.

## 3.2. 分配堆栈内存

进程必须具有一个独立的栈.这个栈往往用来存放临时变量,同时具有最大空间限制(例如8M).

还有堆内存.例如C程序,有静态全局变量这样的东西.同时还有一些动态分配的内存需要堆来负责.所以,这个堆一开始往往较小,随着程序运行而增长.

## 3.3. 其他初始化工作

还需要做一些IO方面的初始化.

如Linux中,每个进程都默认关联stderr,stdout,stdin三个文件描述符.

当一些准备好了,操作系统需要将进程的执行起点调至main函数,进入就需状态,等待合适的CPU时间片分配给进程.

# 4. 进程状态

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

# 5. 进程需要的数据结构

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