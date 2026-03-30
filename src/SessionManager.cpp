#include "../includes/SessionManager.hpp"
#include "../includes/HttpRequest.hpp"

SessionManager::SessionManager() {}

Session::Session() : _userName("Guest"), _counter(0) {}

void    SessionManager::setUpSession(HttpRequest& request)
{
    const std::string cookie_str = request.getCookies();
    std::vector<std::string> cookie;// store cookie as cookie=value
    std::vector<std::string> tmp;
    std::map<std::string, std::string> cookies;

    cookie = Utils::split(cookie_str, ";");
    for (size_t i = 0; i < cookie.size(); i++)
    {
        Utils::trim(cookie[i]);
        tmp = Utils::split(cookie[i], "=");
        if (tmp.size() != 2)
            continue ;
        cookies[tmp[0]] = tmp[1];
    }

    std::string session_id;
    std::map<std::string, std::string>::iterator it = cookies.find("session_id");
    if (it != cookies.end() && this->_sessions.find(it->second) != this->_sessions.end())
        session_id = it->second;
    else
        session_id = this->createSession();
    Session& session = this->getSession(session_id);
    if (session.getId().empty())
        session.setId(session_id);
    /*
        *   process other cookies here
    */
    request.setSession(session);
}

Session&    SessionManager::getSession(const std::string& sessionId)
{
    return this->_sessions[sessionId];
}

std::string SessionManager::createSession()
{
    const std::string   charset = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const size_t        length = 16;
    std::string         id;
    
    std::ifstream urandom("/dev/urandom", std::ios::binary);
    if (urandom.is_open()) 
    {
        char byte;
        for (size_t i = 0; i < length; i++)
        {
            urandom.read(&byte, 1);
            id += charset[static_cast<unsigned char>(byte) % charset.size()];
        }
        urandom.close();
    } else {
        std::srand(static_cast<unsigned int>(std::time(NULL)));
        for (size_t i = 0; i < length; i++)
            id += charset[std::rand() % charset.size()];
    }
    this->_sessions[id] = Session();
    this->_sessions[id].setId(id);
    return id;
}

int Session::getCounter( void ) const
{
    return this->_counter;
}

void Session::incrementCounter( void )
{
    this->_counter++;
}

void Session::setId(const std::string& id)
{
    this->_id = id;
}

std::string Session::getId( void ) const
{
    return this->_id;
}
