使用mmap进程通信的例子
===

注意,如果没有开root,那么创建的文件由于权限问题无法被另一个进程打开.

```bash

gcc process1.c -o 1
gcc process2.c -o 2

sudo su
./1

# 打开另一个终端
sudu su
./2

```