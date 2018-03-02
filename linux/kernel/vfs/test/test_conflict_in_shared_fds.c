#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/*
    文件对象本身没有同步控制.如果多个进程共同读/写一个文件, 会造成dirtry
   write/read.

 */
int main(int argc, char** argv)
{
    int fd = open(argv[1], O_RDWR);

    char buffer[1024] = "";

    fork();

    read(fd, (void*)buffer, 10);

    printf("%s\n", buffer);

    return 0;
}