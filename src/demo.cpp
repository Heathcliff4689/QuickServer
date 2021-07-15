#include <iostream>
#include <vector>

#include "HTTPSession.h"
#include "ThreadPool.h"

int main()
{
    const char *ip = "127.0.0.1";
    const int port = 8080;

    addsig( SIGPIPE, SIG_IGN, true);

    int lfd = initSocket(ip, port);

    epoll_event events[MAX_EVENT_NUMBER];
    std::vector<HTTPSession> Sess(MAX_FD);
    ThreadPool pool(4);

    int epfd = epoll_create(5);
    if (epfd < 0)
    {
        perror("epoll_create error. ");
    }

    // lfd is  not ET.
    epoll_event ev;
    ev.data.fd = lfd;
    ev.events = EPOLLIN | EPOLLRDHUP;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    setnonblocking(lfd);

    std::cout<<"listening..\n";
    while (1)
    {   
        
        int nfds = epoll_wait(epfd, events, MAX_EVENT_NUMBER, -1);
        if ((nfds < 0) && (errno != EINTR))
        {
            std::cout<<"epoll failed. \n";
            break;
        }

        for (int i = 0; i < nfds; ++i)
        {
            int sockfd = events[i].data.fd;
            if (sockfd == lfd)
            {   
                // std::cout<<"lfd activating..\n";
                struct sockaddr_in client;
                socklen_t client_len = sizeof(client);
                int connfd = accept(lfd, (struct sockaddr *)&client, &client_len);
                if (connfd < 0)
                {
                    std::cout<<"accept errno is: "<<errno<<"\n";
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
                    // std::cout<<"EPOLLIN activating..\n";
                    pool.addTask(std::bind(
                        &HTTPSession::readRequest,
                        &Sess[sockfd],
                        epfd,
                        sockfd
                        ));
                    // Sess[sockfd].readRequest(epfd, sockfd);
                    
                }
                else if(events[i].events & EPOLLOUT)
                {
                    /* write */
                    // std::cout<<"EPOLLOUT activating..\n";
                    pool.addTask(std::bind(
                        &HTTPSession::writeResponse,
                        &Sess[sockfd],
                        epfd,
                        sockfd
                    ));
                    // Sess[sockfd].writeResponse(epfd, sockfd);
                }
                else
                {
                    /* others */
                    std::cout<<"undefined events in epfd. "<<std::endl;
                }

            }
        }
    }
}