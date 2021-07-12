#include "utils.h"

int initSocket(const char *ip, int port)
{
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    if (lfd < 0)
    {
        perror("socket error. ");
    }

    sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = PF_INET;
    serv.sin_port = htons(port);
    inet_pton(PF_INET, ip, &serv.sin_addr);

    int ret = 0;

    ret = bind(lfd, (struct sockaddr *)&serv, sizeof(serv));
    if (ret < 0)
    {
        perror("bind error. ");
    }

    ret = listen(lfd, MAX_FD);
    if (ret < 0)
    {
        perror("listen error. ");
    }


    return lfd;
}

void setnonblocking(int fd)
{
    int old_op = fcntl(fd, F_GETFL);
    int new_op = old_op | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_op);
}

void addFd(int epfd, int fd, bool oneshoot)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (oneshoot)
    {
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
}

void removeFd(int epfd, int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

void modFd(int epfd, int fd, int EVENT, bool oneshoot = true)
{
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = EVENT | EPOLLET | EPOLLRDHUP;
    if (oneshoot)
    {
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}