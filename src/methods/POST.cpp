#include "../../includes/HttpResponse.hpp"

std::string     HttpResponse::handlePOST(HttpRequest& request)
{
    (void) request;
    return this->build();
}