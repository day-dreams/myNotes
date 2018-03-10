socket实现原理
===


美团一面后,感觉之前学习思路有点偏了.源码只是帮助了解底层的一种工具,目的还是应该落在API的使用和特性上,才能对实际操作起到最大的指导作用.

不出意外,应该是找到实习前的最后几篇源码分析了.

下文将主要分析有关socket的几个系统调用,即sys_socket,sys_bind,sys_listen,sys_socketcall,sys_socketpair等

<!-- TOC -->

- [socket](#socket)
- [sock](#sock)
- [kernel modules](#kernel-modules)
- [sys_socket()](#sys_socket)
	- [sock_create()](#sock_create)
	- [sock_alloc()](#sock_alloc)
	- [sock_map_fd()](#sock_map_fd)
- [sys_bind()](#sys_bind)
	- [inet_bind](#inet_bind)
- [sys_listen](#sys_listen)

<!-- /TOC -->

## socket

socket是内核抽象出来的接口,用户进程通过socket来使用内核网络协议栈,从而最大程度把用户和底层协议实现隔离开来.

用户使用socket还是用文件fd的方式,内核真实的数据结构是sock,只是把sock安装在了文件对象里,进而放到进程的文件表.

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

## sock

sock是内核自己用来操作网络协议栈的结构,字段十分复杂.sock对象本身就很大,粗略估计有100多个字节,这也导致了一个tcp socket要占用几KB的内存空间,还不包括缓冲区.如果socket收到了数据,用户还没取走,缓冲区就被使用起来,tcp socket占用的空间更大.

sock字段我也不想分析,直接罗列出来吧:
```c
struct sock {
	/*
	 * Now struct tcp_tw_bucket also uses sock_common, so please just
	 * don't add nothing before this first member (__sk_common) --acme
	 */
	struct sock_common	__sk_common;
#define sk_family		__sk_common.skc_family
#define sk_state		__sk_common.skc_state
#define sk_reuse		__sk_common.skc_reuse
#define sk_bound_dev_if		__sk_common.skc_bound_dev_if
#define sk_node			__sk_common.skc_node
#define sk_bind_node		__sk_common.skc_bind_node
#define sk_refcnt		__sk_common.skc_refcnt
	volatile unsigned char	sk_zapped;
	unsigned char		sk_shutdown;
	unsigned char		sk_use_write_queue;
	unsigned char		sk_userlocks;
	socket_lock_t		sk_lock;
	int			sk_rcvbuf;
	wait_queue_head_t	*sk_sleep;
	struct dst_entry	*sk_dst_cache;
	rwlock_t		sk_dst_lock;
	struct xfrm_policy	*sk_policy[2];
	atomic_t		sk_rmem_alloc;
	struct sk_buff_head	sk_receive_queue;
	atomic_t		sk_wmem_alloc;
	struct sk_buff_head	sk_write_queue;
	atomic_t		sk_omem_alloc;
	int			sk_wmem_queued;
	int			sk_forward_alloc;
	unsigned int		sk_allocation;
	int			sk_sndbuf;
	unsigned long 		sk_flags;
	char		 	sk_no_check;
	unsigned char		sk_debug;
	unsigned char		sk_rcvtstamp;
	unsigned char		sk_no_largesend;
	int			sk_route_caps;
	unsigned long	        sk_lingertime;
	int			sk_hashent;
	/*
	 * The backlog queue is special, it is always used with
	 * the per-socket spinlock held and requires low latency
	 * access. Therefore we special case it's implementation.
	 */
	struct {
		struct sk_buff *head;
		struct sk_buff *tail;
	} sk_backlog;
	rwlock_t		sk_callback_lock;
	struct sk_buff_head	sk_error_queue;
	struct proto		*sk_prot;
	int			sk_err,
				sk_err_soft;
	unsigned short		sk_ack_backlog;
	unsigned short		sk_max_ack_backlog;
	__u32			sk_priority;
	unsigned short		sk_type;
	unsigned char		sk_localroute;
	unsigned char		sk_protocol;
	struct ucred		sk_peercred;
	int			sk_rcvlowat;
	long			sk_rcvtimeo;
	long			sk_sndtimeo;
	struct sk_filter      	*sk_filter;
	void			*sk_protinfo;
	kmem_cache_t		*sk_slab;
	struct timer_list	sk_timer;
	struct timeval		sk_stamp;
	struct socket		*sk_socket;
	void			*sk_user_data;
	struct module		*sk_owner;
	struct page		*sk_sndmsg_page;
	__u32			sk_sndmsg_off;
	struct sk_buff		*sk_send_head;
	int			sk_write_pending;
	void			*sk_security;
	__u8			sk_queue_shrunk;
	/* three bytes hole, try to pack */
	void			(*sk_state_change)(struct sock *sk);
	void			(*sk_data_ready)(struct sock *sk, int bytes);
	void			(*sk_write_space)(struct sock *sk);
	void			(*sk_error_report)(struct sock *sk);
  	int			(*sk_backlog_rcv)(struct sock *sk,
						  struct sk_buff *skb);  
	void                    (*sk_destruct)(struct sock *sk);
};
```

## kernel modules


模块也是内核的一部分.要知道,boot的时候,内核代码被装入了内存,但并不是全部的内核代码都被装载了.有一部分代码由于对boot不是很重要,属于boot时可有可无的东西,所以可以推迟装载,等用户进程调用了相关代码,才装载到内存.这样的代码叫做内核模块,即kernel modules.

TCP/IP协议栈就属于一个modules.

## sys_socket()

这个系统调用为用户进程创建socket.接下来首先说说大概细节,在列举相关代码.

sys_socket()内部是这样的:
* 调用sock_create(),创建sock对象
	* sock_create其实是__sock_create的封装.由于网络不是一个module,__sock_create需要处理与模块加载相关的细节.sock_create先调用sock_alloc创建好socket,再试图增加相关模块的引用计数,把socket注册到相关的模块上,再减少引用计数.
	* sock_alloc主要是创建inode节点,创建socket有点像个副业.sock_alloc从socket专用的超级块上创建inode,创建好的inode节点自带一个socket,所以不必单独创建socket对象.创建好后,需要修改inode节点的模式(是普通文件节点还是个socket)、用户使用权限等，同时还要修改系统使用的socket数量。由于socket数量是个cpu相关的数据，所以需要在暂时关闭cpu抢占功能(preempt_disable())。
* 调用sock_map_fd(),把sock对象映射到文件对象,并安装到进程的文件表。
	* 文件对象是依赖与dentry和inode的,所以sock_map_fd需要先创建好dentry,并用socket的操作表去设置dentry和inode的操作表.完成这些工作后,再修改进程的文件表,完成socket创建.

```c
asmlinkage long sys_socket(int family, int type, int protocol)
{
	int retval;
	struct socket *sock;

	/* 创建socket对象 */
	retval = sock_create(family, type, protocol, &sock);
	if (retval < 0)
		goto out;

	/* 把socket对象映射到文件对象,返回值即是文件对象描述副 */
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

### sock_create()

sock_create只是一层封装,底层是__sock_create,主要完成这些事情:
* 处理相关模块的引用计数
* 调用sock_alloc创建socket,并注册到相关模块上


__sock_create的源码很罗嗦,很多是关于错误处理的代码,这里只节选部分:
```c
static int __sock_create(int family, int type, int protocol, struct socket **res, int kern)
{
	int err;
	struct socket *sock;

	...

	/* 创建socket对象 */
	if (!(sock = sock_alloc())) {
		printk(KERN_WARNING "socket: no more sockets\n");
		err = -ENFILE;		/* Not exactly a match, but its the
					   closest posix thing */
		goto out;
	}
	sock->type  = type;

	...

	/* 因为这段代码用到了相关模块,所以增加相关模块的引用计数 */
	err = -EAFNOSUPPORT;
	if (!try_module_get(net_families[family]->owner))
		goto out_release;

	/* 把socket注册到内核模块 */
	if ((err = net_families[family]->create(sock, protocol)) < 0)
		goto out_module_put;

	/* 因为socket被注册了,要再次增加相关模块的引用计数 */
	if (!try_module_get(sock->ops->owner)) {
		sock->ops = NULL;
		goto out_module_put;
	}

	/* socket创建好了,也被注册到了相关模块;这段代码不再使用相关模块,所以减小相关模块的引用计数 */
	module_put(net_families[family]->owner);

}
```

### sock_alloc()

这个函数完成:
* 从socket超级块上创建inode
* 修改系统使用的socket数量,期间需要暂时关闭cpu抢占功能

```c
static struct socket *sock_alloc(void)
{
	struct inode * inode;
	struct socket * sock;

	/* 从专门的socket超级块上创建inode,可能是调用超级块自带的方法,也可能是调用slab机制来创建 */
	inode = new_inode(sock_mnt->mnt_sb);
	if (!inode)
		return NULL;

	/* socket超级块创建的inode,自然都带有一个socket,那么获得这个socket的引用 */
	sock = SOCKET_I(inode);

	/* 设置inode字段 */
	inode->i_mode = S_IFSOCK|S_IRWXUGO;
	inode->i_sock = 1;
	inode->i_uid = current->fsuid;
	inode->i_gid = current->fsgid;

	/* 修改当前系统使用的sockets的数量,因为涉及到修改cpu的数据,需要暂时关闭cpu抢占功能并迅速打开 */
	get_cpu_var(sockets_in_use)++;
	put_cpu_var(sockets_in_use);
	return sock;
}
```

### sock_map_fd()


sock_map_fd主要完成几件事情


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

## sys_bind()

不同类型的socket有不同的bind函数,通过socket->ops->bind可以调用之.

sys_bind根据fd找到socket对象,再检查bind是不是合法的,检查完毕调用socket->ops->bind,完成bind操作.


```c
asmlinkage long sys_bind(int fd, struct sockaddr __user *umyaddr, int addrlen)
{
	struct socket *sock;
	char address[MAX_SOCK_ADDR];
	int err;

	/* 根据fd查找到对应的socket对象 */
	if((sock = sockfd_lookup(fd,&err))!=NULL)
	{
		/* 把要绑定的地址传给内核 */
		if((err=move_addr_to_kernel(umyaddr,addrlen,address))>=0) {
			/* 检查绑定操作是不是安全的 */
			err = security_socket_bind(sock, (struct sockaddr *)address, addrlen);
			if (err) {
				/* 减小socket对应的file对象的引用计数 */
				sockfd_put(sock);
				return err;
			}
			/* 真正的bind */
			err = sock->ops->bind(sock, (struct sockaddr *)address, addrlen);
		}
		/* 减小socket对应的file对象的引用计数 */
		sockfd_put(sock);
	}			
	return err;
}
```

### inet_bind

bind的版本太多,下面主要分析inet_bind,这个版本被tcp socket,udp socket,raw socket共同使用.

inet_bind并没有什么特别复杂的操作,它主要完成这些工作:
* 检查是否可以bind.有时候,很多bind都是无法进行的,比如网线被拔了还试图绑定外部地址等.检查必须确定,有一种bind是可以成功的,这种bind可以是绑定到多播地址,也可以是绑定到本地地址,等等.
* 设置sock对象的相关字段.当网卡收到数据包,将在所有socket里查找正确的收包socket,这里的查找就用到了saddr,sport等字段.

```c
int inet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct sockaddr_in *addr = (struct sockaddr_in *)uaddr;
	struct sock *sk = sock->sk;
	struct inet_sock *inet = inet_sk(sk);
	unsigned short snum;
	int chk_addr_ret;
	int err;

	/* If the socket has its own bind function then use it. (RAW) */
	if (sk->sk_prot->bind) {
		err = sk->sk_prot->bind(sk, uaddr, addr_len);
		goto out;
	}
	err = -EINVAL;
	if (addr_len < sizeof(struct sockaddr_in))
		goto out;

	chk_addr_ret = inet_addr_type(addr->sin_addr.s_addr);

	/* Not specified by any standard per-se, however it breaks too
	 * many applications when removed.  It is unfortunate since
	 * allowing applications to make a non-local bind solves
	 * several problems with systems using dynamic addressing.
	 * (ie. your servers still start up even if your ISDN link
	 *  is temporarily down)
	 */

	/* 各种各样的检查,比如inet socket是否可以自由绑定,是否可以绑定到多播广播地址等 */
	err = -EADDRNOTAVAIL;
	if (!sysctl_ip_nonlocal_bind &&
	    !inet->freebind &&
	    addr->sin_addr.s_addr != INADDR_ANY &&
	    chk_addr_ret != RTN_LOCAL &&
	    chk_addr_ret != RTN_MULTICAST &&
	    chk_addr_ret != RTN_BROADCAST)
		goto out;

	snum = ntohs(addr->sin_port);
	err = -EACCES;
	if (snum && snum < PROT_SOCK && !capable(CAP_NET_BIND_SERVICE))
		goto out;

	/*      We keep a pair of addresses. rcv_saddr is the one
	 *      used by hash lookups, and saddr is used for transmit.
	 *
	 *      In the BSD API these are the same except where it
	 *      would be illegal to use them (multicast/broadcast) in
	 *      which case the sending device address is used.
	 */
	lock_sock(sk);

	/* Check these errors (active socket, double bind). */
	err = -EINVAL;
	if (sk->sk_state != TCP_CLOSE || inet->num)
		goto out_release_sock;

	inet->rcv_saddr = inet->saddr = addr->sin_addr.s_addr;
	if (chk_addr_ret == RTN_MULTICAST || chk_addr_ret == RTN_BROADCAST)
		inet->saddr = 0;  /* Use device */

	/* Make sure we are allowed to bind here. */
	if (sk->sk_prot->get_port(sk, snum)) {
		inet->saddr = inet->rcv_saddr = 0;
		err = -EADDRINUSE;
		goto out_release_sock;
	}

	if (inet->rcv_saddr)
		sk->sk_userlocks |= SOCK_BINDADDR_LOCK;
	if (snum)
		sk->sk_userlocks |= SOCK_BINDPORT_LOCK;
	inet->sport = htons(inet->num);
	inet->daddr = 0;
	inet->dport = 0;
	sk_dst_reset(sk);
	err = 0;
out_release_sock:
	release_sock(sk);
out:
	return err;
}
```

## sys_listen