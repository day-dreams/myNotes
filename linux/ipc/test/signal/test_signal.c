#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void sig_handle(int data)
{
    printf("recv signal, details:%d\n", data);
    exit(0);
}

int main(int argc, char** argv)
{
    sigset(SIGINT, sig_handle);
    while (1)
    {
    }

    return 0;
}