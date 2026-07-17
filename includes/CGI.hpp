#include "../includes/HttpResponse.hpp"

class CGI : protected HttpResponse 
{
	private:
        std::string                         _script_path;
        std::string                         _interpreter;
        std::map<std::string, std::string>  _env;
        HttpRequest&                        _request;
        LocationConfig&                     _location;
        std::vector<std::string> _envStrings;
        std::vector<char*> _envPointers;

        void         buildEnvArray();
        std::string  parseOutput(const std::string& output);
		void         buildEnv();
		
    public:
        
        CGI(HttpRequest& request, LocationConfig& location,
                   const std::string& script_path);
        std::string execute();
};