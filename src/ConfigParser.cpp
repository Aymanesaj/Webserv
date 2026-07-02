#include "../includes/Config.hpp"

LocationConfig::LocationConfig() : autoindex(false), upload_enable(false) {
	has_root = false;
	has_return = false;
	has_cgi_path = false;
	has_cgi_extension = false;
	has_index = false;
	has_upload_store = false;
	has_upload_enable = false;
	has_autoindex = false;
	has_methods = false;
}

ServerConfig::ServerConfig() : listen_port(80), client_max_body_size(1048576), host("0.0.0.0"), root("./www"), index("index.html") {
	has_listen = false;
	has_host = false;
	has_server_name = false;
	has_root = false;
	has_index = false;
	has_client_max_body_size = false;
}

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
		std::string directive = consume();
		if (directive == "root" && (!location.has_root && (location.has_root = true)))
			assignString(location.root);
		else if (directive == "return" && (!location.has_return && (location.has_return = true)))
			assignString(location.return_url);
		else if (directive == "cgi_path" && (!location.has_cgi_path && (location.has_cgi_path = true)))
			assignString(location.cgi_path);
		else if (directive == "index" && (!location.has_index && (location.has_index = true)))
			assignString(location.index);
		else if (directive == "upload_store" && (!location.has_upload_store && (location.has_upload_store = true)))
			assignString(location.upload_path);
		else if (directive == "cgi_extension" && (!location.has_cgi_extension && (location.has_cgi_extension = true)))
			assignString(location.cgi_ext);
		else if (directive == "error_page")
			handle_error_page(location);
		else if (directive == "upload_enable" && (!location.has_upload_enable && (location.has_upload_enable = true)))
			assignBool(location.upload_enable);
		else if (directive == "autoindex" && (!location.has_autoindex && (location.has_autoindex = true)))
			assignBool(location.autoindex);
		else if (directive == "methods" && (!location.has_methods && (location.has_methods = true)))
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
			throw std::runtime_error("Error: " + directive);
	}
	expect("}");
	return location;
}

ServerConfig ConfigParser::parseServer()
{
	ServerConfig server;
	while (!isEOF() && peek() != "}")
	{
		std::string	directive = consume();
		if (directive == "root" && (!server.has_root && (server.has_root = true)))
			assignString(server.root);
		else if (directive == "index" && (!server.has_index && (server.has_index = true)))
			assignString(server.index);
		else if (directive == "error_page")
			handle_error_page(server);
		else if (directive == "listen" && (!server.has_listen && (server.has_listen = true)))
			assignSizeT(server.listen_port);
		else if (directive == "server_name" && (!server.has_server_name && (server.has_server_name = true)))
			assignString(server.server_name);
		else if (directive == "host" && (!server.has_host && (server.has_host = true)))
			assignString(server.host);
		else if (directive == "location")
			server.locations.push_back(parseLocation());
		else if (directive == "client_max_body_size" && (!server.has_client_max_body_size && (server.has_client_max_body_size = true)))
			assignSizeT(server.client_max_body_size);
		else throw std::runtime_error("Error: " + directive);
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

// LocationConfig ConfigParser::findLocation(const std::string& uri, const ServerConfig& server)
// {
//     int best_match_length = -1;
//     int bestIndex = -1;

//     for (int i = 0; i < (int)server.locations.size(); i++)
//     {
//         if (uri.find(server.locations[i].path) == 0)
//         {
//             if ((int)server.locations[i].path.length() > best_match_length)
//             {
//                 best_match_length = server.locations[i].path.length();
//                 bestIndex = i;
//             }
//         }
//     }
//     if (bestIndex == -1)
//         throw std::runtime_error("Location not found");
//     return server.locations[bestIndex];
// }

LocationConfig ConfigParser::findLocation(const std::string& uri,
                                          const ServerConfig& server)
{
    size_t bestLength = 0;
    int bestIndex = -1;

    for (size_t i = 0; i < server.locations.size(); ++i)
    {
        const std::string &loc = server.locations[i].path;

        if (uri.compare(0, loc.size(), loc) != 0)
            continue;
        if (uri.size() == loc.size())
        {
            if (loc.size() > bestLength)
            {
                bestLength = loc.size();
                bestIndex = i;
            }
        }
        else if (loc == "/" || (uri.size() > loc.size() && uri[loc.size()] == '/'))
        {
            if (loc.size() > bestLength)
            {
                bestLength = loc.size();
                bestIndex = i;
            }
        }
    }
    if (bestIndex == -1)
        throw std::runtime_error("Location not found");

    return server.locations[bestIndex];
}

void	ConfigParser::validate(){
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		if (_servers[i].host == "localhost")
			_servers[i].host = "127.0.0.1";
		if (_servers[i].listen_port == 0 || _servers[i].listen_port > 65535)
			throw std::runtime_error("Invalid listen port");
		if (_servers[i].client_max_body_size <= 0)
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

const std::vector<ServerConfig>& ConfigParser::getServers() { return (_servers); }