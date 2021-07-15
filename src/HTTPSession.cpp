#include "HTTPSession.h"

HTTPSession::HTTPSession()
    : prase_done(false),
      crlf("\r\n"),
      dcrlf("\r\n\r\n"),
      request(),
      response(),
      msg(),
      send_msg(),
      have_sent(0),
      need_to_read(0)
{
}

HTTPSession::~HTTPSession()
{
}

bool HTTPSession::praseHttpRequest(std::string &msg, HttpRequest &request, int &need_to_read)
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
            checked = pos_crlf + 2;

            // read remained head
            std::string key, val;
            size_t pos_colon = std::string::npos;

            while (checked != headend + 2)
            {
                key = "";
                val = "";
                pos_colon = std::string::npos;

                pos_crlf = msg.find(crlf, checked);
                std::string line = msg.substr(checked, pos_crlf - checked);
                pos_colon = line.find(":");
                key = line.substr(0, pos_colon);
                val = line.substr(pos_colon + 2);
                request.header.insert(std::make_pair(key, val));

                checked = pos_crlf + 2;
            }
        }

        /* have read http head */
        checked = msg.find(dcrlf) + 4;
        std::string body = msg.substr(checked);
        prase_done = true;

        if (request.header.count("Content-Length") != 0)
        {
            need_to_read = headend + 4 + std::stoi(request.header["Content-Length"]);
        }
        else
        {
            need_to_read = headend + 4;
        }
        return prase_done;
    }
}

void HTTPSession::processHttp(HttpRequest &request, HttpResponse &response)
{
    // if(request.method == "GET" || request.method == "POST")
    if (request.method == "GET")
    {
        response.version = request.version;
        response.statecode = "200";
        response.body = "OK";

        /* deal with response head */
        std::string key = "Content-type";
        std::string val = "text/html; charset=GB2312";
        response.header.insert(std::make_pair(key, val));

        /*deal with body*/
        response.body = msg;
    }
    else
    {
        /* Not implement methods. */
    }

    /* compose send_msg */
    send_msg += response.version + " " + response.statecode + " " + response.statemsg + crlf;
    for (auto iter : response.header)
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
    need_to_read = 0;
    request = std::move(HttpRequest());
    response = std::move(HttpResponse());
    msg = std::move(std::string());
    send_msg = std::move(std::string());
}

int HTTPSession::readRequest(int epfd, int fd)
{
    bool prased = false;
    std::string tmsg;
    if (need_to_read > 0)
    {
        /* twice, request non-completed */
        int ret = readmsg(epfd, fd, tmsg, need_to_read, 0);
        request.body += tmsg;
        if (ret == 1 && need_to_read > 0)
        {
            modFd(epfd, fd, EPOLLIN, true);
            return 1;
        }
        else if (ret = -1)
        {
            perror("readmsg error. ");
            removeFd(epfd, fd);
            reset();
            return -1;
        }
    }
    else
    {
        /* attemp to get need to read */
        int ret = readmsg(epfd, fd, tmsg, need_to_read, MSG_PEEK);

        if (ret == 1)
        {
            /* wait for next EPOLLIN */
            do
            {
                prased = praseHttpRequest(tmsg, request, need_to_read);
                if (prased)
                {
                    ret = readmsg(epfd, fd, msg, need_to_read, 0);
                    if (need_to_read <= 0)
                    {
                        break;
                    }
                }
                modFd(epfd, fd, EPOLLIN, true);
                return 1;
            } while (0);
        }
        else if (ret = -1)
        {
            perror("readmsg error. ");
            removeFd(epfd, fd);
            reset();
            return -1;
        }
        else
        {
            ret = readmsg(epfd, fd, msg, need_to_read, 0);
        }
    }

    prased = praseHttpRequest(msg, request, need_to_read);

    if (!prased)
    {
        /* here should send error msg to client. */
        perror("http request non-completed. ");
        removeFd(epfd, fd);
        reset();
        return -1;
    }
    else
    {
        std::cout << std::this_thread::get_id() << ": readRequest.. " << std::endl;
        /* have read complete head and body */
        processHttp(request, response);
        /* listen write events */
        modFd(epfd, fd, EPOLLOUT, true);
    }
}

int HTTPSession::writeResponse(int epfd, int fd)
{
    std::cout << std::this_thread::get_id() << ": writeResponse.. " << std::endl;
    int ret = writemsg(epfd, fd, send_msg, have_sent);
    if (ret == -1)
    {
        /* close connection. */
        perror("writemsg error. ");
        removeFd(epfd, fd);
        reset();
        return -1;
    }
    else if (ret == 1)
    {
        modFd(epfd, fd, EPOLLOUT, true);
        return 1;
    }
    else
    {
        if (have_sent != send_msg.size())
        {
            std::cout << "writeResponse done, but send_msg len error. " << std::endl;
        }

        // modFd(epfd, fd, EPOLLIN, true);
        removeFd(epfd, fd);
        reset();

        return 0;
    }
}