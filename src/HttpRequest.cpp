#include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest() : session(NULL)
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

void    HttpRequest::setCookies( const std::string& cookies)
{
    this->cookies = cookies;
}

void    HttpRequest::setBody( const std::string& body)
{
    this->body = body;
}

void HttpRequest::setSession(Session& session)
{
	this->session = &session;
}

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

const std::string& HttpRequest::getCookies( void ) const
{
    return this->cookies;
}

const std::string&  HttpRequest::getBody( void ) const
{
    return this->body;   
}

Session&  HttpRequest::getSession( void )
{
    return *this->session;
}

const std::string HttpRequest::getTheme( void ) const
{
    std::string theme_cookie = "theme-dark"; // default theme
    size_t pos = this->cookies.find("theme=");
    if (pos != std::string::npos)
    {
        size_t start = pos + 6; // length of "theme="
        size_t end = this->cookies.find(';', start);
        if (end == std::string::npos)
            end = this->cookies.length();
        theme_cookie = this->cookies.substr(start, end - start);
    }
    return theme_cookie;
}

const std::string& HttpRequest::getQuery( void ) const
{
    size_t pos = this->path.find('?');
    if (pos != std::string::npos)
        return this->path.substr(pos + 1);
    return "";
}

const std::string& HttpRequest::getContentType( void ) const
{
    if (this->headers.find("Content-Type") != this->headers.end())
        return this->headers.at("Content-Type");
    return "";
}
