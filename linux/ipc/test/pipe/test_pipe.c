#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int pipefd[2];

    int err = pipe(pipefd);
    if (err)
    {
        printf("err during pip()\n");
        return -1;
    }

    char obuffer[] = "helloworld";
    char ibuffer[11];

    if (fork() == 0)
    {
        /* 子进程 */
        write(pipefd[1], (void*)obuffer, 11);
        printf("written by child\n");
    }
    else
    {
        /* 父进程 */
        read(pipefd[0], (void*)ibuffer, 11);
        printf("read from father:\n");
        write(0, (void*)ibuffer, 11);
    }

    return 0;
}