#pragma once

#include <string>
#include <sstream>
#include <map>

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

public:
    HTTPSession();
    ~HTTPSession();

    void reset();

    // return true while have read complete http head, otherwise false.
    bool praseHttpRequest(std::string &msg, HttpRequest &request);
    void processHttp(HttpRequest &request, HttpResponse &response);
};
