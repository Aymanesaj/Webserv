#include "../includes/HttpRequest.hpp"

HttpRequest::HttpRequest() : session(NULL), body_file(-1)
{
}

HttpRequest::~HttpRequest()
{
    this->clear();
}

void HttpRequest::clear( void )
{
    this->method.clear();
    this->path.clear();
    this->version.clear();
    this->headers.clear();
    this->cookies.clear();
    this->session = NULL;
    if (this->body_file != -1)
    {
        close(this->body_file);
        this->body_file = -1;
    }
    if (!this->body_file_path.empty())
    {
        unlink(this->body_file_path.c_str());
        this->body_file_path.clear();
    }
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
    write(this->body_file, body.c_str(), body.length());
}

void HttpRequest::setBodyFile( int fd )
{
    this->body_file = fd;
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

const std::string& HttpRequest::getBodyFilePath( void ) const
{
    return this->body_file_path;
}

int HttpRequest::getBody( void ) const
{
    return this->body_file;
}

size_t HttpRequest::getBodySize( void ) const
{
    if (this->body_file_path.empty())
        return 0;
    struct stat st;
    if (stat(this->body_file_path.c_str(), &st) == 0)
        return static_cast<size_t>(st.st_size);
    return 0;
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

void HttpRequest::generateTempFile( void )
{
    char temp[] = "/tmp/webserv_temp_XXXXXX";
    int fd = mkstemp(temp);
    this->body_file_path = std::string(temp);
    this->body_file = fd;
}

const std::string& HttpRequest::getQuery( void ) const
{
    static std::string query;
    size_t pos = this->path.find('?');

    if (pos != std::string::npos)
        query = this->path.substr(pos + 1);
    else
        query.clear();
    return query;
}

const std::string& HttpRequest::getContentType( void ) const
{
    static const std::string empty;
    if (this->headers.find("Content-Type") != this->headers.end())
        return this->headers.at("Content-Type");
    return empty;
}

