#include <stdio.h>

/*

    文件描述符是复用的!

 */

int main(int argc, char** argv)
{
    int fd1 = -1, fd2 = -1, fd3 = -1;

    fd1 = open("test_fd.c", 'r');
    fd2 = open("test_fd.c", 'r');

    printf("fd1 fd2 fd3: %d %d %d\n", fd1, fd2, fd3);

    close(fd1);

    printf("fd1 fd2 fd3: %d %d %d\n", fd1, fd2, fd3);

    fd3 = open("test_fd.c", 'r');

    printf("fd1 fd2 fd3: %d %d %d\n", fd1, fd2, fd3);

    return 0;
}