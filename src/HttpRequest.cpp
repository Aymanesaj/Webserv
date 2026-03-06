#include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest()
{
}

HttpRequest::~HttpRequest()
{
}

void    HttpRequest::setMethod(const std::string& method)
{
    this->method = method;
}

void    HttpRequest::setPath(const std::string& path)
{
    this->path = path;
}

void    HttpRequest::setVersion(const std::string& version)
{
    this->version = version;
}

void    HttpRequest::setHeaders( const std::map<std::string, std::string>& headers)
{
    this->headers = headers;
}

void    HttpRequest::setCookies( const std::vector<std::string>& cookies)
{
    this->cookies = cookies;
}

void    HttpRequest::setBody( const std::string& body)
{
    this->body = body;
}

//
const std::string&  HttpRequest::getMethod( void ) const
{
    return this->method;
}

const std::string&  HttpRequest::getPath( void ) const
{
    return this->path;
}

const std::string&  HttpRequest::getVersion( void ) const
{
    return this->version;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders( void ) const
{
    return this->headers;
}

const std::vector<std::string>& HttpRequest::getCookies( void ) const
{
    return this->cookies;
}

const std::string&  HttpRequest::getBody( void ) const
{
    return this->body;   
}
