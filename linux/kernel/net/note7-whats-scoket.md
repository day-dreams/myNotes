socket实现原理
===

socket在网络编程中的地位是毋庸置疑的，本文将对socket实现原理进行深度剖析，会涉及到一些底层数据结构(socket,sock,数据包缓冲区，sock哈系表)，以及一些重要的socket调用(send,recv,bind,create等)。

全文依赖于`linux-2.6.11.1`源代码。

## socket和sock

`socket`是一个面向用户的结构，用户用socket来操纵网络协议栈，使用相应的网络功能。我们使用`socket()`调用创建的数据结构就是socket，创建完成后，这个`socket`带有自己的协议类型，它可以是`TCP socket`、`UDP socket`、`raw socket`等。这个协议类型限定了，socket可供调用的方法，以及方法的具体行为。

```c
struct socket {/* socket对象 */
	socket_state		state;/* socket状态,比如connected等 */
	unsigned long		flags;/* socket标志 */
	struct proto_ops	*ops;/* socket相关的操作,与协议相关 */
	struct fasync_struct	*fasync_list;/* TODO:异步的等待队列 */
	struct file		*file;/* 相关联的文件对象 */
	struct sock		*sk;/* 相关联的sock对象 */
	wait_queue_head_t	wait;
	short type; /* socket类型,如sock_stream,sock_raw */
	unsigned char		passcred;
};
```

`sock`与`socket`不同，它更多的是面向网络层，主要是内核在使用这个结构。`sock`中有大量内核相关的细节，我们只关心其中的某些成员。

```c
struct sock {
	char		 	sk_no_check;/* 禁用校验和标志 */
	unsigned char		sk_protocol;/* 协议类型，创建socket时设定的 */
	socket_lock_t		sk_lock;/* 锁 */
	int			sk_hashent;/* hash entry */
	int			sk_rcvbuf;/* 接收缓冲区大小(byte) */
	int			sk_sndbuf;/* 发送缓冲区大小(byte) */
	struct sk_buff_head	sk_receive_queue;/* 已接受到的数据包缓冲队列 */
    struct sk_buff_head sk_write_queue; /* 待发送的数据包缓冲队列 */
    long			sk_rcvtimeo;/* 默认的接受超时时间 */
	long			sk_sndtimeo;/* 默认的发送超时时间 */
	wait_queue_head_t	*sk_sleep;/* 等待队列 */
	void			(*sk_state_change)(struct sock *sk);
	void			(*sk_data_ready)(struct sock *sk, int bytes);/* 回调，当有新数据到达时调用 */
	void			(*sk_write_space)(struct sock *sk);/* 回调，用于指出可用于数据传输的内存 */
	void			(*sk_error_report)(struct sock *sk);/* 回调 */
  	int			(*sk_backlog_rcv)(struct sock *sk,
						  struct sk_buff *skb);  /* 回调 */
	void                    (*sk_destruct)(struct sock *sk);/* 回调，可以理解为sock的析构函数 */

        ...

```

## 创建socket

系统调用`sys_socket()`用于创建socket和sock对象。

```c
asmlinkage long sys_socket(int family, int type, int protocol)
{
	int retval;
	struct socket *sock;

	/* 创建socket对象 */
	retval = sock_create(family, type, protocol, &sock);
	if (retval < 0)
		goto out;

	/* 把socket对象映射到文件对象,返回值即是文件对象描述符 */
	retval = sock_map_fd(sock);
	if (retval < 0)
		goto out_release;

out:
	/* It may be already another descriptor 8) Not kernel problem. */
	return retval;

out_release:
	sock_release(sock);
	return retval;
}
```

### 创建socket对象

在`sys_socket()`的内部，`sock_create()`完成socket对象的创建。它首先会分配出一个socket/sock结构体，并完成相应的字段初始化工作。初始化完成后，需要把这个socket注册到相应的协议族(`net_proto_family`)里。

注册的必要性体现在相关的检查：内核启动时，某个协议可能并没有被加载，比如内核没有打开tcp协议功能，注册socket到协议族就意味着`检查协议是否被加载`、`检查内核是否支持该协议`。如果协议还没有加载，内核需要动态的把协议代码加载到内存。

<!-- `sock_create()`的具体实现和地址簇相关。比如，对于IPv4下的socket，`sys_create()`会使用`inet_create()`函数来创建socket对象。不过有一点是通用的：**socket/sock对象的创建都是通过slab机制来获得内存**。 -->

具体到socket/sock的内存分配，`sock_alloc()`函数负责socket/sock的创建。

#### 创建细节

