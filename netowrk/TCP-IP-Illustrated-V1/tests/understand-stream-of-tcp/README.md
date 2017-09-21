这里想做个实验,来加深理解stream的概念.

如例子所示,client分批发送了多个数据,server仅一次接受,就收到了所有的数据.

send内部可能并没有马上发送,而是有延迟,根据具体的情况来决定是不是要和后续的数据包组合在一起发出去.

recv则是从buffer中立即取出要求的所有数据.

结论是,**recv和send并不具有一对一的调用关系**.

同时还想探讨一下读cache和写buffer的大小.可以同通过这个命令来查看:

>moon@moon-Think:~$ cat /proc/sys/net/ipv4/tcp_wmem  
>moon@moon-Think:~$ cat /proc/sys/net/ipv4/tcp_rmem  

我的机器上显示的是这样:

|min|default|max|
|-|-|-|
|4096|	16384|	4194304|
|4096|	87380|	6291456|
