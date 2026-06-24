#ifndef SERVER_HPP
#define SERVER_HPP
#include "Config.hpp"
#include "SessionManager.hpp"
#include "HttpParser.hpp"

struct ClientState {
    std::string outbuf;
    bool        closeAfterWrite;

    ClientState() : closeAfterWrite(false) {}
};

class Server
{
    private:
	    std::map<std::pair<std::string,int>, std::vector<ServerConfig> > groups;
        std::set<int> listening_sockets;
        std::vector<pollfd> fds;
        std::map<int, std::vector<ServerConfig> > socket_servers;
        SessionManager      sessions_manager;
        std::map<int, HttpParser> parse;
        std::map<int, int> client_to_server_socket;
        std::map<int, ClientState> clients;
        static volatile sig_atomic_t flag;
    public:
        void init(const std::vector<ServerConfig>& servers);
        void run();
        ~Server();
        size_t getClientMaxBodySize(const std::string& hostHeader, int client_fd);

    private:
        static void signal_handler(int signum);
        ServerConfig&   getServer(std::string host, int fd);
        void acceptClient(size_t& i);
        void readRequest(size_t& i);
        void writeResponse(size_t& i);
        void queueResponse(size_t& i, const std::string& resp, bool closeAfter);
        void removeClient(size_t& i);
};

#endif