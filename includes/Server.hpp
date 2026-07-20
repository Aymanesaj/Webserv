#ifndef SERVER_HPP
#define SERVER_HPP
#include "Config.hpp"
#include "SessionManager.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"

struct ClientState {
    std::string outbuf;
    bool        closeAfterWrite;

    ClientState() : closeAfterWrite(false) {}
};

struct CgiState {
    pid_t       pid;
    int         pipeFd;         // read end of CGI stdout pipe
    int         pipeFdIn;       // write end of CGI stdin pipe
    int         bodyFd;         // fd of the POST body file
    std::string inputBuf;       // buffer for data read from bodyFd
    int         clientFd;
    time_t      startTime;
    std::string outputBuf;
    HttpResponse response;
    HttpRequest* request;
    bool        closeConn;

    CgiState() : pid(-1), pipeFd(-1), pipeFdIn(-1), bodyFd(-1), clientFd(-1), startTime(0),
                 request(NULL), closeConn(false) {}
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
        std::map<int, CgiState> cgi_processes;
        std::set<int> cgi_pipe_fds;
        std::map<int, int> cgi_in_to_out; // maps pipeFdIn to pipeFd
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
        void handleCgiRequest(size_t& i, HttpRequest& request, HttpResponse& response,
                              LocationConfig& location, const std::string& cgiPath,
                              int fd, bool closeConn, bool isLogout);
        void writeResponse(size_t& i);
        void queueResponse(size_t& i, const std::string& resp, bool closeAfter);
        void removeClient(size_t& i);
        void handleCgiRead(size_t& i);
        void handleCgiWrite(size_t& i);
        void checkCgiTimeouts();
        void finalizeCgiResponse(int pipeFd, int exitStatus);
        size_t findFdIndex(int fd);
};

#endif