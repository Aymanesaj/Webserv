#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "libs.hpp"

class HttpRequest
{
    private:
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
    public:
        HttpRequest();
        ~HttpRequest();
};

#endif