`sock_alloc()`从socket超级块(`sock_mnt`)上创建inode节点，这里不确定是不是从slab机制上创建。我个人认为是走特殊的创建机制，但暂时没看过sock_mnt超级块的初始化流程，我只能作出这样的推测：

sock_mnt具有自己特殊的inode节点创建方法，`sock_mnt->s_ops->alloc_inode()`不仅会创建inode节点，还会创建socket对象。并且返回的是一个inode和socket的复合结构体()。`sock_alloc()`最终返回的是socket对象的地址。
```c
struct socket_alloc {
	struct socket socket;
	struct inode vfs_inode;
};
```

## 把socket对象安装到进程文件表

在linux下，用户无法直接操作socket对象的，而是通过文件对象来使用socket。用户通常根据一个文件描述符，索引到文件对象,再跳转到索引节点来调用相关的socket函数。

`sys_socket()`会调用`sock_map_fd()`来完成socket对象的安装，代码并不复杂。首先，我们获得一个空闲的file对象，然后设置好file指向的inode、dentry、mnt等信息，再从进程文件表获得空闲的fd，把file赋值给文件表中相应的表项。

```c
int sock_map_fd(struct socket *sock)
{
	int fd;
	struct qstr this;
	char name[32];

	/*
	 *	Find a file descriptor suitable for return to the user. 
	 */

	fd = get_unused_fd();
	if (fd >= 0) {
		/* 获取一个空闲的file结构体 */
		struct file *file = get_empty_filp();

		if (!file) {
			put_unused_fd(fd);
			fd = -ENFILE;
			goto out;
		}

		sprintf(name, "[%lu]", SOCK_INODE(sock)->i_ino);
		this.name = name;
		this.len = strlen(name);
		this.hash = SOCK_INODE(sock)->i_ino;

		/* 分配一个dentry对象 */
		file->f_dentry = d_alloc(sock_mnt->mnt_sb->s_root, &this);
		if (!file->f_dentry) {
			put_filp(file);
			put_unused_fd(fd);
			fd = -ENOMEM;
			goto out;
		}
		/* 更新dentry的操作表 */
		file->f_dentry->d_op = &sockfs_dentry_operations;
		/* 把dentry关联到socket对应的inode上 */
		d_add(file->f_dentry, SOCK_INODE(sock));
		file->f_vfsmnt = mntget(sock_mnt);
		file->f_mapping = file->f_dentry->d_inode->i_mapping;

		/* 把file关联到socket上 */
		sock->file = file;
		/* 更新file的操作表 */
		file->f_op = SOCK_INODE(sock)->i_fop = &socket_file_ops;
		file->f_mode = FMODE_READ | FMODE_WRITE;
		file->f_flags = O_RDWR;
		file->f_pos = 0;
		/* 设置文件表 */
		fd_install(fd, file);
	}

out:
	return fd;
}
```

## `sock_iocb`，`msghdr`和`kiocb`

在本文里，我们只关心socket相关的io操作封装：`msghdr`、`kiocb`、`sock_iocb`。这些并不影响我们对socket的理解，如果不感兴趣，可以跳过这一节。

对于2.6.11版本内核，IO操作都需要经过封装，才能最终传入内核。在最上层的系统调用里(read/write/send/recv)，操作都是裸露的，用户提供用户态的缓冲区，以及相应的大小信息;到了这些系统调用的底层，操作会被封装成`kiocb`，以提供更多的状态信息，比如用户信息、回调函数等。这里的回调函数包括取消、重试、析构等一系列函数。

对于socket，写调用(send,sendto等)最终都依赖于系统调用`sys_sendto()`。
```c
asmlinkage long sys_sendto(int fd, void __user * buff, size_t len, unsigned flags,
			   struct sockaddr __user *addr, int addr_len)
```

在`sys_sendto()`中，buff、len等信息最终被封装成`msghdr`，这个结构的意义更多地在于接口的统一，倒不是说有多么重要。

```c
struct msghdr {
	void	*	msg_name;	/* socket的地址 */
	int		msg_namelen;	/* socket地址的长度 */
	struct iovec *	msg_iov;	/* 数据块矢量数组的首地址（就是数据地址+长度） */
	__kernel_size_t	msg_iovlen;	/* 数据块矢量的个数 */
	void 	*	msg_control;	/* 控制块地址 */
	__kernel_size_t	msg_controllen;	/* 控制块长度 */
	unsigned	msg_flags;      /* 标志 */
};
```

`sys_sendto()`封装好msghdr后，调用`sock_sendmsg()`，`sock_sendmsg()`完成对`kiocb`和`sock_iocb`的封装，最终调用socket对象的sendmsg方法。


