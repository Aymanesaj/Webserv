#include "../../includes/HttpResponse.hpp"

std::string     HttpResponse::handleDELETE(HttpRequest& request)
{
    LocationConfig location = ConfigParser::findLocation(request.getPath()
        , _server);
    std::string path = location.root + location.path;
    if(path == location.root + "/")
        return errorResponse(BAD_REQUEST);
    if(path.find("..") != std::string::npos)
        return errorResponse(FORBIDDEN);
    if (!location.upload_enable)
        return this->errorResponse(FORBIDDEN);
    std::ifstream   file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return this->errorResponse(NOT_FOUND);
    file.close();
    struct stat file_stat;
    if (stat(path.c_str(), &file_stat) == 0 && S_ISDIR(file_stat.st_mode))
        return errorResponse(FORBIDDEN);
    if (remove(path.c_str()) != 0)
        return this->errorResponse(INTERNAL_SERVER_ERROR);
    this->setStatusCode(NO_CONTENT);
    this->setBody("");
    return this->build();
}