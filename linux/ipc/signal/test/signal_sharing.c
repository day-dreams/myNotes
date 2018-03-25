/*
    父子进程共享一份sighandle表，copy on write
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int childpid;  //子程序进程ID号

void when_sigint2()
{
    printf("in siging handle 2\n");
}

void when_sigint1()
{
    printf("in siging handle 1\n");
    if (childpid > 0)
        signal(SIGINT, when_sigint2);
}

int main()
{
    signal(SIGINT, when_sigint1);  //当接收到SIGINT信号时，调用when_sigint函数

    if ((childpid = fork()) > 0)  //父进程
    {
        while (1)
            pause();  //将父进程暂停下来，等待SIGINT信号到来
    }
    else if (childpid == 0)  //子进程
    {
        while (1)
            pause();
    }
    else
        printf("fork()函数调用出现错误！/n");
    return 0;
}
