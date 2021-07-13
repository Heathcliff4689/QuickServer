#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string>


const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int READBUFSIZ = 4096;

int initSocket(const char* ip, int port);

void setnonblocking(int fd);

void addFd(int epfd, int fd, bool oneshoot);

void removeFd(int epfd, int fd);

void modFd(int epfd, int fd, int EVENT, bool oneshoot);

int readmsg(int fd, std::string& msg);