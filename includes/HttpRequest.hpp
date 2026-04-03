#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "libs.hpp"
#include "SessionManager.hpp"

class HttpRequest
{
    private:
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string cookies;
        std::string body;
        Session    *session;
    public:
        HttpRequest();
        ~HttpRequest();
        void    setMethod( const std::string& method);
        void    setPath( const std::string& Path);
        void    setVersion( const std::string& version);
        void    setHeaders( const std::map<std::string, std::string>& headers);
        void    setCookies( const std::string& cookies);
        void    setBody(const std::string& body);
        void    setSession(Session& session);
        const std::string&      getMethod( void ) const;
        const std::string&      getPath( void ) const;
        const std::string&      getVersion( void ) const;
        const std::string&      getBody( void ) const;
        const std::string&      getCookies( void ) const;
        Session&                getSession( void );
        const std::map<std::string, std::string>&   getHeaders( void ) const;
};

#endif
