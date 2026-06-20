#include "../../includes/HttpResponse.hpp"

/* session and cookie handling */
/* * */
static std::string getData(int body_fd, const std::string& key)
{
    char buffer[1024];
    ssize_t bytes = read(body_fd, buffer, sizeof(buffer));
    if (bytes <= 0)
        return "";
    std::string body(buffer, bytes);
    std::vector<std::string> pairs = Utils::split(body, "&");
    std::vector<std::string> kv;
    for (size_t i = 0; i < pairs.size(); i++) {
        kv = Utils::split(pairs[i], "=");
        if (kv.size() == 2 && kv[0] == key)
            return kv[1];
    }
    return "";
}

static std::string loadLoginPage(bool showError)
{
    std::ifstream file("./www/login.html", std::ios::in | std::ios::binary);
    if (!file.is_open())
        return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html = buffer.str();

    if (showError)
    {
        size_t pos = html.find("error hidden");
        if (pos != std::string::npos)
            html.erase(pos + 5, 7);
    }
    return html;
}

std::string   HttpResponse::redirectWithCookie(const std::string& location, const std::string& cookie)
{
    if (!cookie.empty())
        this->setCookie(cookie);
    this->setStatusCode(SEE_OTHER);
    this->setHeader("Location", location);
    return this->build();
}

std::string   HttpResponse::login(HttpRequest& request)
{
    int body = request.getBody();
    Session& session = request.getSession();
    
    std::string username = getData(body, "username");
    std::string password = getData(body, "password");

    if (username != session.getUserName() || password != session.getPassword())
    {
        this->setStatusCode(UNAUTHORIZED);
        std::string loginPage = loadLoginPage(true); // Load the login page with an error message
        if (loginPage.empty())
            return "<html><body><h1>Invalid username or password.</h1></body></html>";
        return loginPage;
    }

    this->setStatusCode(SEE_OTHER);
    this->setHeader("Location", "/profile.html");
    return "";
}

std::string   HttpResponse::signup(HttpRequest& request)
{
    int body = request.getBody();
    Session& session = request.getSession();
    
    std::string username = getData(body, "username");
    std::string password = getData(body, "password");

    session.setUserName(username);
    session.setPassword(password);

    this->setStatusCode(SEE_OTHER);
    this->setHeader("Location", "/login.html");
    return "";
}
/* * */

std::string   HttpResponse::handleRawBody(HttpRequest& request, const LocationConfig& location)
{
    int body_fd = request.getBody();
    std::string filename = "upload", req_path = request.getPath();
    std::string content_type;

    if (request.getHeaders().count("Content-type"))
        content_type = request.getHeaders().at("Content-type");
    else
        content_type = "application/octet-stream";
    std::string extension = Utils::getExtension(content_type);
    filename = "upload_" + Utils::to_string_c98(Utils::getCurrentTime()) + extension;
    filename = location.upload_path + "/" + filename;
    std::ofstream outfile(filename.c_str(), std::ios::binary);
    if (!outfile.is_open())
        return this->errorResponse(INTERNAL_SERVER_ERROR);
    const size_t BUF_SZ = 4096;
    char buf[BUF_SZ];
    while (true)
    {
        ssize_t bytes = read(body_fd, buf, BUF_SZ);
        if (bytes < 0)
            return this->errorResponse(INTERNAL_SERVER_ERROR);
        if (bytes == 0)
            break;
        outfile.write(buf, bytes);
        if (!outfile)
            return this->errorResponse(INTERNAL_SERVER_ERROR);
    }
    outfile.close();
    this->setStatusCode(CREATED);
    this->setHeader("Content-Type", "text/html");
    std::string response_body = "<html><body><h1>File uploaded successfully</h1></body></html>";
    this->setHeader("Content-Length", Utils::to_string_c98(response_body.size()));
    this->setBody(response_body);
    return this->build();
}

