#include <iostream>

#include "utils.h"

int main()
{
    const char *ip = "127.0.0.1";
    const int port = 8080;
    int lfd = initSocket(ip, port);

    epoll_event events[MAX_EVENT_NUMBER];

    int epfd = epoll_create(5);
    if (epfd < 0)
    {
        perror("epoll_create error. ");
    }

    addFd(epfd, lfd, false);

    while (1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        if ((nfds < 0) && (errno != EINTR))
        {
            printf("epoll failed. \n");
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == lfd)
            {
                struct sockaddr_in client;
                socklen_t client_len = sizeof(client);
                int connfd = accept(lfd, (struct sockaddr *)&client, &client_len);
                if (connfd < 0)
                {
                    printf("accept errno is: %d\n", errno);
                    continue;
                }
                else
                {
                    addFd(epfd, connfd, true);
                }
            }
            else
            {
                /* other actions */
            }
        }
    }
}