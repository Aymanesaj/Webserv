#include "../../includes/HttpResponse.hpp"

std::string     HttpResponse::handleGET(const HttpRequest& request)
{
    std::string path = "./www" + request.getPath();
    if (path == "./www/")
        path = "./www/index.html";

    std::ifstream   file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return this->errorResponse(NOT_FOUND);
    std::string         line;
    std::stringstream   buffer;
    buffer << file.rdbuf();
    this->_body = buffer.str();
    this->setHeader("Content-type", this->getMimeType(path));
    this->setHeader("Content-Length", Utils::to_string_c98(this->_body.size()));
    this->setBody(this->_body);
    this->setStatusCode(OK);
    return this->build();
}
