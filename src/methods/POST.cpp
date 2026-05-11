#include "../../includes/HttpResponse.hpp"

/* session and cookie handling */
/* * */

std::string HttpResponse::redirectWithCookie(const std::string &location, const std::string &cookie)
{
	if (!cookie.empty())
		this->setCookie(cookie);
	this->setStatusCode(SEE_OTHER);
	this->setHeader("Location", location);
	return this->build();
}

// static std::string getData(const std::string& body, const std::string& key)
// {
//     std::vector<std::string> pairs = Utils::split(body, "&");
//     std::vector<std::string> kv;
//     for (size_t i = 0; i < pairs.size(); i++) {
//         kv = Utils::split(pairs[i], "=");
//         if (kv.size() == 2 && kv[0] == key)
//             return kv[1];
//     }
//     return "";
// }

// static std::string loadLoginPage(bool showError)
// {
//     std::ifstream file("./www/login.html", std::ios::in | std::ios::binary);
//     if (!file.is_open())
//         return "";

//     std::stringstream buffer;
//     buffer << file.rdbuf();
//     std::string html = buffer.str();

//     if (showError)
//     {
//         size_t pos = html.find("error hidden");
//         if (pos != std::string::npos)
//             html.erase(pos + 5, 7);
//     }
//     return html;
// }

// std::string   HttpResponse::redirectWithCookie(const std::string& location, const std::string& cookie)
// {
//     if (!cookie.empty())
//         this->setCookie(cookie);
//     this->setStatusCode(SEE_OTHER);
//     this->setHeader("Location", location);
//     return this->build();
// }

// std::string   HttpResponse::login(HttpRequest& request)
// {
//     std::string body = request.getBody();
//     Session& session = request.getSession();
    
//     std::string username = getData(body, "username");
//     std::string password = getData(body, "password");

//     if (username != session.getUserName() || password != session.getPassword())
//     {
//         this->setStatusCode(UNAUTHORIZED);
//         std::string loginPage = loadLoginPage(true); // Load the login page with an error message
//         if (loginPage.empty())
//             return "<html><body><h1>Invalid username or password.</h1></body></html>";
//         return loginPage;
//     }

//     this->setStatusCode(SEE_OTHER);
//     this->setHeader("Location", "/profile.html");
//     return "";
// }

// std::string   HttpResponse::signup(HttpRequest& request)
// {
//     std::string body = request.getBody();
//     Session& session = request.getSession();
    
//     std::string username = getData(body, "username");
//     std::string password = getData(body, "password");

//     session.setUserName(username);
//     session.setPassword(password);

//     this->setStatusCode(SEE_OTHER);
//     this->setHeader("Location", "/login.html");
//     return "";
// }
// /* * */

// std::string     HttpResponse::handlePOST(HttpRequest& request)
// {
//     std::string response_html;
//     if (request.getPath() == "/login")
//         response_html = this->login(request);
//     else if (request.getPath() == "/signup")
//         response_html = this->signup(request);
//     else if (request.getPath() == "/logout")
//         return this->redirectWithCookie("/login.html", "session_id=; Path=/; Max-Age=0; HttpOnly");
//     else if (request.getPath() == "/toggle-theme")
//     {
//         const std::string theme_cookie = request.getTheme();
//         std::string new_theme = (theme_cookie == "theme-light") ? "theme-dark" : "theme-light";
//         return this->redirectWithCookie("/", "theme=" + new_theme + "; Path=/; Max-Age=3600; HttpOnly");
//     }
//     else
//         return this->errorResponse(NOT_FOUND);
//     this->setHeader("Content-Type", "text/html");
//     this->setHeader("Content-Length", Utils::to_string_c98(response_html.size()));
//     this->setBody(response_html);
//     return this->build();
// }