std::string HttpResponse::handleMultipartBody(HttpRequest& request, const LocationConfig& location)
{
    int body_fd = request.getBody(); 
    std::map<std::string, std::string> headers = request.getHeaders();
    std::string content_type;

    if (headers.count("Content-type")) content_type = headers.at("Content-type");
    else return this->errorResponse(BAD_REQUEST);

    size_t boundary_pos = content_type.find("boundary=");
    if (boundary_pos == std::string::npos)
        return this->errorResponse(BAD_REQUEST);

    std::string boundary = "--" + content_type.substr(boundary_pos + 9);
    std::string target_str = "\r\n" + boundary;
    std::vector<char> target(target_str.begin(), target_str.end());
    std::vector<char> buffer;
    std::string line;

    bool found_boundary = false;
    while (Utils::extractLine(buffer, body_fd, line)) {
        if (line == boundary) {
            found_boundary = true;
            break;
        }
    }
    if (!found_boundary)
        return this->errorResponse(BAD_REQUEST);

    while (true) {
        std::string file_name = "";
        bool is_file = false;

        // Parse Headers
        bool headers_complete = false;
        while (Utils::extractLine(buffer, body_fd, line)) {
            if (line.empty()) {
                headers_complete = true;
                break; // End of headers (\r\n\r\n)
            }
            
            if (line.find("Content-Disposition:") != std::string::npos) {
                size_t name_pos = line.find("filename=\"");
                if (name_pos != std::string::npos) {
                    name_pos += 10;
                    size_t end_pos = line.find("\"", name_pos);
                    if (end_pos != std::string::npos) {
                        file_name = line.substr(name_pos, end_pos - name_pos);
                        if (!file_name.empty()) is_file = true;
                    }
                }
            }
        }
        if (!headers_complete)
            return this->errorResponse(BAD_REQUEST);

        std::ofstream outfile;
        if (is_file) {
            std::string full_path = location.upload_path + "/" + file_name;
            outfile.open(full_path.c_str(), std::ios::binary);
            if (!outfile.is_open())
                return this->errorResponse(INTERNAL_SERVER_ERROR);
        }
        std::vector<char>::iterator it;
        size_t boundary_pos;
        size_t write_size;
        ssize_t bytes;
        while (true) {
            // Search for "\r\n--boundary" in the current buffer
            it = std::search(buffer.begin(), buffer.end(), target.begin(), target.end());
            if (it != buffer.end()) {
                boundary_pos = std::distance(buffer.begin(), it);
                if (is_file && outfile.is_open())
                    outfile.write(&buffer[0], boundary_pos);
                buffer.erase(buffer.begin(), buffer.begin() + boundary_pos);
                if (outfile.is_open()) outfile.close();
                found_boundary = true;
                break;
            } else {
                // Boundary NOT found. 
                // Write everything EXCEPT the last (boundary.size() - 1) bytes.
                // We keep those just in case the boundary is split between this read and the next!
                if (buffer.size() >= target.size()) {
                    write_size = buffer.size() - target.size() + 1;
                    if (is_file && outfile.is_open())
                        outfile.write(&buffer[0], write_size);
                    buffer.erase(buffer.begin(), buffer.begin() + write_size);
                }
                char tmp[8192];
                bytes = read(body_fd, tmp, sizeof(tmp));
                if (bytes <= 0) {
                    if (is_file && outfile.is_open() && !buffer.empty())
                        outfile.write(&buffer[0], buffer.size());
                    buffer.clear();
                    if (outfile.is_open()) outfile.close();
                    break;
                }
                buffer.insert(buffer.end(), tmp, tmp + bytes);
            }
        }

        if (!found_boundary)
            return this->errorResponse(BAD_REQUEST);

        // Process the boundary we left in the buffer
        // Because the buffer starts with "\r\n--boundary", the first extractLine call 
        // will find the \r\n immediately and return an empty line.
        std::string empty_line;
        if (!Utils::extractLine(buffer, body_fd, empty_line))
            return this->errorResponse(BAD_REQUEST);

        // extract boundary
        std::string bound_line;
        if (!Utils::extractLine(buffer, body_fd, bound_line))
            return this->errorResponse(BAD_REQUEST);
        // Check if it's the final boundary
        if (bound_line == boundary + "--")
            break;
    }

    this->setStatusCode(CREATED);
    this->setHeader("Content-Type", "text/html");
    std::string response_body = "<html><body><h1>Files uploaded successfully</h1><p><a href=\"/\">Back to Home Page</a></p></body></html>";
    this->setHeader("Content-Length", Utils::to_string_c98(response_body.size()));
    this->setBody(response_body);    
    return this->build();
}


std::string   HttpResponse::handleUpload(HttpRequest& request, const LocationConfig& location)
{
    if (!location.isMethodAllowed(request.getMethod()))
        return this->errorResponse(METHOD_NOT_ALLOWED);
    if (!location.upload_enable)
        return this->errorResponse(FORBIDDEN);
    if (!Utils::is_Writable(location.upload_path))
        return this->errorResponse(INTERNAL_SERVER_ERROR);
    std::string content_type = request.getHeaders().at("Content-type");
    if (content_type.find("multipart/form-data") != std::string::npos)
        return handleMultipartBody(request, location);
    else
        return handleRawBody(request, location);
}


std::string     HttpResponse::handlePOST(HttpRequest& request)
{
    std::string response_html;

    int fd = open(request.getBodyFilePath().c_str(), O_RDONLY);
    if (fd == -1)
        return this->errorResponse(INTERNAL_SERVER_ERROR);
    request.setBodyFile(fd);
    if (request.getPath() == "/login")
        response_html = this->login(request);
    else if (request.getPath() == "/signup")
        response_html = this->signup(request);
    else if (request.getPath() == "/logout")
        return this->redirectWithCookie("/login.html", "session_id=; Path=/; Max-Age=0; HttpOnly");
    else if (request.getPath() == "/toggle-theme")
    {
        const std::string theme_cookie = request.getTheme();
        std::string new_theme = (theme_cookie == "theme-light") ? "theme-dark" : "theme-light";
        return this->redirectWithCookie("/", "theme=" + new_theme + "; Path=/; Max-Age=3600; HttpOnly");
    }
    else if (request.getPath() == "/upload")
        return this->handleUpload(request, ConfigParser::findLocation(request.getPath(), this->_server));
    else
        return this->errorResponse(NOT_FOUND);
    this->setHeader("Content-Type", "text/html");
    this->setHeader("Content-Length", Utils::to_string_c98(response_html.size()));
    this->setBody(response_html);
    return this->build();
}