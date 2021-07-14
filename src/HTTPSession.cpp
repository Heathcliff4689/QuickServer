#include "HTTPSession.h"

HTTPSession::HTTPSession()
    : prase_done(false),
      crlf("\r\n"),
      dcrlf("\r\n\r\n"),
      request(),
      response(),
      msg(),
      send_msg(),
      have_sent(0)
{
}

HTTPSession::~HTTPSession()
{
}

bool HTTPSession::praseHttpRequest(std::string &msg, HttpRequest &request)
{
    int headend = msg.find(dcrlf);
    if (headend == std::string::npos)
    {
        /* not found complete http head */
        prase_done = false;
        return prase_done;
    }
    else
    {
        int checked = 0;
        if (!prase_done)
        {
            /* found http head */
            // read first line
            int pos_crlf = msg.find(crlf, checked);
            std::string first_line = msg.substr(checked, pos_crlf - checked);
            std::stringstream ss(first_line);
            ss >> request.method;
            ss >> request.url;
            ss >> request.version;

            // read remained head
            std::string key, val;
            size_t pos_colon = std::string::npos;

            while (checked != headend)
            {
                key = "";
                val = "";
                pos_colon = std::string::npos;
                checked = pos_crlf + 2;

                pos_crlf = msg.find(crlf, checked);
                std::string line = msg.substr(checked, pos_crlf - checked);
                pos_colon = line.find(":");
                key = line.substr(0, pos_colon);
                val = line.substr(pos_colon + 1);
                request.header.insert(std::make_pair(key, val));
            }
        }

        /* have read http head */
        checked = msg.find(dcrlf) + 4;
        std::string body = msg.substr(checked);
        prase_done = true;
        return prase_done;
    }
}

void HTTPSession::processHttp(HttpRequest &request, HttpResponse &response)
{
    if(request.method == "GET" || request.method == "POST")
    {
        response.version = request.version;
        response.statecode = "200";
        response.body = "OK";

        /* deal with response head */
        std::string key = "Content-type";
        std::string val = "text/html; charset=GB2312";
        response.header.insert(std::make_pair(key, val));

        /*deal with body*/
        response.body = "RESPONSE BODY. ";
    }
    else
    {
        /* Not implement methods. */
    }

    
    /* compose send_msg */
    send_msg += response.version + " " + response.statecode + " " + response.statemsg + crlf;
    for(auto iter: response.header)
    {
        send_msg += iter.first + ":" + iter.second + crlf;
    }
    send_msg += dcrlf;
    send_msg += response.body;

}

void HTTPSession::reset()
{
    prase_done = false;
    have_sent = 0;
    request = std::move(HttpRequest());
    response = std::move(HttpResponse());
    msg = std::move(std::string());
    send_msg = std::move(std::string());
}

int HTTPSession::readRequest(int epfd, int fd)
{
    int ret = readmsg(epfd, fd, msg);

    if(ret == 1)
    {
        /* wait for next EPOLLIN */
        modFd(epfd, fd, EPOLLIN, true);
        return 1;
    }
    else if(ret = -1)
    {
        perror("readmsg error. ");
        removeFd(epfd, fd);
        reset();
        return -1;
    }
    
    bool prased = praseHttpRequest(msg, request);
    
    if(!prased)
    {   
        /* here should send error msg to client. */
        perror("http request non-completed. ");
        removeFd(epfd, fd);
        reset();
        return -1;
    }
    else 
    {   
        /* have read complete head and body */
        processHttp(request, response);
        /* listen write events */
        modFd(epfd, fd, EPOLLOUT, true);
    }
}

int HTTPSession::writeResponse(int epfd, int fd)
{   
    int ret = writemsg(epfd, fd, send_msg, have_sent);
    if(ret == -1)
    {
        /* close connection. */
        perror("writemsg error. ");
        removeFd(epfd, fd);
        reset();
        return -1;
    }
    else if(ret == 1)
    {
        modFd(epfd, fd, EPOLLOUT, true);
        return 1;
    }
    else
    {   
        if(have_sent != send_msg.size())
        {
            std::cout<<"writeResponse done, but send_msg len error. "<<std::endl;
        }

        // modFd(epfd, fd, EPOLLIN, true);
        removeFd(epfd, fd);
        reset();

        return 0;
    }



}