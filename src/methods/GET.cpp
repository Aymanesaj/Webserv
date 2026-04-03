#include "../../includes/HttpResponse.hpp"


std::string     HttpResponse::handleGET(HttpRequest& request)
{
    std::string path = "./www" + request.getPath();
    if (path == "./www/")
        path = "./www/index.html";
    
    if (path == "./www/counter") // Example of a dynamic response using sessions, to be optimized and moved to a better place in the future
    {
        Session& session = request.getSession();
        std::string sessionId = session.getId();
        int counter = session.getCounter();
        session.incrementCounter();
        
        std::string counter_html = "<html><body><h1>Counter: " + Utils::to_string_c98(counter) + "</h1></body></html>";
        this->setHeader("Content-type", "text/html");
        this->setHeader("Content-Length", Utils::to_string_c98(counter_html.size()));
        this->setHeader("Set-Cookie", "session_id=" + sessionId + "; Path=/");
        this->setBody(counter_html);
        this->setStatusCode(OK);
        return this->build();
    }    
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
