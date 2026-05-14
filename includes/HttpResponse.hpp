#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "libs.hpp"
#include "HttpParser.hpp"
#include "Config.hpp"

struct	ServerConfig;

class HttpResponse
{
    private:
		ServerConfig						_server;
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
		void			setCookie(const std::string& cookie);
		std::string		getMimeType( const std::string& path ) const;
		std::string		handleGET(HttpRequest& request);
		std::string		handlePOST(HttpRequest& request);
		std::string		handleDELETE(HttpRequest& request);
		std::string 	validateUpload(HttpRequest& request,
						LocationConfig& location, std::string& content_type);
		std::string 	handleUrlEncoded(HttpRequest& request, LocationConfig& location);
		std::string 	handleMultipart(HttpRequest& request,
                        LocationConfig& location, const std::string& content_type);
		std::string 	handleRawBody(HttpRequest& request,
                        LocationConfig& location, const std::string& content_type);
		std::string		handleAutoIndex(const std::string& path, HttpRequest& request);
	std::string		build( void ) const;
	std::string		redirectWithCookie(const std::string &location, const std::string &cookie);
	std::string     getErrorPage(StatusCode errorCode) const;
		std::string   	login(HttpRequest& request);
		std::string     signup(HttpRequest& request);
	public:
        HttpResponse();
		void			setServer( ServerConfig _server );
		std::string		handleRequest(HttpRequest& request);
		std::string		errorResponse(StatusCode errorCode);
		StatusCode		getStatusCode( void ) const;
		
};


#endif
