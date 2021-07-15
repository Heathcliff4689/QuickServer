#pragma once

#include <string>
#include <sstream>
#include <map>
#include "utils.h"
#include "ThreadPool.h"

class HttpRequest
{
public:
    std::string method;
    std::string url;
    std::string version;
    std::map<std::string, std::string> header;
    std::string body;
};

class HttpResponse
{
public:
    std::string version;
    std::string statecode;
    std::string statemsg;
    std::map<std::string, std::string> header;
    std::string body;
};

class HTTPSession
{
private:
    bool prase_done;
    const std::string crlf, dcrlf;

public:
    HttpRequest request;
    HttpResponse response;
    std::string msg;
    std::string send_msg;
    int have_sent;
    int need_to_read;

public:
    HTTPSession();
    ~HTTPSession();

    /* after response calling this. */
    void reset();

    int readRequest(int epfd, int fd);
    int writeResponse(int epfd, int fd);

private:
    // return true while have read complete http head, otherwise false.
    bool praseHttpRequest(std::string &msg, HttpRequest &request, int& need_to_read);
    void processHttp(HttpRequest &request, HttpResponse &response);

};
