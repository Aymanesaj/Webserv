#include "../../includes/HttpResponse.hpp"

// std::string     HttpResponse::handleGET(const HttpRequest& request)
// {
//     (void) request;
//     return this->build();
// }


std::string to_string_c98(const int& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string HttpResponse::handleGET(const HttpRequest& request)
{
    std::string path = "./www" + request.getPath();

    if (path == "./www/")
        path = "./www/index.html";

    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        std::ifstream err("./www/error/404.html");

        std::stringstream buffer;

        if (err.is_open())
        {
            buffer << err.rdbuf();
            this->_body = buffer.str();
        }
        else
        {
            this->_body = "404 Not Found\n";
        }

        this->_statusCode = NOT_FOUND;
        this->_statusMessage = "Not Found";

        this->_headers["Content-Type"] = "text/html";
        this->_headers["Content-Length"] = to_string_c98(_body.size());

        return this->build();
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    this->_body = buffer.str();

    this->_statusCode = OK;
    this->_statusMessage = "OK";

    this->_headers["Content-Type"] = "text/html";
    this->_headers["Content-Length"] = to_string_c98(_body.size());

    return this->build();
}