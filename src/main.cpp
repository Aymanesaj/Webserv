#include "Config.hpp"
#include "../includes/HttpParser.hpp"


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: ./webserv [config_file]" << std::endl;
        return 1;
    }
    try {
        ConfigParser parser(argv[1]);
        std::cout << "✅ Configuration parsed and validated successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    HttpParser parse;
    try
    {
        ParseResult res = parse.parseRequest("\r\nGET /index.html HTTP/1.1\r\nHost: localhost:8080\r\ntest: hey\r\n\r\n\r\n");
        if (res == INCOMPLETE)
            throw std::runtime_error("JUST FOR TESTING:  Request is incomplete");
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "status code is : " << parse.getStatusCode() << std::endl;
        return 1;
    }
    const HttpRequest request = parse.getRequest();

    std::cout << "\nRequest line" << std::endl;
    std::cout << "*******" << std::endl;
    std::cout << "Method : " << request.getMethod() << std::endl;
    std::cout << "Path : " << request.getPath() << std::endl;
    std::cout << "Version : " << request.getVersion() << std::endl;
    std::cout << "*******" << std::endl << std::endl;

    std::cout << "Request Headers:" << std::endl;
    std::cout << "+++++++" << std::endl;
    const std::map<std::string, std::string>& headers = request.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end();   ++it)
    {
        std::cout << it->first << ": " << it->second << std::endl;
    }
    std::cout << "+++++++" << std::endl;
    
    std::cout << "\nBody:"<< std::endl;
    std::cout << "[" << request.getBody() << "]" << std::endl;
    std::cout << "+++++++" << std::endl;

    return 0;
}