#include "HTTPSession.h"

#include <exception>

HTTPSession::HTTPSession()
    : prase_done(false),
      keep_alive(false),
      crlf("\r\n"),
      dcrlf("\r\n\r\n"),
      request(),
      response(),
      recv_msg(),
      send_msg()
{
}

HTTPSession::~HTTPSession()
{
}

bool HTTPSession::praseHttpRequest(std::string &recv_msg, HttpRequest &request)
{
    int headend = recv_msg.find(dcrlf);
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
            int pos_crlf = recv_msg.find(crlf, checked);
            std::string first_line = recv_msg.substr(checked, pos_crlf - checked);
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

                pos_crlf = recv_msg.find(crlf, checked);
                std::string line = recv_msg.substr(checked, pos_crlf - checked);
                pos_colon = line.find(":");
                key = line.substr(0, pos_colon);
                val = line.substr(pos_colon + 2);
                request.header.insert(std::make_pair(key, val));

                checked = pos_crlf + 2;
            }
        }

        /* have read http head */
        if (request.header.count("Connection") && request.header["Connection"] == "Keep-Alive")
        {
            keep_alive = true;
        }

        checked = recv_msg.find(dcrlf) + 4;
        std::string body = recv_msg.substr(checked);
        prase_done = true;
        return prase_done;
    }
}

void HTTPSession::processHttp(HttpRequest &request, HttpResponse &response)
{
    if (request.method == "GET" || request.method == "POST")
    {
        response.version = request.version;
        response.statecode = "200";
        response.statemsg = "OK";

        /* deal with response head */
        std::string key = "Content-type";
        std::string val = "text/html; charset=utf-8";
        response.header.insert(std::make_pair(key, val));

        key = "Connection";
        val = "Keep-Alive";
        response.header.insert(std::make_pair(key, val));

        /*deal with body*/
        // response.body = recv_msg;
        std::string responsebody;
        if (request.url == "/index.html")
        {
            int hfd = open("../test/index.html", O_RDWR);
            char buf[READBUFSIZ];
            bzero(buf, sizeof(buf));
            int ret = read(hfd, buf, READBUFSIZ);
            close(hfd);

            if (ret == -1)
            {
                perror("read index.html error. ");
            }
            else
            {
                responsebody += std::move(std::string(buf));
            }
        }
        else
        {
            responsebody += "<html><title>Server Work</title>";
            responsebody += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>";
            responsebody += "<style>body{background-color:#f;font-size:14px;}h1{font-size:60px;color:#eeetext-align:center;padding-top:30px;font-weight:normal;}</style>";
            responsebody += "<body bgcolor=\"ffffff\"><h3>";
            responsebody += response.statecode + " " + response.statemsg;
            responsebody += "</h3>\nURL: " + request.url;
            responsebody += "</h1><hr>Request: </body></html>";
            responsebody += recv_msg;
            responsebody += "<hr><em> ZhangJie's QuickServer</em>\n</body></html>";
        }
        response.body = responsebody;
    }
    else
    {
        /* Not implement methods. */
        response.version = request.version;
        response.statecode = "501";
        response.body = "Not Implemented";

        /* deal with response head */
        std::string key = "Content-type";
        std::string val = "text/html; charset=utf-8";
        response.header.insert(std::make_pair(key, val));

        /*deal with body*/

        std::string responsebody;
        responsebody += "<html><title>Error</title>";
        responsebody += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"></head>";
        responsebody += "<style>body{background-color:#f;font-size:14px;}h1{font-size:60px;color:#eeetext-align:center;padding-top:30px;font-weight:normal;}</style>";
        responsebody += "<body bgcolor=\"ffffff\"><h3>";
        responsebody += response.statecode + " " + response.statemsg;
        responsebody += "</h3>\nURL: " + request.url;
        responsebody += "</h1><hr>Request: </body></html>";
        responsebody += recv_msg;
        responsebody += "<hr><em> ZhangJie's QuickServer</em>\n</body></html>";

        response.body = responsebody;
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
    keep_alive = false;
    request = std::move(HttpRequest());
    response = std::move(HttpResponse());
    recv_msg.clear();
    send_msg.clear();
}

int HTTPSession::readRequest(int epfd, int fd)
{
    std::string msg_peek;
    int n_to_read = INT32_MAX;
    int ret = readmsg(epfd, fd, msg_peek, n_to_read, MSG_PEEK);

    bool prased = false;
    try
    {
        prased = praseHttpRequest(msg_peek, request);
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
    if (!prased)
    {
        if (ret == -1)
        {
            // perror("readmsg error. ");
            removeFd(epfd, fd);
            reset();
            return -1;
        }
        else if (ret == 0)
        {
            if (send_msg.size() == 0)
            {
                removeFd(epfd, fd);
                reset();
                return 0;
            }
            // read file end, listen to output
            modFd(epfd, fd, EPOLLOUT, true);
            return 0;
        }
        else
        {
            /* wait for next EPOLLIN */
            modFd(epfd, fd, EPOLLIN, true);
            return 1;
        }
    }
    else // parsing done
    {
        if (request.header.count("Content-Length"))
        {
            n_to_read = msg_peek.find(dcrlf) + 4 + std::stoi(request.header["Content-Length"]);
        }
        else
        {
            n_to_read = INT32_MAX;
        }

        ret = readmsg(epfd, fd, recv_msg, n_to_read, 0);
        prased = praseHttpRequest(recv_msg, request);
        processHttp(request, response);
        modFd(epfd, fd, EPOLLOUT, true);
    }
}

int HTTPSession::writeResponse(int epfd, int fd)
{
    // std::cout << std::this_thread::get_id() << ": writeResponse.. " << std::endl;
    int have_sent = 0;
    int ret;
    if (send_msg.size() != 0)
    {
        ret = writemsg(epfd, fd, send_msg, have_sent);
        if (ret == -1)
        {
            /* close connection. */
            perror("writemsg error. ");
            removeFd(epfd, fd);
            reset();
            return -1;
        }
        else if (have_sent < send_msg.size())
        {
            send_msg = send_msg.substr(have_sent);
            modFd(epfd, fd, EPOLLOUT, true);
            return 1;
        }
        else
        {
            // send all send_msg
            if (keep_alive)
            {
                send_msg.clear();
                modFd(epfd, fd, EPOLLIN, true);
                return 0;
            }
            else
            {
                removeFd(epfd, fd);
                reset();
                return 0;
            }
        }
    }
    else
    {
        if (!keep_alive)
        {
            shutdown(fd, SHUT_WR); // close writing port
        }
        modFd(epfd, fd, EPOLLIN, true);
        return 0;
    }
}