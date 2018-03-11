#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int pollfd = epoll_create(1);

    struct epoll_event e, revents[5];
    e.data.fd = 2;
    e.events  = EPOLLIN; /* default as Level Trigger */
    // e.events  = EPOLLIN | EPOLLET; /* set as Edge Trigger */

    epoll_ctl(pollfd, EPOLL_CTL_ADD, 2, &e);

    while (true)
    {
        int nfds = epoll_wait(pollfd, revents, 5, 1000);

        if (nfds == 0)
            continue;

        std::cout << "fd to handle:" << nfds << std::endl;

        for (int i = 0; i != nfds; ++i)
        {
            auto fd = revents[i].data.fd;

            static char buffer[1024] = "";

            read(fd, buffer, 1);
            std::cout << "from fd " << fd << ':' << std::string(buffer) << std::endl;
        }
    }

    return 0;
}