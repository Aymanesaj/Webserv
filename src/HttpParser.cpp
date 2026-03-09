#include "../includes/HttpParser.hpp"

HttpParser::HttpParser()
    : _state(REQUEST_LINE), _expectedBodySize(0), _bodyType(NO_BODY), 
    _chunkState(CHUNK_SIZE), _statusCode(0)
{
}

HttpParser::~HttpParser() {}

ParseResult		HttpParser::parseRequest(const std::string& data)
{
    ParseResult result = NONE;
    this->_statusCode = 0;
    this->_buffer.append(data);
    while (true)
    {
        switch (this->_state)
        {
            case REQUEST_LINE:
                result = parseRequestLine();
                break;
            case HEADERS:
                result = parseHeaders();
                break;
            case BODY:
                result = parseBody();
                break;
            default:
                break;
        }
        if (result != NONE) break;
    }
    if (result == COMPLETE)
        resetStates();
    return result;
}

ParseResult HttpParser::parseRequestLine( void )
{
    std::vector<std::string>    requestLine;
    size_t                      pos = this->_buffer.find("\r\n");
    size_t                      uriMaxLength = 8000;
    size_t                      p_size = strlen("HTTP/"); // Pattern size
    double                      version = 0.0;

    if (pos == std::string::npos)
        return INCOMPLETE;
    if (pos == 0) // if request has leading empty line
    {
        this->_buffer.erase(0, 2);
        return NONE;
    }
    requestLine = Utils::split(this->_buffer.substr(0, pos), " ");
    this->_buffer.erase(0, pos + 2); // 2 for \r\n
    if (requestLine.size() != 3)
    {
        if (requestLine.size() == 2)
            requestLine.push_back("HTTP/0.9"); // default version
        else
            setStatusCode(BAD_REQUEST);
    } else if (requestLine.size() == 3 && requestLine[2] == "HTTP/0.9")
            setStatusCode(BAD_REQUEST);
    if (!_statusCode && (requestLine[0] != "GET" && requestLine[0] != "POST"
                            && requestLine[0] != "DELETE"))
        Utils::isAllUpper(requestLine[0]) ? setStatusCode(METHOD_NOT_ALLOWED) : setStatusCode(BAD_REQUEST);
    if (!_statusCode && requestLine[1].length() > uriMaxLength)
        setStatusCode(URI_TOO_LONG);
    if (!_statusCode && requestLine[2].substr(0, p_size) != "HTTP/")
            setStatusCode(BAD_REQUEST);
    else if (!_statusCode)
    {
        char *endptr = NULL;
        version = strtod(requestLine[2].substr(p_size, requestLine[2].length() - p_size).c_str(), &endptr);
        if (version < 0.9 || *endptr)
            setStatusCode(BAD_REQUEST);
        else if (version > 1.1)
            setStatusCode(HTTP_VERSION_NOT_SUPPORTED);
    }
    this->_request.setMethod(requestLine[0]);
    this->_request.setPath(requestLine[1]);
    this->_request.setVersion(requestLine[2]);
    this->_state = HEADERS; // adjust parse state
    return NONE;
}

/*
    All duplicate headers should be combined separated by comma 
    Edge cases:
        {Host, Conten-length} headers can't be duplicated
        {Set-Cookie} headers should not be concatenated
*/
ParseResult HttpParser::parseHeaders( void )
{
    size_t  pos = this->_buffer.find("\r\n\r\n");
    if (pos == std::string::npos)
        return INCOMPLETE;
    std::map<std::string, std::string>  map; // headers map
    std::vector<std::string> cookies;
    std::vector<std::string> lines;
    std::pair<std::string, std::string> header;
    lines = Utils::split(this->_buffer.substr(0, pos), "\r\n");
    this->_buffer.erase(0, pos + 4); // 4 for \r\n\r\n

    // set default connection
    this->_request.getVersion() == "HTTP/1.1" ? map["Connection"] = "keep-alive"
        : map["Connection"] = "close";

    for (size_t i = 0; i < lines.size(); i++)
    {
        pos = lines[i].find(":");
        if (pos == std::string::npos)
            setStatusCode(BAD_REQUEST);
        else
            header = std::make_pair(lines[i].substr(0, pos), lines[i].substr(pos + 1));
        if (Utils::isAllSpaces(header.first) || Utils::isAllSpaces(header.second)
            || std::isspace(static_cast<unsigned char>(header.first[header.first.length() - 1])))
            setStatusCode(BAD_REQUEST);
        Utils::capitalizeWord(header.first);
        Utils::trim(header.first);
        Utils::trim(header.second);
        if ((header.first == "Host" || header.first == "Content-length")
                && map.find(header.first) != map.end())
            setStatusCode(BAD_REQUEST);
        else if (header.first == "Set-cookie")
            cookies.push_back(header.second);
        else if (map.find(header.first) == map.end()
            || (header.first == "Connection" && header.second == "keep-alive"))
            map[header.first] = header.second;
        else if (header.first != "Connection")
            map[header.first] = map[header.first] + ", " + header.second;
    }
    if (this->_request.getVersion() == "HTTP/1.1"
        && map.find("Host") == map.end())
        setStatusCode(BAD_REQUEST);
    this->_request.setHeaders(map);
    this->_request.setCookies(cookies);
    this->_state = BODY;
    this->setBodyType(map);
    if (this->_buffer.compare(0, 2, "\r\n") == 0)
        this->_buffer.erase(0, 2); // for the empty line after headers
    return NONE;
}

