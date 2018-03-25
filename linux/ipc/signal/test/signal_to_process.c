/*

    一个进程下挂了多个线程，向这个进程发信号，看看有多少线程能收到。

    只有一个线程能够收到。

 */

#include <pthread.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

void sig_handle(int pid)
{
    printf("用户态下的进程id：%d, "
           "内核态下的进程id：%d,用户态下的进程里的线程id：%d\n",
           getpid(),
           syscall(SYS_gettid),
           pthread_self());
    sleep(10);
}

void work()
{
    while (1)
    {
        // printf("用户态下的进程id：%d, "
        //        "内核态下的进程id：%d,用户态下的进程里的线程id：%d\n",
        //        getpid(),
        //        syscall(SYS_gettid),
        //        pthread_self());
        sleep(5);
    }
}

int main(int argc, char** argv)
{
    signal(SIGINT, sig_handle);

    pthread_t t1;
    pthread_create(&t1, NULL, work, NULL);
    pthread_t t2;
    pthread_create(&t2, NULL, work, NULL);
    pthread_t t3;
    pthread_create(&t3, NULL, work, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}