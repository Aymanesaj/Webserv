#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include "libs.hpp"
#include "HttpRequest.hpp"
#include "Utils.hpp"

enum StatusCode {
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    SEE_OTHER = 303,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    REQUEST_TIMEOUT = 408,
    CONTENT_LENGTH_REQUIRED = 411,
    CONTENT_TOO_LARGE = 413,
    URI_TOO_LONG = 414,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    HTTP_VERSION_NOT_SUPPORTED = 505
};

enum ParseResult {
	NONE,
    INCOMPLETE,
    COMPLETE,
};

class HttpParser
{
    private:
        enum ParseState {
            REQUEST_LINE,
            HEADERS,
            BODY,
        };
        enum BodyType {
            NO_BODY,
            CONTENT_LENGTH,
            CHUNKED
        };
        enum ChunkState {
            CHUNK_SIZE,
            CHUNK_DATA
        };
        ParseState	_state;
        std::string	_buffer;
		HttpRequest	_request;
        size_t		_expectedBodySize;
        BodyType	_bodyType;
        ChunkState  _chunkState;
		int			_errorCode;

		ParseResult		parseRequestLine( void );
		ParseResult		parseHeaders( void );
        ParseResult     handleEmptyHeaders( std::map<std::string, std::string>& headers );
		ParseResult		parseBody( void );

        void            setBodyType( const std::map<std::string, std::string>& headers );
        ParseResult     parseLengthBody( void );
		ParseResult		parseChunkBody( void );
        void            resetStates( void );
    public:
        HttpParser();
        ~HttpParser();
		ParseResult         parseRequest(const std::string& data);
		const HttpRequest&  getRequest( void ) const;
        void                setErrorCode(StatusCode statusCode);
		int				    getErrorCode( void ) const;
};

#endif
