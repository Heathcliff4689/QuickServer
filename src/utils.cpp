#include "utils.h"

void addsig(int sig, void(handler)(int), bool restart)
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
    {
        sa.sa_flags |= SA_RESTART;
    }

    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int initSocket(const char *ip, int port)
{
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    if (lfd < 0)
    {
        perror("socket error. ");
        exit(1);
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
        exit(1);
    }

    ret = listen(lfd, MAX_FD);
    if (ret < 0)
    {
        perror("listen error. ");
        exit(1);
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

int readmsg(int epfd, int fd, std::string &msg, int &need_to_read, int flags)
{
    int nrds = 0;
    if (need_to_read == 0)
    {
        char readbuf[READBUFSIZ];
        while (1)
        {

            bzero(readbuf, sizeof(readbuf));
            nrds = recv(fd, readbuf, READBUFSIZ, flags);
            if (nrds == 0)
            {
                return 0;
            }
            else if (nrds == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    modFd(epfd, fd, EPOLLIN, true);
                    break;
                }

                return -1;
            }
            else
            {

                std::string tmp(readbuf);
                msg += tmp;
                // continue read
                break;
            }
        }
    }
    else
    {
        while (1)
        {
            char readbuf[need_to_read];
            bzero(readbuf, sizeof(readbuf));
            nrds = recv(fd, readbuf, need_to_read, flags);
            if (nrds == 0)
            {
                return 0;
            }
            else if (nrds == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    modFd(epfd, fd, EPOLLIN, true);
                    break;
                }

                return -1;
            }
            else
            {
                need_to_read -= nrds;
                std::string tmp(readbuf);
                msg += tmp;
                // continue read
                if (need_to_read <= 0)
                {
                    return 0;
                }
            }
        }
    }

    /* wait for next EPOLLIN */
    return 1;
}

int writemsg(int epfd, int fd, const std::string &msg, int &have_sent)
{
    const char *msg_r = msg.c_str();
    int msg_len = msg.size() - have_sent;
    int nwts = 0, nrds = 0;

    while (nwts < msg_len)
    {
        nrds = send(fd, msg_r + have_sent + nwts, msg_len - nwts, 0);
        if (nrds >= 0)
        {
            nwts += nrds;
        }
        else if (nrds == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                modFd(epfd, fd, EPOLLOUT, true);
                have_sent = nwts;
                return 1;
            }
            return -1;
        }
    }

    have_sent = nwts;
    return 0;
}