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
    const char* const PATHNAME = "./forfifo";  //关联的路径名
    const char* const BUFF     = "Fifo Write Test.\r\n";
    int               fd;

    if ((mkfifo(PATHNAME, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) && (errno != EEXIST))
    {
        printf("can't create %s\r\n", PATHNAME);
        return -1;
    }

    fd = open(PATHNAME, O_WRONLY, 0);
    write(fd, BUFF, strlen(BUFF));

    return 0;
}