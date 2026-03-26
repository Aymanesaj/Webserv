#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "libs.hpp"
#include "HttpParser.hpp"

class HttpResponse
{
    private:
		StatusCode							_statusCode;
		std::string							_statusMessage;
		std::map<std::string, std::string>	_headers;
		std::map<std::string, std::string>	_mimes;
		std::vector<std::string>			_cookies;
		std::string						 	_body;

		void			setMimeTypes( void );
		void			setStatusCode(StatusCode code);
		void			setHeader(const std::string& key, const std::string& value);
		void			setBody(const std::string& body);
		// void			setCookies(const std::vector<std::string>& cookies);// to be implemented later
		std::string		getMimeType( const std::string& path ) const;
		std::string		handleGET( const HttpRequest& request);
		std::string		handlePOST( const HttpRequest& request);
		std::string		handleDELETE( const HttpRequest& request);
		std::string		build( void ) const;
		std::string     getErrorPage(StatusCode errorCode) const;
    public:
        HttpResponse();
		std::string		handleRequest(const HttpRequest& request);
		std::string		errorResponse(StatusCode errorCode);
};


#endif
