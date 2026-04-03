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
	bool has_root;
	bool has_return;
	bool has_cgi_path;
	bool has_cgi_extension;
	bool has_index;
	bool has_upload_store;
	bool has_upload_enable;
	bool has_autoindex;
	bool has_methods;

    LocationConfig();
};

struct ServerConfig {
	size_t listen_port;
	std::string server_name;
    size_t client_max_body_size;
	std::string host;
	std::string root;
	std::string index;
    std::vector<LocationConfig> locations;
    std::map<size_t, std::string> error_pages;
	bool has_listen;
	bool has_host;
	bool has_server_name;
	bool has_root;
	bool has_index;
	bool has_client_max_body_size;
	ServerConfig();
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
		void validate();

	public:
		ConfigParser(const std::string& filename);
		~ConfigParser();
		const std::vector<ServerConfig>& getServers() const;
};


#endif