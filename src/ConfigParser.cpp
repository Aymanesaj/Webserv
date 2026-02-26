#include "Config.hpp"

void ConfigParser::tokenize(const std::string& filename)
{
	std::ifstream file(filename.c_str());

	if (!file.is_open()){
		throw std::runtime_error("could not open file : " + filename);
	}
	std::string current_word = "";
	char c;
		
	while (file.get(c))
	{
		if (c == '#') { while (c != '\n' && file.get(c)) { /* skippi a 3abd samad */ } }
		else if (c == '{' || c == '}' || c == ';') {
			if (!current_word.empty()) {
				_tokens.push_back(current_word);
				current_word = "";
			}
			_tokens.push_back(std::string(1, c));
		}
		else if (std::isspace(c) && !current_word.empty())
		{
			_tokens.push_back(current_word);
			current_word = "";
		}
		else if (!std::isspace(c))
			current_word += c;
	}
		
	if (!current_word.empty())
		_tokens.push_back(current_word);
}

void ConfigParser::assignString(std::string& dest) {
    dest = consume();
    expect(";");
}

void ConfigParser::assignSizeT(size_t& dest) 
{
    std::string val = consume();
    for (size_t i = 0; i < val.length(); ++i)
        if (!std::isdigit(val[i]))
            throw std::runtime_error("Invalid number format: " + val);
    std::stringstream ss(val);
    ss >> dest;
    if (ss.fail())
        throw std::runtime_error("Number overflow: " + val);
    expect(";");
}

size_t ConfigParser::parseSizeT(const std::string& val)
{
    for (size_t i = 0; i < val.length(); ++i)
        if (!std::isdigit(val[i]))
            throw std::runtime_error("Invalid error code format: " + val);
    std::stringstream ss(val);
    size_t result;
    ss >> result;
    if (ss.fail())
        throw std::runtime_error("Error code overflow: " + val);
    return result;
}

void ConfigParser::assignBool(bool& dest) {
    std::string val = consume();
    if (val == "on") dest = true;
    else if (val == "off") dest = false;
    else throw std::runtime_error("Expected 'on' or 'off'");
    expect(";");
}

std::string ConfigParser::peek() const
{
	if (isEOF())
		return "";
	return _tokens[_index];
}

std::string ConfigParser::consume()
{
	if (isEOF())
		return "";
	return _tokens[_index++];
}

void ConfigParser::expect(const std::string& expected)
{
	if (peek() == expected)
		consume();
	else
		throw std::runtime_error("Syntax Error: Expected '" + expected + "' but found '" + peek() + "'");
}

bool ConfigParser::isEOF() const
{
	if (_index >= _tokens.size())
		return true;
	return false;
}

LocationConfig ConfigParser::parseLocation()
{
	LocationConfig location;
	location.path = consume();
	expect("{");
	while (!isEOF() && peek() != "}")
	{
		std::string	directive = consume();
		if (directive == "root")
			assignString(location.root);
		else if (directive == "return")
			assignString(location.return_url);
		else if (directive == "cgi_path")
			assignString(location.cgi_path);
		else if (directive == "index")
			assignString(location.index);
		else if (directive == "upload_store")
			assignString(location.upload_path);
		else if (directive == "cgi_extension")
			assignString(location.cgi_ext);
		else if (directive == "error_page")
			handle_error_page(location);
		else if (directive == "upload_enable")
			assignBool(location.upload_enable);
		else if (directive == "autoindex")
			assignBool(location.autoindex);
		else if (directive == "methods")
		{
			while (!isEOF() && peek() != ";")
			{
				std::string method = consume();
				if (method == "GET" || method == "POST" || method == "DELETE")
					location.allowed_methods.push_back(method);
				else
					throw std::runtime_error("Unsupported method");
			}
			if (!location.allowed_methods.size())
				throw std::runtime_error("You should provide the methods");
			expect(";");
		}
		else
			throw std::runtime_error("Unknown directive in location block: " + directive);
	}
	expect("}");
	return (location);
}

ServerConfig ConfigParser::parseServer()
{
	ServerConfig server;
	while (!isEOF() && peek() != "}")
	{
		std::string	directive = consume();
		if (directive == "root")
			assignString(server.root);
		else if (directive == "index")
			assignString(server.index);
		else if (directive == "error_page")
			handle_error_page(server);
		else if (directive == "listen")
			assignSizeT(server.listen_port);
		else if (directive == "server_name")
			assignString(server.server_name);
		else if (directive == "host")
			assignString(server.host);
		else if (directive == "location")
			server.locations.push_back(parseLocation());
		else if (directive == "client_max_body_size") { server.client_max_body_size = std::atol(consume().c_str()); expect(";"); }
		else throw std::runtime_error("Unknown directive in server block: " + directive);
	}
	expect("}");
	return (server);
}

void ConfigParser::parse()
{
	while (!isEOF())
	{
		expect("server");
		expect("{");
		_servers.push_back(parseServer());
	}
}

void	ConfigParser::validate(){
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		if (_servers[i].listen_port == 0 || _servers[i].listen_port > 65535)
			throw std::runtime_error("Invalid listen port");
		if (_servers[i].client_max_body_size == 0)
			throw std::runtime_error("Client max body size is > 0.");
		std::vector<LocationConfig> &__locations = _servers[i].locations;
		for (size_t j = 0; j < __locations.size(); ++j)
			for (size_t k = j + 1; k < __locations.size(); ++k)
				if (__locations[j].path == __locations[k].path)
					throw std::runtime_error("Ambiguous routing.");
		for (size_t idx = 0; idx < __locations.size(); ++idx)
		{
			if (__locations[idx].allowed_methods.empty())
				throw std::runtime_error("There is no location method");
			if ((!__locations[idx].cgi_ext.empty() && __locations[idx].cgi_path.empty())
				|| (__locations[idx].cgi_ext.empty() && !__locations[idx].cgi_path.empty()))
				throw std::runtime_error("CGI parameters has only one of (extention or path) need both.");
			if (__locations[idx].upload_enable && __locations[idx].upload_path.empty())
				throw std::runtime_error("Upload enabled with no path.");
			if (__locations[idx].root.empty())
				__locations[idx].root = _servers[i].root;
		}
	}
	for (size_t i = 0; i < _servers.size(); ++i)
		for (size_t j = i + 1; j < _servers.size(); ++j)
			if (_servers[i].host == _servers[j].host
				&& _servers[i].listen_port == _servers[j].listen_port
				&& _servers[i].server_name == _servers[j].server_name)
					throw std::runtime_error("host:port collision");
}

ConfigParser::ConfigParser(const std::string& filename):_index(0)
{
	tokenize(filename);
	if (!_tokens.size())
		throw std::runtime_error("Empty config file");
	parse();
	validate();
}

ConfigParser::~ConfigParser() { }

const std::vector<ServerConfig>& ConfigParser::getServers() const { return (_servers); }