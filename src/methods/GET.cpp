#include "../../includes/HttpResponse.hpp"


std::string     HttpResponse::handleGET(HttpRequest& request)
{
    (void)request;
    return this->build();
}