ParseResult HttpParser::parseBody( void )
{
    ParseResult result = COMPLETE;
    switch (this->_bodyType)
    {
        case NO_BODY:
            break;
        case CONTENT_LENGTH:
            result = parseLengthBody();
            break;
        case CHUNKED:
            result = parseChunkBody();
            break;
    }
    return result;
}

void    HttpParser::setBodyType( const std::map<std::string, std::string>& headers )
{
    if (this->_request.getMethod() != "POST")
        this->_bodyType = NO_BODY;
    else if (headers.find("Transfer-encoding") != headers.end())
    {
        if (headers.find("Transfer-encoding")->second == "chunked")
        {
            this->_bodyType = CHUNKED;
            this->_chunkState = CHUNK_SIZE;
        }
        else
            setStatusCode(NOT_IMPLEMENTED);
    }
    else if (headers.find("Content-length") != headers.end())
    {
        char *endptr = NULL;
        long contentLength = strtol(headers.find("Content-length")->second.c_str(), &endptr, 10);
        if (contentLength < 0 || *endptr) // i need to add a max content length check here
            setStatusCode(BAD_REQUEST);
        else
        {
            this->_bodyType = CONTENT_LENGTH;
            this->_expectedBodySize = static_cast<size_t>(contentLength);
        }
    }
    else
        setStatusCode(CONTENT_LENGTH_REQUIRED);
}

ParseResult HttpParser::parseLengthBody( void )
{
    if (this->_buffer.length() < _expectedBodySize)
        return INCOMPLETE;
    this->_request.setBody(this->_buffer.substr(0, _expectedBodySize));
    if (this->_buffer.compare(_expectedBodySize, 2, "\r\n") == 0)//if request has trailing empty line
        this->_buffer.erase(0, _expectedBodySize + 2);
    else
        this->_buffer.erase(0, _expectedBodySize);
    return COMPLETE;
}

ParseResult HttpParser::parseChunkBody( void )
{
    size_t      pos = this->_buffer.find("\r\n0\r\n");
    if (pos == std::string::npos)
        return INCOMPLETE;
    long        chunkSize;
    char*       endptr;
    std::string tmp;
    std::string body;
    ParseResult result = NONE;
    while (true)
    {
        switch (this->_chunkState)
        {
            case CHUNK_SIZE:
                pos = this->_buffer.find("\r\n");
                if (pos == std::string::npos)
                    return INCOMPLETE;
                tmp = this->_buffer.substr(0, pos);
                this->_buffer.erase(0, pos + 2);
                chunkSize = strtol(tmp.c_str(), &endptr, 16);
                if (*endptr || chunkSize < 0)
                    setStatusCode(BAD_REQUEST);
                if (chunkSize == 0)
                    result = COMPLETE;
                else
                    this->_chunkState = CHUNK_DATA;
                break;
            case CHUNK_DATA:
                pos = this->_buffer.find("\r\n");
                if (pos == std::string::npos)
                    return INCOMPLETE;
                tmp = this->_buffer.substr(0, pos);
                if (static_cast<size_t>(chunkSize) != tmp.length())
                    setStatusCode(BAD_REQUEST);
                body += tmp;
                this->_buffer.erase(0, pos + 2);
                this->_chunkState = CHUNK_SIZE;
            break;
        }
        if (result != NONE) break;
    }
    if (result == COMPLETE && this->_buffer.compare(0, 2, "\r\n") == 0) // if request has trailing empty line
        this->_buffer.erase(0, 2);
    this->_request.setBody(body);
    return COMPLETE;
}

const HttpRequest&    HttpParser::getRequest( void ) const
{
    return this->_request;
}

int HttpParser::getStatusCode( void ) const
{
    return this->_statusCode;
}

void    HttpParser::setStatusCode(StatusCode statusCode)
{
    this->_statusCode = statusCode;
    this->_state = REQUEST_LINE;
    throw std::runtime_error("Invalid HTTP request");
}

void    HttpParser::resetStates( void )
{
    this->_state = REQUEST_LINE;
    this->_expectedBodySize = 0;
    this->_bodyType = NO_BODY;
    this->_chunkState = CHUNK_SIZE;
    this->_statusCode = 0;
}
