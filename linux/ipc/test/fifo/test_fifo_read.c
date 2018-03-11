#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    const int         MAXLEN   = 1024;
    const char* const PATHNAME = "./forfifo";  //关联的路径名
    ssize_t           n;
    int               fd;
    char              readbuff[MAXLEN];
    memset(readbuff, 0x00, sizeof(readbuff));  //初始化
    if ((mkfifo(PATHNAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) && (errno != EEXIST))
    {
        printf("can't create %s\r\n", PATHNAME);
        return -1;
    }

    fd = open(PATHNAME, O_RDONLY, 0);

    while ((n = read(fd, readbuff, sizeof(readbuff))) > 0)
    {
        write(STDOUT_FILENO, readbuff, n);  //标准输出
    }
    return 0;
}