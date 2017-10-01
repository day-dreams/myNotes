这个仓库仅用于笔记记录
<!-- TOC -->

- [笔记](#笔记)
- [待解决](#待解决)

<!-- /TOC -->

# 笔记
* [算法数据结构](./algorithms-datastructure)  
   * [主定理的理解和证明](主定理的理解和证明.md)  
* [8086汇编](./assembly-8086)  
    * [寄存器布局](./assembly-8086/registers.md)
* [C++](./C++)  
    * [异常,断言等错误处理的实现原理](./C++/assert-exception-panic-recover)  
    * [对象模型](./C++/class)  
    * [右值引用](./C++/rvalue)
    * [尾递归优化](./C++/tail-recuresion.md)
    * [编译链接全过程](./C++/whats-compile-and-link.md)
    * [智能指针](./C++/智能指针.md)
* [数据库](./database)
    * [Mysql常识性介绍](./database/intro-to-database.md)
    * [redis](./redis)  
        * [redis数据类型](./redis/redis-type.md)
* [python](./python)
    * [Celery学习笔记](./python/understand-and-use-celery.md)
* [如何设计系列](./how-to-design)
* [网络](./netowrk)
    * [unix中的5种socket IO模型](./netowrk/socket-io-model-in-unix.md)
    * [TCP/IP详解卷一,笔记](./netowrk/TCP-IP-Illustrated-V1)
    * [第0部分 IP地址结构](./netowrk/TCP-IP-Illustrated-V1/0-structure-of-IP-address.md)
    * [第2分 ARP协议](./netowrk/TCP-IP-Illustrated-V1/2-address-resolve-protocol.md)
    * [第9分 TCP协议](./netowrk/TCP-IP-Illustrated-V1/9-master-tcp.md)
    * [测试](./netowrk/TCP-IP-Illustrated-V1/tests)  
       * [TCP stream中的序列号变化](./netowrk/TCP-IP-Illustrated-V1/tests/seq-num-during-tcp-stream)
       * [SO_REUSEPORT-and-SO_RESUEADDR对服务器程序的影响](./netowrk/TCP-IP-Illustrated-V1/tests/SO_REUSEPORT-and-SO_RESUEADDR)
        * [如何理解TCP stream](./netowrk/TCP-IP-Illustrated-V1/tests/understand-stream-of-tcp/README.md)
* [操作系统](oprating-system)
    * [Unix环境高级编程学习记录](./oprating-system/APUE)
    * [Operating Systems: Three Easy Pieces 学习笔记](./oprating-system/OSTEP)
        * [详细](./oprating-system/OSTEP/README.md)
        * [测试](./oprating-system/OSTEP/examples)
            * [race condition](./oprating-system/OSTEP/examples/data-race/README.md)
            * [重定向的实现](./oprating-system/OSTEP/examples/data-race/how_to_redirect.cpp)
            * [线程调度](./oprating-system/OSTEP/examples/thread-schedule/README.md)
    * [其他](./oprating-system/others)
        * [内存地址](./oprating-system/others/memory-addressing.md)
        * [c/c++ 运行时内存结构](./oprating-system/others/memory-structure.md)
        * [unix系统API笔记](./oprating-system/others/unix-programming-api.md)
        * [简短的计组笔记](./oprating-system/others/x86-cpu.md)
* [日常](./daily)
* [其他一](./others)  
    * [CMAKE](./others/cmake)  
        * [语法记录](./others/cmake/how-to-use-cmake.md)
        * [project实例](./others/cmake/mydemo)
* [其他二](flags)
* [其他三](./finance)



# 待解决

|代码|问题|
|-|-|
|C++/vfun-class-model.cpp|观察虚函数表尾失败,未找到type_info object的观察方法|

