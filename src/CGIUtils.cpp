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
std::string uri = _request.getPath();
std::string script_name;
std::string path_info;
size_t pos = std::string::npos;

if ((pos = uri.find(".php")) != std::string::npos)
    pos += 4;         
else if ((pos = uri.find(".py")) != std::string::npos)
    pos += 3;          
if (pos != std::string::npos)
{
    script_name = uri.substr(0, pos);
    path_info = uri.substr(pos);
}
	_env["REQUEST_METHOD"] = _request.getMethod();
	_env["QUERY_STRING"] = _request.getQuery();
	_env["CONTENT_TYPE"] = _request.getContentType();
	_env["CONTENT_LENGTH"] = Utils::to_string_c98(_request.getBody());
	_env["SCRIPT_FILENAME"] = _script_path;
	_env["SCRIPT_NAME"] = script_name;
	_env["PATH_INFO"] = path_info;
	_env["SERVER_NAME"] = "localhost";
	_env["SERVER_PORT"] = "8080";
	_env["SERVER_PROTOCOL"] = _request.getVersion();
	_env["REDIRECT_STATUS"] = "200";
	_env["REMOTE_ADDR"] = "127.0.0.1";
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
		tmp = it->first + "=" + it->second;
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

std::string CGI::parseOutput(const std::string& output)
{
	std::string body, headers, key, value, line;
	size_t pos, colon;
	std::map<std::string, std::string> headers_map;
	std::map<std::string, std::string>::iterator it;

	if (output.empty())
		return (this->errorResponse(INTERNAL_SERVER_ERROR));

	pos = output.find("\r\n\r\n");
	if (pos != std::string::npos)
	{
		headers = output.substr(0, pos);
		body = output.substr(pos + 4);
	}
	else if ((output.find("\n\n")) != std::string::npos)
	{
		pos = output.find("\n\n");
		headers = output.substr(0, pos);
		body = output.substr(pos + 2);
	}
	else
	{
		headers = "";
		body = output;
	}

	std::istringstream stream(headers);
	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line = line.substr(0, line.size() - 1);
		colon = line.find(":");
		if (colon != std::string::npos)
		{
			key = line.substr(0, colon);
			value = line.substr(colon + 1);
			headers_map[key] = value;
		}
	}

	if (headers_map.find("Content-Type") == headers_map.end())
		headers_map["Content-Type"] = "text/html";

	this->setStatusCode(OK);
	this->setBody(body);
	this->setHeader("Content-Length", Utils::to_string_c98(body.size()));

	for (it = headers_map.begin(); it != headers_map.end(); it++)
		this->setHeader(it->first, it->second);

	return (this->build());

}

CGI::CGI(HttpRequest& request, LocationConfig& location, const std::string& script_path)
	: _script_path(script_path), _request(request), _location(location)
{
	if (script_path.find(".php") != std::string::npos)
		_interpreter = "/usr/bin/php-cgi";
	else if (script_path.find(".py") != std::string::npos)
		_interpreter = "/usr/bin/python3";

}