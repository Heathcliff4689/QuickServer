#include <iostream>
#include <vector>

#include "utils.h"
#include "HTTPSession.h"
#include "ThreadPool.h"

int main()
{
    const char *ip = "127.0.0.1";
    const int port = 8080;
    int lfd = initSocket(ip, port);

    epoll_event events[MAX_EVENT_NUMBER];
    std::vector<HTTPSession> Sess(MAX_FD);
    ThreadPool pool;

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
                if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
                {
                    /* errors */
                    removeFd(epfd, sockfd);
                    Sess[sockfd].reset();
                }
                else if(events[i].events & EPOLLIN)
                {
                    /* read */
                    if(readmsg(sockfd, Sess[sockfd].msg) != 1)
                    {
                        /* read finish or error*/
                        removeFd(epfd, sockfd);
                    }

                    pool.addTask(std::bind(
                        &HTTPSession::praseHttpRequest, &Sess[sockfd], 
                        Sess[sockfd].msg,
                        Sess[sockfd].request
                    ));
                    
                }
                else if(events[i].events & EPOLLOUT)
                {
                    /* write */
                }
                else
                {
                    /* others */
                }

            }
        }
    }
}