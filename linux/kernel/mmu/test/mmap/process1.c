#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

struct vhost_virtqueue
{
    unsigned long long *desc;
    int                 num;
    unsigned long long  desc_phys;
};

int main(int argc, char **argv)
{
    int fd;
    //定义描述符
    int                 i, j = 0;
    unsigned long long *mapped;
    //命令行清屏用
    const char clr[]     = {27, '[', '2', 'J', '\0'};
    const char topLeft[] = {27, '[', '1', ';', '1', 'H', '\0'};

    if ((fd = open("test", O_RDWR | O_CREAT)) < 0)
    {
        perror("open");
    }

    struct vhost_virtqueue vq;
    vq.desc_phys = 0x7ff40000;
    vq.num       = 8;
    write(fd, "", 1);
    //先向文件里随便写什么，不然直接mmap空文件会报bus error的错

    if ((vq.desc = (unsigned long long *)mmap(NULL,
                                              vq.num * sizeof(unsigned long long),
                                              PROT_READ | PROT_WRITE,
                                              MAP_SHARED | MAP_POPULATE,
                                              fd,
                                              0)) == (void *)-1)
    {  //映射共享内存，本来mmap返回值是void *，强制转换我们自己要的类型
        perror("mmap");
    }

    /* 映射完后, 关闭文件也可以操纵内存 */
    close(fd);

    for (i = 0; i < vq.num; i++)
    {
        *(vq.desc + i) = vq.desc_phys + i;
        printf("0x%llx\n", *(vq.desc + i));
    }
    //往共享内存中写入地址，8个，依次加一

    //为了让现象更明显，让该程序一直执行对该共享内存写数据，地址不断增加
    while (1)
    {
        printf("%s%s", clr, topLeft);
        printf("************Process 1************\n");
        printf("Current address list:\n");
        j++;
        for (i = 0; i < vq.num; i++)
        {
            *(vq.desc + i) = vq.desc_phys + i + j;
            printf("0x%llx\n", *(vq.desc + i));
        }
        sleep(1);
    }

    return 0;
}