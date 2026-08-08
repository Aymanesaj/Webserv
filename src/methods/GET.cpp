#include "../../includes/HttpResponse.hpp"

std::string     HttpResponse::handleGET(HttpRequest& request)
{
    LocationConfig location = ConfigParser::findLocation(request.getPath(), this->_server);
    if (!location.isMethodAllowed(request.getMethod()))
        return this->errorResponse(METHOD_NOT_ALLOWED);
    std::string path = location.root + request.getPath();
    if (!Utils::isPathSafe(path, location.root))
        return this->errorResponse(FORBIDDEN);
    if (path == location.root + "/")
        path += location.index;
    if (location.has_return && !location.return_url.empty())
        return this->redirectWithCookie(location.return_url, "");
    if (Utils::is_Directory(path)) {
        if (path[path.length() - 1] != '/')
            path += "/";
        if (location.autoindex)
            return this->handleAutoIndex(request, location);
        if (!location.has_index || location.index.empty()) {
            return this->errorResponse(FORBIDDEN);
        }
        path += location.index;
    }

    /*--- session handling ---*/
    Session& session = request.getSession();
    if (request.getPath() == "/profile.html" && session.getUserName().empty())
        return this->redirectWithCookie("/login.html", "");
    else if ((request.getPath() == "/login.html" || request.getPath() == "/signup.html")
        && !session.getUserName().empty())
        return this->redirectWithCookie("/profile.html", "");
    /*--- ---*/

    std::ifstream   file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open() && Utils::isFileExists(path))
        return this->errorResponse(FORBIDDEN);
    else if (!file.is_open())
        return this->errorResponse(NOT_FOUND);
    std::string         line;
    std::stringstream   buffer;
    buffer << file.rdbuf();
    this->_body = buffer.str();
    if (request.getPath() == "/profile.html")
        Utils::replace(this->_body, "{{USERNAME}}", session.getUserName());
    else if (request.getPath() == "/index.html")
    {
        const std::string theme_cookie = request.getTheme();
        const std::string token = (theme_cookie == "theme-dark") ? "theme-light" : "theme-dark";
        Utils::replace(this->_body, token, theme_cookie);
    }
    this->setHeader("Content-type", this->getMimeType(path));
    this->setHeader("Content-Length", Utils::to_string_c98(this->_body.size()));
    this->setBody(this->_body);
    this->setStatusCode(OK);
    return this->build();
}
