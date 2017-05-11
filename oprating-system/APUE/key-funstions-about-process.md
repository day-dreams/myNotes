# 一些关于进程控制的重要函数

## 进程环境
### 进程退出

>```cpp
>#include<stdlib.h>
>void exit(int status);
>```
>清理所有文件流,缓冲区中所有数据都会被写道完文件后在清空.

>```cpp
>#include<stdlib.h>
>int atexit(void (*func)(void))
>```
>为exit绑定函数,exit被调用式会首先调用它们

### 环境表

>```cpp
>#include<stdlib.h>
>char *getenv(const char *name);
>```
>

## 进程控制
>```cpp
>#include<unistd.h>
>pid_t getpid();/*当前进程ID*/
>pid_t getppid();/*父进程ID*/
>```
>

>```cpp
>#include<unistd.h>
>pid_t fork();
>```
>创建子进程,两套返回值
> * 文件描述符共享,引用计数加一
> * copy on write
> * 先后执行顺序不确定

>```cpp
>#include<unistd.h>
>pid_t vfork();/*保证子进程先运行,父进程在子进程exec或exit后才能运行*/
>```

>```cpp
>#include<sys/wait.h>
>pid_t wait(int *staloc);/*无差别等待子进程结束,陷入阻塞;子进程结束状态在staloc中*/
>pid_t wait(pid_t pid,int *staloc,int options);/*等待某个子进程结束,可根据选项选择是否阻塞*/
>```
