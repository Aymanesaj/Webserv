#include "../includes/CGI.hpp"

bool HttpResponse::isCGI(const std::string& path, const LocationConfig& location)
{
	if (location.cgi.empty())
		return false;
	std::string cleanPath = path;
	size_t qpos = cleanPath.find('?');
	if (qpos != std::string::npos)
		cleanPath = cleanPath.substr(0, qpos);
    
    for (std::map<std::string, std::string>::const_iterator it = location.cgi.begin(); it != location.cgi.end(); ++it) {
        const std::string& ext = it->first;
        if (cleanPath.size() >= ext.size() && cleanPath.compare(cleanPath.size() - ext.size(), ext.size(), ext) == 0)
            return true;
    }
	return false;
}

void CGI::buildEnv()
{
	std::string uri = _request.getPath();
	std::string script_name;
	std::string path_info;
	size_t pos = std::string::npos;
	const std::string& ext = _ext;

	pos = uri.find(ext);
	if (pos != std::string::npos)
	{
		pos += ext.size();
		script_name = uri.substr(0, pos);
		path_info = uri.substr(pos);
	}
	_env["REQUEST_METHOD"] = _request.getMethod();
	_env["QUERY_STRING"] = _request.getQuery();
	_env["CONTENT_TYPE"] = _request.getContentType();
	_env["CONTENT_LENGTH"] = Utils::to_string_c98(static_cast<int>(_request.getBodySize()));
	char *abs = realpath(_script_path.c_str(), NULL);
	if (abs)
	{
		_env["SCRIPT_FILENAME"] = std::string(abs);
		free(abs);
	}
	else
		_env["SCRIPT_FILENAME"] = _script_path;
	_env["SCRIPT_NAME"] = script_name;
	_env["PATH_INFO"] = path_info;
	_env["SERVER_NAME"] = "localhost";
	_env["SERVER_PROTOCOL"] = _request.getVersion();
	_env["REDIRECT_STATUS"] = "200";
	_env["REMOTE_ADDR"] = "127.0.0.1";
}

void CGI::buildEnvArray()
{
    _envStrings.clear();
    _envPointers.clear();

    for (std::map<std::string, std::string>::iterator it = _env.begin();
         it != _env.end(); ++it)
    {
        _envStrings.push_back(it->first + "=" + it->second);
    }

    for (size_t i = 0; i < _envStrings.size(); ++i)
    {
        _envPointers.push_back(const_cast<char*>(_envStrings[i].c_str()));
    }

    _envPointers.push_back(NULL);
}


std::string CGI::parseOutput(const std::string& output)
{
	std::string body, headers, key, value, line;
	size_t pos, colon;
	std::map<std::string, std::string> headers_map;
	std::map<std::string, std::string>::iterator it;

	if (output.empty())
		return (_response.errorResponse(INTERNAL_SERVER_ERROR));

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
			Utils::trim(value);
			headers_map[key] = value;
		}
	}
	StatusCode statusCode = OK;
	if (headers_map.find("Status") != headers_map.end())
	{
		std::string statusStr = headers_map["Status"];
		int code = std::atoi(statusStr.c_str());
		if (code >= 100 && code <= 599)
			statusCode = static_cast<StatusCode>(code);
		headers_map.erase("Status");
	}
	if (headers_map.find("Location") != headers_map.end())
	{
		if (statusCode == OK)
			statusCode = SEE_OTHER;
		_response.setStatusCode(statusCode);
		_response.setHeader("Location", headers_map["Location"]);
		headers_map.erase("Location");
		for (it = headers_map.begin(); it != headers_map.end(); it++)
			_response.setHeader(it->first, it->second);
		return (_response.build());
	}

	if (headers_map.find("Content-Type") == headers_map.end())
		headers_map["Content-Type"] = "text/html";

	_response.setStatusCode(statusCode);
	_response.setBody(body);
	_response.setHeader("Content-Length", Utils::to_string_c98(body.size()));

	for (it = headers_map.begin(); it != headers_map.end(); it++)
		_response.setHeader(it->first, it->second);

	return (_response.build());

}

CGI::CGI(HttpRequest& request, LocationConfig& location, const std::string& script_path,
	HttpResponse& response)
	: _script_path(script_path), _request(request), _location(location), _response(response)
{
	for (std::map<std::string, std::string>::iterator it = location.cgi.begin(); it != location.cgi.end(); ++it) {
        if (_script_path.size() >= it->first.size() && _script_path.compare(_script_path.size() - it->first.size(), it->first.size(), it->first) == 0) {
            _ext = it->first;
            _interpreter = it->second;
            break;
        }
    }
}