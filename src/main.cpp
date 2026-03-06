// #include "Config.hpp"
#include "../includes/HttpParser.hpp"

int main(int argc, char **argv) {
    // if (argc != 2) {
    //     std::cerr << "Usage: ./webserv [config_file]" << std::endl;
    //     return 1;
    // }
    // try {
    //     ConfigParser parser(argv[1]);
    //     std::cout << "✅ Configuration parsed and validated successfully!" << std::endl;
    // } catch (const std::exception& e) {
    //     std::cerr << e.what() << std::endl;
    // }
    (void) argc;
    (void) argv;

    HttpParser parse;
    ParseResult res = parse.parseRequest("GET /index.html HTTP/1.1\r\nHost: otmn\r\n\r\n\r\nwalo");
    const HttpRequest request = parse.getRequest();
    
    if (res != COMPLETE)
        std::cout << "Error found and status code is : " << parse.getErrorCode() << std::endl;
    else
    {
        std::cout << "Request Parsed Succefully" << std::endl;
        std::cout << "Request line" << std::endl;
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
        
        // std::cout << "Buffer remaining :"<< std::endl;
        // std::cout << "[" <<  parse.buffer << "]" << std::endl;
        // std::cout << "+++++++" << std::endl;

    }

    return 0;
}