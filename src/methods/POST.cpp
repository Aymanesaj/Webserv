#include "../../includes/HttpResponse.hpp"

/* session and cookie handling */
/* * */
static std::string getData(const std::string& body, const std::string& key)
{
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
    std::string body = request.getBody();
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
    std::string body = request.getBody();
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

std::string     HttpResponse::handlePOST(HttpRequest& request)
{
    std::string response_html;
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
    else
        return this->errorResponse(NOT_FOUND);
    this->setHeader("Content-Type", "text/html");
    this->setHeader("Content-Length", Utils::to_string_c98(response_html.size()));
    this->setBody(response_html);
    return this->build();
}