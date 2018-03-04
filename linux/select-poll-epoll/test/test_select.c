#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define BUF_SIZE 30

int main(int argc, char* argv[])
{
    fd_set         reads, temps;
    int            result, str_len;
    char           buf[BUF_SIZE];
    struct timeval timeout;

    FD_ZERO(&reads);
    FD_SET(0, &reads);  //监视文件描述符0的变化, 即标准输入的变化

    while (1)
    {
        /*将准备好的fd_set变量reads的内容复制到temps变量，因为调用select函数后，除了发生变化的fd对应位外，剩下的所有位
        都将初始化为0，为了记住初始值，必须经过这种复制过程。
        */
        temps = reads;

        //每次select都要设置超时,因而select会修改timeout结构体
        timeout.tv_sec  = 5;
        timeout.tv_usec = 0;

        //调用select函数.
        result = select(1, &temps, 0, 0, &timeout);

        //若有控制台输入数据，则返回大于0的整数，如果没有输入数据而引发超时，返回0.
        if (result == -1)
        {
            perror("select() error");
            break;
        }
        else if (result == 0)
        {
            puts("timeout");
        }
        else
        {
            //读取数据并输出
            if (FD_ISSET(0, &temps))
            {
                str_len      = read(0, buf, BUF_SIZE);
                buf[str_len] = 0;
                printf("%s", buf);
            }
        }
    }

    return 0;
}