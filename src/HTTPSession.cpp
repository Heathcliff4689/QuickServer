#include "HTTPSession.h"


HTTPSession::HTTPSession()
    : prase_done(false),
      crlf("\r\n"),
      dcrlf("\r\n\r\n"),
      request(),
      response(),
      msg()
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
            int pos_colon = std::string::npos;

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
                val = line.substr(pos_colon + 2);
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
    
}

void HTTPSession::reset(){
    prase_done = false;
    request = HttpRequest();
    response = HttpResponse();
    msg = std::string();
}

