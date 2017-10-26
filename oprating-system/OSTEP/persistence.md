Operating Systems: Three Easy Pieces
=====================================

这是OSTEP的持续性部分章节的笔记.

<!-- TOC -->

- [1. chapter 36, I/O Devices](#1-chapter-36-io-devices)
    - [1.1. System Architecture:现代计算机的逻辑结构](#11-system-architecture现代计算机的逻辑结构)
    - [1.2. A Canonical Device:IO设备的组成是什么样的](#12-a-canonical-deviceio设备的组成是什么样的)
    - [1.3. Canonical Protocol:简单的IO协议](#13-canonical-protocol简单的io协议)
    - [1.4. Canonical Protocol:带有中断功能的IO协议](#14-canonical-protocol带有中断功能的io协议)
        - [1.4.1. 不是所有IO设备都适合使用中断机制.](#141-不是所有io设备都适合使用中断机制)
        - [1.4.2. 中断机制的维护可能会抢占用户线程的时间片](#142-中断机制的维护可能会抢占用户线程的时间片)
        - [1.4.3. 中断组合(coalescing)](#143-中断组合coalescing)
    - [1.5. Canonical Protocol:更高效的DMS方式](#15-canonical-protocol更高效的dms方式)
    - [1.6. Methods of Device Interactio:设备的寻址方式](#16-methods-of-device-interactio设备的寻址方式)
    - [1.7. Fitting Into OS: Device Driver:外包一样的驱动](#17-fitting-into-os-device-driver外包一样的驱动)
- [2. chapter 37, Hard Disk Devices](#2-chapter-37-hard-disk-devices)
- [3. chapter 38, Redundant Arrays of Inexpensive Disks(RAIDS)](#3-chapter-38-redundant-arrays-of-inexpensive-disksraids)
- [4. chapter 39, Interlude: File and Directories](#4-chapter-39-interlude-file-and-directories)
- [5. chapter 40, File System Implementation](#5-chapter-40-file-system-implementation)
- [6. chapter 41, Locality and The Fast Fils System](#6-chapter-41-locality-and-the-fast-fils-system)
- [7. chapter 42, Crash Consistency:FSCK and Journaling](#7-chapter-42-crash-consistencyfsck-and-journaling)
- [8. chapter 43, Log-structured File Systems](#8-chapter-43-log-structured-file-systems)
- [9. chapter 44, Data Integrith and Protection](#9-chapter-44-data-integrith-and-protection)
- [10. chapter 47, Distributed Systems](#10-chapter-47-distributed-systems)
- [11. chapter 48, Sun's Network File System(NFS)](#11-chapter-48-suns-network-file-systemnfs)
- [12. chapter 49, The Andrew File System(AFS)](#12-chapter-49-the-andrew-file-systemafs)

<!-- /TOC -->
# 1. chapter 36, I/O Devices

## 1.1. System Architecture:现代计算机的逻辑结构

简单介绍一下现代计算机的系统结构.

现在的计算机由几个重要的逻辑部件连接在一起:CPU,Memory,Bus,I/O设备.总线是贯穿整个结构的概念,它分级连接了多个逻辑部件.按传输速度从高到底,Memory Bus连接CPU和内存,General I/O Bus连接一些高速IO设备(图形显卡,DMA等),Perpheral I/O Bus连接低速IO设备(SATA磁盘,鼠标等).

示意图如下.

![system architecture](./system-architecture.png)

## 1.2. A Canonical Device:IO设备的组成是什么样的

一个标准的IO设备由Interface和Internal Structure组成.接口具有一些寄存器,定义了操作系统和本设备的交互方式(那个寄存器用来输入,哪个用来输出,哪个用来查询状态,哪个用来控制状态等);内部结构可能具有一个小CPU,内存等各种芯片,负责设备逻辑的具体实现.

![io structure](./standard-io-structure.png)

## 1.3. Canonical Protocol:简单的IO协议

IO接口大概有这几种寄存器:Status,Command,Data,分别有不同的用途.

我们需要有一份IO协议,来规定IO应该如何接受工作任务,CPU应该如何提交工作命令,也就是交互方式.这里有一份简化的交互协议,大概经过几个步骤:

* CPU等待IO设备空闲
* 等待完成后,将数据写入到IO设备的接口的Data寄存器中
* 将命令写入Io设备的接口的Command寄存器中,IO开始工作
* CPU检查IO设备接口的状态,查看工作是否成功完成.

这种简化的协议显然效率太高,CPU经常出现轮讯操作.

## 1.4. Canonical Protocol:带有中断功能的IO协议

不再多说,已经很熟悉了.只介绍一两点.

### 1.4.1. 不是所有IO设备都适合使用中断机制.  

对于速度较快的设备,中断带来的上下文切换开销是不容忽视的,不如直接使用上面简单的轮训方式.也可将两种方式混合在一起,就像linux下的futex,spin wait一段时间后陷入Bock.  

### 1.4.2. 中断机制的维护可能会抢占用户线程的时间片

如果在某个场景中,中端频繁出现,可能会导致没有机会去真正服务这些IO事件.比如网络,数据包不断到来,如果频繁陷入中断处理,就无法快速对数据包作出响应.这样的情景,就需要更多的干预措施,让系统真正处理数据包,做相应的计算任务.

### 1.4.3. 中断组合(coalescing)

由于中断会带来额外开销,我们有理由尽量优化这种开销.可以通过中断组合的方式,把一段时间内到来的中断聚集成同一个中断事件,让系统一次性处理.

比如在网络环境中,当一定数量的数据包到来,才引发中断,而不是一个包一个中断.

## 1.5. Canonical Protocol:更高效的DMS方式

前两种IO协议中,都需要CPU将输出数据从IO复制到内存,或者是把输入数据从内存复制到IO设备,显然是比较耗费时间的.

DirectMemoryAccess就是针对这种情况作出的优化措施,也不多说.

## 1.6. Methods of Device Interactio:设备的寻址方式

这个其实就是IO与CPU交互的两种方式,定义了CPU如何向CPU发起请求.基本方式还是通过指令来完成,区别在于是采用特有的指令寻址方式还是统一的寻址方式.

就是IO统一编址和非统一编址,也不再多说,几个老师已经把这个分析了无数次.

## 1.7. Fitting Into OS: Device Driver:外包一样的驱动

因为IO设备众多,操作系统懒得为每个IO设备提供操作指令代码.于是OS用一种外包的形式,抽象出了设备驱动.IO设备的生产厂家按照不同操作系统的不同规范,自己写好设备驱动(因为只有他们自己最清楚细节),提供给OS使用.

值得注意的是,**大部分操作系统的代码是关于设备驱动的**(Linux的代码中有70%是和设备却动相关的).

# 2. chapter 37, Hard Disk Devices



# 3. chapter 38, Redundant Arrays of Inexpensive Disks(RAIDS)



# 4. chapter 39, Interlude: File and Directories



# 5. chapter 40, File System Implementation



# 6. chapter 41, Locality and The Fast Fils System



# 7. chapter 42, Crash Consistency:FSCK and Journaling



# 8. chapter 43, Log-structured File Systems



# 9. chapter 44, Data Integrith and Protection



# 10. chapter 47, Distributed Systems



# 11. chapter 48, Sun's Network File System(NFS)



# 12. chapter 49, The Andrew File System(AFS)


