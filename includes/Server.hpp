#ifndef SERVER_HPP
#define SERVER_HPP

#include "libs.hpp"
#include "Config.hpp"
#include "SessionManager.hpp"
#include "HttpParser.hpp"

class Server
{
    private:
	    std::map<std::pair<std::string,int>, std::vector<ServerConfig> > groups;
        std::set<int> listening_sockets;
        std::vector<pollfd> fds;
        std::map<int, std::string> connections;
        std::map<int, std::vector<ServerConfig> > socket_servers;
        SessionManager      sessions_manager;
        std::map<int, HttpParser> parse;
        std::map<int, int> client_to_server_socket;
    public:
        void init(const std::vector<ServerConfig>& servers);
        void run();
        ~Server();
        size_t getClientMaxBodySize(const std::string& hostHeader, int client_fd);

    private:
        ServerConfig&   getServer(std::string host, int fd);
        void acceptClient(size_t& i);
        void readRequest(size_t& i);
        void removeClient(size_t& i);
};

#endif