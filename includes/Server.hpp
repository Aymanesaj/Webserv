#ifndef SERVER_HPP
#define SERVER_HPP

#include "libs.hpp"
#include "Config.hpp"
#include "SessionManager.hpp"

class Server
{
    private:
        std::set<int> listening_sockets;
        std::vector<pollfd> fds;
        std::map<int, std::string> connections;
        std::map<int, std::vector<ServerConfig> > socket_servers;
        SessionManager      sessions_manager;
    public:
        void init(const std::vector<ServerConfig>& servers);
        void run();
        ~Server();

    private:
        void acceptClient(size_t i);
        void readRequest(size_t& i);
        void removeClient(size_t& i);
};

#endif