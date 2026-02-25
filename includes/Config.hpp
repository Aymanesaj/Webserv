#ifndef CONFIG_HPP
#define CONFIG_HPP


#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <map>
#include <cstdlib>

struct LocationConfig {
    std::string path;
    std::string root;
    std::string index;
    bool autoindex;
	bool upload_enable;
	std::string	cgi_ext;
	std::string	upload_path;
	std::string	cgi_path;
    std::vector<std::string> allowed_methods;
    std::map<size_t, std::string> error_pages;
	std::string return_url;

    LocationConfig() : autoindex(false) {} 
};

struct ServerConfig {
	size_t listen_port;
	std::string server_name;
	std::string root;
	std::string index;
	std::string host;
    size_t client_max_body_size;
    std::vector<LocationConfig> locations;
    std::map<size_t, std::string> error_pages;

    ServerConfig() : listen_port(80), client_max_body_size(1048576) {} 
};

class ConfigParser {
	private:
		std::vector<std::string> _tokens;
		size_t _index;
		std::vector<ServerConfig> _servers;

		void tokenize(const std::string& filename);

		std::string peek() const;
		std::string consume();
		void expect(const std::string& expected);
		bool isEOF() const;

		ServerConfig parseServer();
		LocationConfig parseLocation();
		size_t parseSizeT(const std::string& val);
		template<typename S>
		void	handle_error_page(S &it)
		{
			std::vector<std::string> tmp;
			while (!isEOF() && peek() != ";")
				tmp.push_back(consume());
			if (tmp.size() < 2)
				throw std::runtime_error("error_page requires arguments");
			std::string error_uri = tmp[tmp.size() - 1];
			tmp.pop_back();
			for (size_t i = 0; i < tmp.size(); i++)
				it.error_pages.insert(std::make_pair(parseSizeT(tmp[i]), error_uri));
			expect(";");
		}
		void assignString(std::string& dest);
		void assignSizeT(size_t& dest);
		void assignBool(bool& dest);
		void parse();

	public:
		ConfigParser(const std::string& filename);
		~ConfigParser();
		const std::vector<ServerConfig>& getServers() const;
};


#endif