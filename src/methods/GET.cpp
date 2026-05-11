#include "../../includes/HttpResponse.hpp"

// std::string     HttpResponse::handleGET(HttpRequest& request)
// {
//     std::string path = "./www" + request.getPath();
//     if (path == "./www/")
//         path = "./www/index.html";
//     Session& session = request.getSession();
//     if (request.getPath() == "/profile.html" && session.getUserName().empty())
//         return this->redirectWithCookie("/login.html", "");
//     else if ((request.getPath() == "/login.html" || request.getPath() == "/signup.html")
//         && !session.getUserName().empty())
//         return this->redirectWithCookie("/profile.html", "");
//     std::ifstream   file(path.c_str(), std::ios::in | std::ios::binary);
//     if (!file.is_open())
//         return this->errorResponse(NOT_FOUND);
//     std::string         line;
//     std::stringstream   buffer;
//     buffer << file.rdbuf();
//     this->_body = buffer.str();
//     if (request.getPath() == "/profile.html")
//         Utils::replace(this->_body, "{{USERNAME}}", session.getUserName());
//     else if (request.getPath() == "/")
//     {
//         const std::string theme_cookie = request.getTheme();
//         const std::string token = (theme_cookie == "theme-dark") ? "theme-light" : "theme-dark";
//         Utils::replace(this->_body, token, theme_cookie);
//     }
//     this->setHeader("Content-type", this->getMimeType(path));
//     this->setHeader("Content-Length", Utils::to_string_c98(this->_body.size()));
//     this->setBody(this->_body);
//     this->setStatusCode(OK);
//     return this->build();
// }
std::string     HttpResponse::handleGET(HttpRequest& request)
{
	LocationConfig location = ConfigParser::findLocation(request.getPath(), _server);
	std::string path = location.root + request.getPath();
	if(path == location.root + "/")
		path += location.index;
	if (Utils::is_Directory(path))
	{
		try
		{
			if (location.autoindex)
				return (handleAutoIndex(path, request)); // ← return here
			else
				return (this->errorResponse(FORBIDDEN)); // ← no autoindex = 403
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
			return (this->errorResponse(INTERNAL_SERVER_ERROR));
		}
	}
	Session& session = request.getSession();
	if (request.getPath() == "/profile.html" && session.getUserName().empty())
		return this->redirectWithCookie("/login.html", "");
	else if ((request.getPath() == "/login.html" || request.getPath() == "/signup.html")
		&& !session.getUserName().empty())
		return this->redirectWithCookie("/profile.html", "");   
	std::ifstream   file(path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
		return this->errorResponse(NOT_FOUND);
	std::string         line;
	std::stringstream   buffer;
	buffer << file.rdbuf();
	this->_body = buffer.str();
	if (request.getPath() == "/profile.html")
		Utils::replace(this->_body, "{{USERNAME}}", session.getUserName());
	else if (request.getPath() == "/")
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