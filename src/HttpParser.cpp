#include "../includes/HttpParser.hpp"

HttpParser::HttpParser()
    : state(REQUEST_LINE), expectedBodySize(0), errorCode(0)
{
    (void) state; // to silent -Werror for now
    (void) expectedBodySize;
    (void) errorCode;
}

HttpParser::~HttpParser() {}

ParseResult		HttpParser::parseRequest(const std::string& data)
{
    ParseResult result = NONE;
    this->errorCode = 0;
    this->buffer.append(data);
    while (true)
    {
        switch (this->state)
        {
            case REQUEST_LINE:
                result = parseRequestLine();
                break;
            case HEADERS:
                result = parseHeaders();
                break;
            case BODY:
                // result = parseBody();
            return COMPLETE; // temporary for testing only, should be removed when body parsing is implemented
                break;
            default:
                break;
        }
        if (result != NONE) break;
    }
    return result;
}

ParseResult HttpParser::parseRequestLine()
{
    std::vector<std::string>    requestLine;
    size_t                      pos = buffer.find("\r\n");
    size_t                      uriMaxLength = 8000;
    size_t                      p_size = strlen("HTTP/"); // Pattern size
    double                      version = 0.0;

    if (pos == std::string::npos)
        return INCOMPLETE;
    requestLine = Utils::split(buffer.substr(0, pos), " ");
    buffer.erase(0, pos + 2); // 2 for \r\n
    if (requestLine.size() != 3)
    {
        if (requestLine.size() == 2)
            requestLine.push_back("HTTP/0.9"); // default version
        else
            errorCode = BAD_REQUEST;
    } else if (requestLine.size() == 3 && requestLine[2] == "HTTP/0.9")
        errorCode = BAD_REQUEST;
    if (!errorCode && (requestLine[0] != "GET" && requestLine[0] != "POST"
                            && requestLine[0] != "DELETE"))
        Utils::isAllUpper(requestLine[0]) ? errorCode = METHOD_NOT_ALLOWED : errorCode = BAD_REQUEST;
    if (!errorCode && requestLine[1].length() > uriMaxLength)
        errorCode = URI_TOO_LONG;
    if (!errorCode && requestLine[2].substr(0, p_size) != "HTTP/")
        errorCode = BAD_REQUEST;
    else if (!errorCode)
    {
        char *endptr = NULL;
        version = strtod(requestLine[2].substr(p_size, requestLine[2].length() - p_size).c_str(), &endptr);
        if (version < 0.9 || *endptr)
            errorCode = BAD_REQUEST;
        else if (version > 1.1)
            errorCode = HTTP_VERSION_NOT_SUPPORTED;
    }

    if (errorCode == 0)
    {
        this->request.setMethod(requestLine[0]);
        this->request.setPath(requestLine[1]);
        this->request.setVersion(requestLine[2]);
        this->state = HEADERS; // adjust parse state
        return NONE;
    }
    return ERROR;
}

/*
    All duplicate headers should be combined separated by comma 
    Edge cases:
        {Host, Conten-length} headers can't be duplicated
        {Set-Cookie} headers should not be concatenated
*/
ParseResult HttpParser::parseHeaders()
{
    size_t  pos = buffer.find("\r\n\r\n");
    if (pos == std::string::npos)
        return INCOMPLETE;
    std::map<std::string, std::string>  map; // headers map
    std::vector<std::string> cookies;
    std::vector<std::string> lines;
    std::vector<std::string> header; // temp
    lines = Utils::split(buffer.substr(0, pos), "\r\n");

    // set default connection
    this->request.getVersion() == "HTTP/1.1" ? map["Connection"] = "keep-alive"
        : map["Connection"] = "close";

    for (size_t i = 0; i < lines.size(); i++)
    {
        header = Utils::split(lines[i], ":");
        if (std::count(lines[i].begin(), lines[i].end(), ':') != 1 && header.size() != 2)
        {
            this->errorCode = BAD_REQUEST;
            return ERROR;
        }
        Utils::capitalizeWord(header[0]);
        Utils::trim(header[0]);
        Utils::trim(header[1]);
        if ((header[0] == "Host" || header[0] == "Content-length")
                && map.find(header[0]) != map.end())
        {
            this->errorCode = BAD_REQUEST;
            return ERROR;
        } else if (header[0] == "Set-cookie")
            cookies.push_back(header[1]);
        else if (map.find(header[0]) == map.end()
            || (header[0] == "Connection" && header[1] == "keep-alive"))
            map[header[0]] = header[1];
        else if (header[0] != "Connection")
            map[header[0]] = map[header[0]] + ", " + header[1];
    }
    if (this->request.getVersion() == "HTTP/1.1"
        && map.find("Host") == map.end())
    {
        this->errorCode = BAD_REQUEST;
        return ERROR;
    }
    this->request.setHeaders(map);
    this->request.setCookies(cookies);
    this->state = BODY;
    return NONE;
}

const HttpRequest&    HttpParser::getRequest( void ) const
{
    return this->request;
}

int HttpParser::getErrorCode( void ) const
{
    return this->errorCode;
}
