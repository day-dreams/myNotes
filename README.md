master分支太乱了，暂时不想整理，所以重新开一个分支来放笔记

<!-- TOC -->

- [编程语言](#编程语言)
    - [C++](#c)
    - [Python](#python)
- [TCP/IP](#tcpip)
- [Linux](#linux)
    - [编程](#编程)
    - [内核](#内核)
        - [文件子系统](#文件子系统)
        - [网络子系统](#网络子系统)
        - [内存管理子系统](#内存管理子系统)
        - [进程调度子系统](#进程调度子系统)
        - [进程通信子系统](#进程通信子系统)
        - [其他](#其他)
- [数据库](#数据库)
- [Web](#web)
- [算法数据结构](#算法数据结构)

<!-- /TOC -->

# 编程语言

## C++

- [x] [STL中迭代器的实现及失效问题](cpp/iterator/README.md)

- [x] [emplace_pack VS push_back](cpp/emplace-vs-insert/README.md)

- [x] [RVO VS std::move](cpp/RVO-vs-move/README.md)

- [x] [内存的分配和释放(malloc/free,new/delete operator,new/delete expression)](cpp/memory-control/README.md)

- [ ] 异常的实现方式

- [ ] 默认,复制,拷贝构造函数的原理与使用

- [ ] 完美转发的实现方式

- [ ] 引用和指针的区别(汇编等角度)

- [ ] extern关键字对于编译流程的影响

- [ ] static关键字对于编译流程的影响

- [ ] 智能指针和线程安全

- [ ] 为什么说allocator是无用的设计(参考陈硕的博客)

## Python

- [ ] 迭代器,生成器,装饰器的使用

- [ ] 函数式编程

# TCP/IP

- [ ] TCP的so\_reuseport和so\_reuseaddr

- [ ] TCP的流量控制(传输速率在不同网络状态下的变化)


# Linux

## 编程

- [ ] 进程间通信(如何使用signal,pipe,消息队列等)

- [ ] epoll和select的使用与区别

## 内核


### 文件子系统

- [ ] 文件描述符的实现方式(从普通的文件io和网络io等角度去讨论)


### 网络子系统

- [ ] epoll和select的实现方式

- [ ] UDP socket在收包发包时的具体细节(为什么可以用一个socket向很多个服务器发包再使用select收包)

- [ ] tcp的关键实现函数

- [ ] udp的关键实现函数

### 内存管理子系统

- [ ] 内核的内存管理机制

- [ ] malloc,free的实现方式

### 进程调度子系统

- [ ] 线程的实现方式(从stuct_task的角度去分析group,thread,process)

###　进程通信子系统

### 其他

- [ ] 如何编译内核

# 数据库

- [ ] Mysql索引的使用和实现原理

- [ ] 事务提交的注意事项和实现细节

# Web

- [ ] Restful风格api的设计

# 算法数据结构


- [ ] 红黑树,B树的大致工作原理和特性

