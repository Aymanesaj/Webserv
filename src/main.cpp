#include "../includes/Config.hpp"
#include "../includes/HttpParser.hpp"
#include "../includes/HttpResponse.hpp"
#include "../includes/Server.hpp"

int main(int argc, char **argv)
{
    if (argc != 2) 
    {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }
    try 
    {
        ConfigParser    parser(argv[1]);
        Server          server;
        server.init(parser.getServers());
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}