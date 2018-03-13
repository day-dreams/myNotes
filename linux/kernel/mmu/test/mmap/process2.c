#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int                 fd, i;
    void *              mapped;
    unsigned long long *output = malloc(8 * sizeof(unsigned long long));
    //命令行清屏用
    const char clr[]     = {27, '[', '2', 'J', '\0'};
    const char topLeft[] = {27, '[', '1', ';', '1', 'H', '\0'};
    fd                   = open("test", O_RDWR | O_CREAT);

    if ((mapped = (unsigned long long *)mmap(
             NULL, 8 * sizeof(unsigned long long), PROT_READ, MAP_SHARED, fd, 0)) ==
        (void *)-1)
    {
        perror("mmap");
    }

    /* 映射完后, 关闭文件也可以操纵内存 */
    close(fd);

    output = mapped;
    while (1)
    {  //清屏
        printf("%s%s", clr, topLeft);
        printf("************Process 2************\n");
        printf("Current address list:\n");
        for (i = 0; i < 8; i++)
        {
            printf("0x%llx\n", output[i]);
        }
        sleep(1);  //停一秒刷新一下
    }
    return 0;
}
