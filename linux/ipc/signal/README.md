---
title: signal是如何实现的
date: 2018-03-25 12:33:27
tags: linux kernel
---


在Linux中，signal算是一种很重要的进程通信机制。signal不仅应用在用户进程之间的交互，还可以应用在用户进程和内核进程的交互。这篇文章将主要探讨：signal的实现机制，以及signal的相关使用事项。

<!-- TOC -->

- [线程组，进程组，会话](#线程组进程组会话)
- [signal的种类](#signal的种类)
- [signal的数据结构](#signal的数据结构)
	- [sighand_struct](#sighand_struct)
	- [k_sigaction](#k_sigaction)
	- [signal_struct](#signal_struct)
	- [sigpending](#sigpending)
	- [siginfo](#siginfo)
- [设置信号handle: sigaction()](#设置信号handle-sigaction)
- [产生信号: kill()](#产生信号-kill)
	- [kill分析](#kill分析)
	- [group_send_sig_info分析](#group_send_sig_info分析)
- [处理信号：do_signal()](#处理信号do_signal)
- [注意事项](#注意事项)
- [reference](#reference)

<!-- /TOC -->

## 线程组，进程组，会话

## signal的种类

在内核里，signal的值其实是一组宏定义，这组宏定义是架构相关的，不同架构之间略有差别。

在i386架构下有32个signal，部分signal表如下：
|编号|名称|缺省操作|解释|
|-|-|-|-|
|1|SIGHUP|Terminate|挂起控制终端/进程|
|2|SIGINT|Terminate|来自键盘的中断，即`Ctrl+c`|
|9|SIGKILL|Terminate|强制进程终止，命令`kill`默认发送这个信号|
|11|SIGSEGV|Dump|无效的内存访问，比如访问内存0xFFFFFFFF时，内核会向用户进程发这个信号|
|19|SIGTOP|Stop|停止进程运行|

## signal的数据结构

### sighand_struct

`sturct sighand_struct`用来记录进程的信号处理函数，并可以被父子进程共享。这里的也存在`copy on write`机制：父子进程共享一张sighandle表，产生修改时，sighandle分化为两张。[这里](test/signal_sharing.c)有copy on write的示例。

```c
struct sighand_struct {
	atomic_t		count;/* 引用计数 */
	struct k_sigaction	action[_NSIG];/* 用来记录信号处理函数的数据结构数组，64个元素 */
	spinlock_t		siglock;/* 自旋锁 */
};
```

### k_sigaction

`struct k_sigaction`是信号handle的载体，它的内部可以简化为`sigaction`。它三个成员，分别描述handle函数的指针、信号默认处理方式、处理当前信号时应该屏蔽的信号：
```c
struct sigaction {
	union {
	  __sighandler_t _sa_handler;
	  void (*_sa_sigaction)(int, struct siginfo *, void *);
	} _u;/* 用户提供的handle */
	sigset_t sa_mask;/* 进程处理当前信号时，哪些信号应该屏蔽，避免受到干扰 */
	unsigned long sa_flags;/* 信号默认的处理方式 */
	void (*sa_restorer)(void);/* 这个似乎没什么用，内核注释写着：linux没有使用这个成员*/
};

/* 信号的默认处理方式 */
#define SA_NOCLDSTOP	0x00000001u
#define SA_NOCLDWAIT	0x00000002u
#define SA_SIGINFO	0x00000004u
#define SA_ONSTACK	0x08000000u
#define SA_RESTART	0x10000000u
#define SA_NODEFER	0x40000000u
#define SA_RESETHAND	0x80000000u
#define SA_NOMASK	SA_NODEFER
#define SA_ONESHOT	SA_RESETHAND
#define SA_INTERRUPT	0x20000000 /* dummy -- ignored */
#define SA_RESTORER	0x04000000
```

### signal_struct

`struct signal_struct`代表进程的信号描述符，会记录当前有哪些信号要处理、线程组中活动的线程数、终端信息等。成员太多，就不做列举了。

### sigpending

sigpending包含一个list和一个singal。sigpending->list里的元素是一系列queue，每个queue包含一系列进程，这些进程需要处理特定的信号。sigpending->signal是一个信号集合，本质是个`unsigned long`，代表list里有哪些信号。

进程描述符中，有两个相关字段：pending，shared_pending。

### siginfo

进程产生信号，发给其他进程时，要把一些信息填入`struct siginfo`再发送，而不是直接发送一个信号的标号(SIGHUP等)。

`struct siginfo`包含如下信息：
* 信号的标号
* 一个union，意义随着siginfo在调用背景变化
	* 当siginfo用于kill()，代表信号发送者的pid和uid
	* 当用于POSIX timers，代表timer的id等信息
	* 当用于POSIX signal，代表timer的id等信息
	* 其他
* 其他

```c
typedef struct siginfo {
	int si_signo;
	int si_errno;
	int si_code;

	union {
		int _pad[SI_PAD_SIZE];

		/* kill() */
		struct {
			pid_t _pid;		/* sender's pid */
			__ARCH_SI_UID_T _uid;	/* sender's uid */
		} _kill;

		/* POSIX.1b timers */
		struct {
			timer_t _tid;		/* timer id */
			int _overrun;		/* overrun count */
			char _pad[sizeof( __ARCH_SI_UID_T) - sizeof(int)];
			sigval_t _sigval;	/* same as below */
			int _sys_private;       /* not to be passed to user */
		} _timer;

		/* POSIX.1b signals */
		struct {
			pid_t _pid;		/* sender's pid */
			__ARCH_SI_UID_T _uid;	/* sender's uid */
			sigval_t _sigval;
		} _rt;

		/* SIGCHLD */
		struct {
			pid_t _pid;		/* which child */
			__ARCH_SI_UID_T _uid;	/* sender's uid */
			int _status;		/* exit code */
			clock_t _utime;
			clock_t _stime;
		} _sigchld;

		/* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
		struct {
			void __user *_addr; /* faulting insn/memory ref. */
#ifdef __ARCH_SI_TRAPNO
			int _trapno;	/* TRAP # which caused the signal */
#endif
		} _sigfault;

		/* SIGPOLL */
		struct {
			__ARCH_SI_BAND_T _band;	/* POLL_IN, POLL_OUT, POLL_MSG */
			int _fd;
		} _sigpoll;
	} _sifields;
} siginfo_t;
```

## 设置信号handle: sigaction()

一般的，我们使用signal()/sigaction()来为进程设置信号的处理handle,signal是老版本的调用方式，新的版本里被废弃了，采用sigaction来代替。他们的底层都是调用`do_sigaction()`，所以下面直接分析do_sigaction()。

```c
int
do_sigaction(int sig, const struct k_sigaction *act, struct k_sigaction *oact);
```

这个函数提供对信号handle的设置/保存功能。如果act不为空，那么sig的handle被设置为act;如果oact不为空，那么handle表里的k_sigaction被拷贝到oact。这个函数的开销并不大，主观上远小于select/poll这种庞然大物。主要完成这些任务：
* 获取进程信号handle表的相应表项的指针
* 如果有信号制衡在等待处理，那么直接返回，不再继续后续工作
* 尝试复制给oact
* 尝试设置进程信号handle表
	* 从进程待处理信号队列中，删除与`sig`相同的信号。因为handle即将被重置，我们禁止老旧的signal触发新的handle。老旧的signal要么成功触发相应函数，要么不出发，绝不允许出错。
	* 把act复制到进程信号handle表

这个函数的源码就不贴了，感兴趣的话可以自己去找，或者去看[我的注释版本](https://github.com/day-dreams/linux-2.6.11.1/blob/master/kernel/signal.c)。

## 产生信号: kill()

实际上，有很多系统调用都可以为进程产生信号，有些还可以为进程组产生信号。这里我们仅仅讨论kill()函数,kill()能够向进程组或者进程发送信号。

### kill分析

```c
/* 用户进程能够调用的接口 */
int kill(pid_t pid, int sig);

/* 系统调用 */
asmlinkage long
sys_kill(int pid, int sig)
{
	struct siginfo info;

	/* 填好siginfo */
	info.si_signo = sig;/* 信号的标号 */
	info.si_errno = 0;
	info.si_code = SI_USER;/* 用户进程发来的信号 */
	info.si_pid = current->tgid;/* 用户进程的pid */
	info.si_uid = current->uid;/* 用户进程的uid */

	return kill_something_info(sig, &info, pid);
}
```

可以看到，`sys_kill`最终落到了`kill_something_info()`。`kill_something_info()`也具有很多功能，可以向进程/进程组发信号，还可以专门服务与终端产生的信号。

```c
static int kill_something_info(int sig, struct siginfo *info, int pid)
{
	if (!pid) {
	/* 如果pid为0，向当前进程的进程组发送信号。终端下的Ctrl+C Ctrl+Z 就是这样的调用*/
		return kill_pg_info(sig, info, process_group(current));
	} else if (pid == -1) {
	/* 遍历所有进程，向当前进程具有发送信号权限的所有进程发送信号*/
		int retval = 0, count = 0;
		struct task_struct * p;

		read_lock(&tasklist_lock);
		/* 从1号进程开始，遍历所有进程 */
		for_each_process(p) {
			/* 如果进程不是init进程，并且不属于当前进程的进程组 */
			if (p->pid > 1 && p->tgid != current->tgid) {
				/* 尝试向这个进程组中的每个进程发送signal */
				int err = group_send_sig_info(sig, info, p);
				++count;
				if (err != -EPERM)
					retval = err;
			}
		}
		read_unlock(&tasklist_lock);
		return count ? retval : -ESRCH;
	} else if (pid < 0) {
	/* 向当前进程组-pid中的每个进程发信号 */
		return kill_pg_info(sig, info, -pid);
	} else {
	/* 向进程pid发信号 */
		return kill_proc_info(sig, info, pid);
	}
```

这里我们关注的是如何向一个进程发信号，即`kill_proc_info()`函数。`kill_proc_info()`函数会通过pid查找进程描述符，再调用`group_send_sig_info()`向进程就发信号。向进程发信号代表着向这个进程所属的所有线程发信号，这个进程下的所有线程都会接受到信号。这里要特别提醒一下，不要混淆了操作系统课里的线程/进程概念，和linux内核里的进程/线程概念。在内核看来，所有线程都是一个真实的进程。

### group_send_sig_info分析

kill底层调用了`group_send_sig_info()`，我们不得不继续分析下去。

`group_send_sig_info()`会先检查权限，看当前进程是否有资格向目标进程发送信号。如果有权限，就继续调用`send_signal()`，把当前进程添加到信号对应的队列里，即是`task->signal->shared_peding`。

添加完毕后，调用`__group_complete_signal()`，遍历线程组，并唤醒一个**满足某些条件的轻量级进程**。也就是说，我们写c时，一个进程下开了n个线程，每个线程都是一个轻量级进程，一个signal只会触发一个线程来处理。

具体的怎么唤醒呢？首先把轻量级进程的状态设置为SIGPENDING，再调用进程调度系统中的try_to_wake_up来唤醒之。

## 处理信号：do_signal()

## 注意事项

* 当有一个信号正在等待处理，如果进程尝试去修改这个信号的handle，那么正在等待处理的信号会被丢弃。

## reference

* Linux kernel 2.6.11.1 源码
* 深入理解LINUX内核
* [POSIX Threads and the Linux Kernel](https://www.kernel.org/doc/ols/2002/ols2002-pages-330-337.pdf)