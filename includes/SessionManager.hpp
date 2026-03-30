#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include "Utils.hpp"
#include "libs.hpp"

class HttpRequest;

class Session
{
    private:
        std::string _id;// session ID
        std::string _userName;
        int         _counter;
    public:
        Session( void );
        std::string getId( void ) const;
        std::string getUserName( void ) const;
        void        setId(const std::string& id);
        void        setUserName(const std::string& name);
        void        incrementCounter( void );
        int         getCounter( void ) const;
};

class HttpRequest;

class SessionManager
{
    private:
        std::map<std::string, Session> _sessions; // sessionId -> sessionData
    public:
        SessionManager( void );
        void        setUpSession(HttpRequest& request);
        std::string createSession( void );
        Session&    getSession(const std::string& sessionId);
};

#endif
