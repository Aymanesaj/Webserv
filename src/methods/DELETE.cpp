#include "../../includes/HttpResponse.hpp"

std::string     HttpResponse::handleDELETE(HttpRequest& request)
{
    LocationConfig location = ConfigParser::findLocation(request.getPath()
        , _server);
    std::string path = location.root + request.getPath();
    if (request.getPath().empty() || request.getPath() == "/")
        return (this->errorResponse(BAD_REQUEST));
    if(path.find("..") != std::string::npos)
        return errorResponse(FORBIDDEN);
    if (!location.isMethodAllowed("DELETE"))
        return errorResponse(METHOD_NOT_ALLOWED);
    struct stat file_stat;  
    if (stat(path.c_str(), &file_stat) != 0)
        return errorResponse(NOT_FOUND);
    if (S_ISDIR(file_stat.st_mode))
        return errorResponse(FORBIDDEN);
    if (access(path.c_str(), W_OK) != 0)
        return errorResponse(FORBIDDEN);
    if (remove(path.c_str()) != 0)
        return errorResponse(INTERNAL_SERVER_ERROR);
    this->setStatusCode(NO_CONTENT);
    this->setBody("");
    this->setHeader("Content-Length", "0");
    return (this->build());
}
