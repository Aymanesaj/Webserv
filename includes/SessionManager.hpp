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
        std::string _password;
        int         _maxAge; // in seconds
        time_t      _lastAccessTime;
    public:
        Session( void );
        const std::string&  getId( void ) const;
        const std::string&  getUserName( void ) const;
        const std::string&  getPassword( void ) const;
        const time_t&       getLastAccessTime( void ) const;
        void                setId(const std::string& id);
        void                setUserName(const std::string& name);
        void                setPassword(const std::string& password);
        void                setLastAccessTime(time_t currentTime);
        int                 getMaxAge( void ) const;
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
        void        removeSession(const std::string& sessionId);
};

#endif
