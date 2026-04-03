#include "../../includes/HttpResponse.hpp"

std::string     HttpResponse::handleDELETE(HttpRequest& request)
{
    (void) request;
    return this->build();
}