#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "libs.hpp"
#include "HttpRequest.hpp"

enum ParseResult {
    INCOMPLETE,
    COMPLETE,
    ERROR
};

class HttpParser
{
    private:
        enum ParseState {
            REQUEST_LINE,
            HEADERS,
            BODY,
        };
        ParseState	state;
        std::string	buffer;
		HttpRequest	request;
        size_t		expectedBodySize;
		int			errorCode;

		void		parseRequestLine();
		void		parseHeaders();
		void		parseBody();
    public:
        HttpParser();
        ~HttpParser();
		ParseResult		parseRequest(const std::string& data);
		HttpRequest&	getRequest( void ) const;
		int				getErrorCode( void ) const;
};


#endif