std::string HttpResponse::handlePOST(HttpRequest& request)
{
    LocationConfig location = ConfigParser::findLocation(request.getPath(), _server);
    size_t max_size = 10 * 1024 * 1024;
    std::vector<std::string> parts, kv;
    std::string key, value, all_path, boundary, filename, full_data;
    std::map<std::string, std::string> my_map;
    std::map<std::string, std::string>::iterator help;
    size_t num;
    std::string content_type = "";

    if (request.getHeaders().find("Content-Type") != request.getHeaders().end())
        content_type = request.getHeaders().at("Content-Type");
    std::cout << "alialilaila" << std::endl;
    if (location.upload_enable == false)
        return (this->errorResponse(FORBIDDEN));
    if (request.getHeaders().find("Content-Length") == request.getHeaders().end())
        return (this->errorResponse(CONTENT_LENGTH_REQUIRED));
    if (request.getBody().empty())
        return (this->errorResponse(BAD_REQUEST));
    if (request.getBody().size() > max_size)
        return (this->errorResponse(CONTENT_TOO_LARGE));
    if (location.upload_path.empty())
        return (this->errorResponse(INTERNAL_SERVER_ERROR));
    if (!Utils::DirctoryIsExists(location.upload_path) || !Utils::is_Directory(location.upload_path))
        return (this->errorResponse(INTERNAL_SERVER_ERROR));

    if (content_type.find("application/x-www-form-urlencoded") != std::string::npos)
    {
        parts = Utils::split(request.getBody(), "&");
        for (size_t i = 0; i < parts.size(); i++)
        {
            kv = Utils::split(parts[i], "=");
            if (kv.size() == 2)
            {
                key   = kv[0];
                value = kv[1];
                my_map[key] = value;
            }
        }
        all_path = location.upload_path + "/save.txt";
        std::ofstream file(all_path.c_str(), std::ios::binary);
        if (!file.is_open())
            return (this->errorResponse(INTERNAL_SERVER_ERROR));
        for (help = my_map.begin(); help != my_map.end(); help++)
            file << help->first << " = " << help->second << std::endl;
        file.close();
        this->setStatusCode(CREATED);
        this->setHeader("Content-Type", "text/html");
        this->setBody("<html><body><h1>201 Created!</h1></body></html>");
    }
    else if (content_type.find("multipart/form-data") != std::string::npos)
    {
        if (content_type.find("boundary=") == std::string::npos)
            return (this->errorResponse(BAD_REQUEST));
        boundary = content_type.substr(content_type.find("boundary=") + 9);
        if (boundary.empty())
            return (this->errorResponse(BAD_REQUEST));
        boundary = "--" + boundary;
        parts = Utils::split(request.getBody(), boundary);
        for (size_t i = 0; i < parts.size(); i++)
        {
            if (parts[i].find("Content-Disposition: form-data") != std::string::npos)
            {
                num = parts[i].find("filename");
                if (num == std::string::npos)
                    continue;
                filename = parts[i].substr(num + 9);
                filename = filename.substr(0, filename.find("\r\n"));
                if (!filename.empty() && filename[0] == '"')
                    filename = filename.substr(1, filename.size() - 2);
                if (filename.empty())
                    continue;
                filename = location.upload_path + "/" + filename;
                std::ofstream file(filename.c_str(), std::ios::binary);
                if (!file.is_open())
                    return (this->errorResponse(INTERNAL_SERVER_ERROR));
                num = parts[i].find("\r\n\r\n");
                if (num == std::string::npos)
                {
                    file.close();
                    return (this->errorResponse(BAD_REQUEST));
                }
                full_data = parts[i].substr(num + 4);
                if (full_data.size() >= 2 &&
                    full_data.substr(full_data.size() - 2) == "\r\n")
                    full_data = full_data.substr(0, full_data.size() - 2);
                file.write(full_data.c_str(), full_data.size());
                file.close();
                this->setStatusCode(CREATED);
                this->setHeader("Content-Type", "text/html");
                this->setBody("<html><body><h1>201 Created!</h1></body></html>");
            }
        }
    }
    else
    {
        std::string req_path = request.getPath();
        size_t slash = req_path.rfind("/");

        if (slash != std::string::npos && slash != req_path.size() - 1)
            filename = req_path.substr(slash + 1);

        if (filename.empty())
        {
            if (content_type.find("image/jpeg") != std::string::npos)
                filename = "upload.jpg";
            else if (content_type.find("image/png") != std::string::npos)
                filename = "upload.png";
            else if (content_type.find("application/json") != std::string::npos)
                filename = "upload.json";
            else if (content_type.find("text/plain") != std::string::npos)
                filename = "upload.txt";
            else if (content_type.find("application/pdf") != std::string::npos)
                filename = "upload.pdf";
        }

        if (filename.empty())
            filename = "upload.bin";

        filename = location.upload_path + "/" + filename;
        std::ofstream file(filename.c_str(), std::ios::binary);
        if (!file.is_open())
            return (this->errorResponse(INTERNAL_SERVER_ERROR));
        file.write(request.getBody().c_str(), request.getBody().size());
        file.close();
        this->setStatusCode(CREATED);
        this->setHeader("Content-Type", "text/html");
        this->setBody("<html><body><h1>201 Created!</h1></body></html>");
    }

    return (this->build());
}
