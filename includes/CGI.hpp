#ifndef CGI_HPP
#define CGI_HPP

#include "../includes/HttpResponse.hpp"

struct CgiProcessInfo {
    pid_t pid;
    int   pipeFd;
    int   pipeFdIn;
    bool  ok; // to check if process started successfully

    CgiProcessInfo() : pid(-1), pipeFd(-1), pipeFdIn(-1), ok(false) {}
};

class CGI
{
	private:
        std::string                         _script_path;
        std::string                         _ext;
        std::string                         _interpreter;
        std::map<std::string, std::string>  _env;
        HttpRequest&                        _request;
        LocationConfig&                     _location;
        HttpResponse&                       _response;
        std::vector<std::string> _envStrings;
        std::vector<char*> _envPointers;

        void         buildEnvArray();
        void         buildEnv();
		
    public:
        
        CGI(HttpRequest& request, LocationConfig& location,
                   const std::string& script_path, HttpResponse& response);

        CgiProcessInfo startProcess();
        std::string  parseOutput(const std::string& output);
};

#endif