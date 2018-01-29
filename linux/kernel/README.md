这个目录主要是一些关于linux内核的学习笔记.由于linux源代码过于复杂,这个文档会记录一些全局通用的知识概念.

本目录下一切linux源代码均基于linux-2.6.0

<!-- TOC -->

- [笔记目录](#笔记目录)
- [参考资料](#参考资料)
- [关于Linux本身](#关于linux本身)
    - [重要性](#重要性)
    - [版本号](#版本号)
    - [学习方法](#学习方法)
- [代码阅读工具](#代码阅读工具)
    - [内核函数调用可视化工具](#内核函数调用可视化工具)

<!-- /TOC -->

# 笔记目录

按照推荐学习顺序排序.

[文件子系统](vfs/README.md)

[网络子系统](network/README.md)

内存管理子系统

进程调度子系统

进程通信子系统

# 参考资料

* [Linux kernel map](http://makelinux.net/kernel_map/)

* 深入理解LINUX内核

    这本书我看不太明白.

* Linux内核设计与实现,第三版.

    能看懂,挺好的.
    
* [Linux Kernel Networking: Implementation and Theory](https://www.amazon.com/gp/product/143026196X/ref=as_li_ss_il?ie=UTF8&camp=1789&creative=390957&creativeASIN=143026196X&linkCode=as2&tag=makelinux-20) 

    专门讲网络子系统,是我想要了解的部分.111.

# 关于Linux本身

## 重要性

我觉得服务器端开发者多少都应该学习一下,如果有时间的话. 

## 版本号

linux版本号由3或4个部分组成.

* 主版本号
* 副版本号(偶数代表稳定版,基数代表开发版)
* 修订版本号
* 稳定版本号


## 学习方法

只看当前最感兴趣的部分,一定不要迷失在代码里.

# 代码阅读工具

## 内核函数调用可视化工具

**我并没有安装成功,一下步骤仅供参考!**

[一款开源的可视化工具](https://github.com/vonnyfly/kernel_visualization),这个工具不是那么的必须,如果你觉得有必要,可以折腾试试.

使用步骤:

* 安装systemtap

* 安装内核debug符号包

如果直接这样安装:```apt-get install linux-image-`uname -r`-dbgsym```,是找不到程序包的.必须自己去[这里](http://ddebs.ubuntu.com/pool/main/l/linux/)找到相应的文件.


我的机器是ubuntu16.04,x86_64架构.
```
moon@moon-Think:~$ uname -a
    Linux moon-Think 4.4.0-109-generic #132-Ubuntu SMP Tue Jan 9 19:52:39 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux

```

下载的文件是**linux-image-4.4.0-109-generic-dbgsym\_4.4.0-109.132_amd64.ddeb**,文件大小474MiB,下载速度在200KB/s左右,很费劲.

下载完成后安装:
```bash
sudo dpkg -i linux-image-4.4.0-109-generic-dbgsym_4.4.0-109.132_amd64.ddeb 
```

* 安装graphvize

* 生成一个tap文件

比如:
```gen_stap.sh -m iscsi_target_mod.ko,target_core_mod.ko,target_core_file.ko,target_core_pscsi.ko -e fd_do_rw```

* 使用tap文件来生成

