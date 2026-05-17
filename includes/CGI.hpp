#include "../includes/HttpResponse.hpp"

class CGI
{
	private:
        std::string                         _script_path;
        std::string                         _interpreter;
        std::map<std::string, std::string>  _env;
        HttpRequest&                        _request;
        LocationConfig&                     _location;

		void         buildEnv();
        char**       getEnvArray();
        std::string  parseOutput(const std::string& output);
        void         freeEnvArray(char** env);
		
    public:
        
        CGI();
        std::string execute();
};

CGI::CGI()
{

}