## UDP socket

接下来，我们正式分析不同协议下的socket行为。先分析UDP socket。

### 用户发送UDP数据包

针UDP socket的`send`、`sendto`、`sendmsg`、`write`调用，是由`udp_sendmsg()`函数实现的。在最初创建udp socket的时候，`udp_sendmsg()`被写入到socket操作表的`sendmsg`和`write`域；在后续对socket的写入调用里，无论是一般的write调用，还是socket专用的send系列调用，最终都落实到`udp_sendmsg()`函数。

`udp_sendmsg()`有将近200行的长度，主要负责`UDP数据包生成`、`UDP数据包推入IP层`、`主动调用IP层发送数据包`。我们仅仅分析它的大概流程，不对代码做详尽注释。

```c
int udp_sendmsg(struct kiocb *iocb, struct sock *sk, struct msghdr *msg,
		size_t len);
```

首先，`udp_sendmsg()`做一些检查工作，比如数据长度必须大于0等。

如果这个udp socket下有数据需要处理，比如其他进程也调用了这个socket的send方法，导致这个socket正在发送其他数据包，内核必须等待它们完成这个操作。内核会尝试获取udp socket的锁，获得后再进行下一步操作。

接下来就是组织UDP 数据包了，包括目的IP、端口的设置，以及长度、校验和的设置。组织好UDP层的数据后，内核把数据推入IP层。IP层可能会采取分片策略，把数据包拆分成更小的部分，不过这是IP层的事情，对于UDP层是透明不可见的。

数据推入IP层后，数据包并没有经过网卡被发送出去，而是堆积在缓冲区里。如果这个socket没有设置CORK选项，内核立即把缓冲区里的数据包发送出去;否则，内核什么也不做，完成`udp_sendmsg`调用。另外，如果内核把UDP数据推入IP层时，发生了某些错误，内核必须立即把缓冲区里的所有数据发送出去。

UDP数据包的发送相对简单，到这里就分析完毕了。至于IP层如何把数据包发出去，这不是我们现在关心的重点。我们只用知道：IP层发送数据包也是个相对耗时的操作，并且会引起相关阻塞。

### 用户接收UDP数据包

相比UDP数据包的发送，我个人更关心UDP数据包的接收。与`udp_sendmsg()`相似，UDP数据包的接收由`recv`、`recvfrom`、`recvmsg`、`read`、发起，最终落实到`udp_recvmsg()`函数。

```c
static int udp_recvmsg(struct kiocb *iocb, struct sock *sk, struct msghdr *msg,
		       size_t len, int noblock, int flags, int *addr_len);
```

`udp_recvmsg()`会尝试从socket的输入缓冲区中取数据包。

```c
	skb = skb_recv_datagram(sk, flags, noblock, &err);
```

`skb_recv_datagram()`陷入一个循环，试图在超时时间内，从`socket->sock->sk_receive_queue`里获取`sk_buff`。如果超时还未获取到，返回错误信息，`udp_recvmsg()`会直接返回这个错误信息。如果socket被设置成非阻塞，那么超市时间是0,即不允许阻塞。

获取到`sk_buff`后，`udp_recvmsg()`继续进行校验工作，并把数据从内核态复制到用户态。复制的具体细节是，从sk_buff复制到msghdr指向的用户提供的buffer。

复制完成后，内核会为这个数据包打上时间戳。这个时间戳可以表示`用户调用recv从sk_buff获取数据的时间`，也可以表示`数据包到达主机的时间`，通过`SO_TIMESTAMP`选项可以进行选择。可见，**我们可以精确获取数据包到达主机的时间**！

最后，`udp_recvmsg()`把sk_buff从释放掉，减小udp socket使用的缓存空间。

### 内核接受UDP数据包

UDP数据包不是一开始就在`socket->sock->sk_receive_queue`里的。

网卡接受到数据包后，发起中断或者信号，通知内核接受数据包，内核创建好`sk_buff`,把数据从网卡内存复制到内核内存;内核再根据端口地址信息，搜索hash表，查找应该接受这个数据包的socket，再把`sk_buff`复制到目标socket。

如果发现没有socket要接受这个数据包，内核会选择丢弃这个数据包，再发回一个错误信息,比如`地址不可到达`。

这些工作部分由`udp_rcv()`完成，部分由IP层完成，本文不做进一步分析。

## reference

[理解 linux 中的 container_of 和 offsetof 宏](https://baurine.github.io/2013/03/24/understand_container_of.html)