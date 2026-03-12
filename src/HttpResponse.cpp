#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse()
    : _statusCode(OK), _statusMessage("OK")
{
    this->setMimeTypes();
    this->setHeader("Server", "Webserv");
    this->setHeader("Host", "localhost");
    this->setHeader("Date", Utils::getCurrentDate());
}

void HttpResponse::setStatusCode(StatusCode code)
{
    this->_statusCode = code;
    switch (code)
    {
        case OK: this->_statusMessage = "OK"; break;
        case CREATED: this->_statusMessage = "Created"; break;
        case NO_CONTENT: this->_statusMessage = "No Content"; break;
        case SEE_OTHER: this->_statusMessage = "SEE OTHER"; break;
        case BAD_REQUEST: this->_statusMessage = "Bad Request"; break;
        case FORBIDDEN: this->_statusMessage = "Forbidden"; break;
        case NOT_FOUND: this->_statusMessage = "Not Found"; break;
        case METHOD_NOT_ALLOWED: this->_statusMessage = "Method Not Allowed"; break;
        case REQUEST_TIMEOUT: this->_statusMessage = "Request Timeout"; break;
        case CONTENT_LENGTH_REQUIRED: this->_statusMessage = "Content Length Required"; break;
        case CONTENT_TOO_LARGE: this->_statusMessage = "Content Too Large"; break;
        case URI_TOO_LONG: this->_statusMessage = "URI Too Long"; break;
        case INTERNAL_SERVER_ERROR: this->_statusMessage = "Internal Server Error"; break;
        case NOT_IMPLEMENTED: this->_statusMessage = "Not Implemented"; break;
        case HTTP_VERSION_NOT_SUPPORTED: this->_statusMessage = "HTTP Version Not Supported"; break;
        default: this->_statusMessage = "Unknown Status"; break;
    }
}

void    HttpResponse::setHeader(const std::string& key, const std::string& value)
{
    this->_headers[key] = value;
}

void    HttpResponse::setBody(const std::string& body)
{
    this->_body = body;
}

/* build the response structure */
std::string     HttpResponse::build( void ) const
{
    std::stringstream resp;

    resp << "HTTP/1.1 " << this->_statusCode << " " << this->_statusMessage << "\r\n";
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
        it != _headers.end(); it++)
    {
        resp << it->first << ": " << it->second << "\r\n";
    }
    for (size_t i = 0; i < this->_cookies.size(); i++)
    {
        resp << "Set-Cookie: " << this->_cookies[i] << "\r\n"; 
    }
    resp << "\r\n";
    resp << _body;

    return resp.str();
}

void    HttpResponse::setMimeTypes( void )
{
    this->_mimes[".txt"] = "text/plain";
    this->_mimes[".html"] = "text/html";
    this->_mimes[".htm"] = "text/html";
    this->_mimes[".css"] = "text/css";
    this->_mimes[".js"] = "application/javascript";
    this->_mimes[".png"] = "image/png";
    this->_mimes[".jpg"] = "image/jpeg";
    this->_mimes[".mp4"] = "video/mp4";
}

std::string     HttpResponse::getMimeType(const std::string& path) const
{
    std::string extention;
    size_t ext_pos = path.find_last_of('.');
    if (ext_pos != std::string::npos)
        extention = path.substr(ext_pos);
    return this->_mimes.count(extention) ? this->_mimes.find(extention)->second : "application/octet-stream";
}

std::string    HttpResponse::errorResponse(StatusCode errorCode, std::string errorPage)
{
    std::ifstream   file(errorPage.c_str());
    std::string     body;

    this->setStatusCode(errorCode);
	if (file.is_open())
    {
        std::string     line;
        this->setHeader("Content-type", this->getMimeType(errorPage));
        while (std::getline(file, line))
            body.append(line);
	}
    else
    {
        std::stringstream ss;
        ss << this->_statusCode;// convert number to string
        std::string statusCode_str = ss.str();
        this->setHeader("Content-type", "text/html");
        body.append("<html><h1>" + statusCode_str + " Error: " + this->_statusMessage + "</h1></html>");
    }
    this->setBody(body);
    return this->build();
}

std::string     HttpResponse::handleRequest(const HttpRequest& request)
{
    std::string response;
    std::string method = request.getMethod();

    if (method == "GET")
        response = handleGET(request);
    else if (method == "POST")
        response = handlePOST(request);
    else if (method == "DELETE")
        response = handleDELETE(request);
    else
        response = errorResponse(METHOD_NOT_ALLOWED, "www/error/405.html");
    return response;
}
