#include "../includes/CGI.hpp"

bool HttpResponse::isCGI(const std::string& path)
{
	if (path.find(".php") != std::string::npos
		|| path.find(".py") != std::string::npos)
		return true;
	return false;
}

void CGI::buildEnv()
{
	std::string script_name;

	if (_request.getPath().find("/cgi-bin/") != std::string::npos)
		script_name = _request.getPath().substr(_request.getPath().find("/cgi-bin/"));
	else
		script_name = _request.getPath();
	_env["REQUEST_METHOD "] = _request.getMethod();
	_env["QUERY_STRING"] = _request.getQuery();
	_env["CONTENT_TYPE"] = _request.getContentType();
	_env["CONTENT_LENGTH"] = _request.getBody().size();
	_env["SCRIPT_FILENAME"] = _script_path;
	_env["SCRIPT_NAME"] = script_name;
	_env["SERVER_NAME"] = "localhost";
	_env["SERVER_PORT"] = "8080";
	_env["SERVER_PROTOCOL"] = _request.getVersion();
	_env["REMOTE_ADDR"] = OK;
	_env["HTTP_COOKIE"] = _request.getCookies();
}

char**       CGI::getEnvArray()
{
	char **envp = new char*[_env.size() + 1];
	size_t i = 0;
	std::string tmp;
	std::map<std::string, std::string>::iterator it;
	for (it = _env.begin() ; it != _env.end(); it++)
	{
		tmp = it->first + "+" + it->second;
		envp[i] = new char[tmp.size() + 1];
		std::strcpy(envp[i], tmp.c_str());
		i++;
	}
	envp[i] = NULL;
	return (envp);
}

void CGI::freeEnvArray(char** env)
{
    for (int i = 0; env[i] != NULL; i++)
        delete[] env[i];
    delete[] env;
}

CGI::CGI()
{
	
}