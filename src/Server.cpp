#include "Server.hpp"
#include "Config.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"

void Server::init(const std::vector<ServerConfig>& servers)
{
	std::map<std::pair<std::string,int>, std::vector<ServerConfig> > groups;
	for (size_t i = 0; i < servers.size(); ++i)
	{
		std::pair<std::string,int> key = std::make_pair(servers[i].host, servers[i].listen_port);
		groups[key].push_back(servers[i]);
	}
	for (std::map<std::pair<std::string,int>, std::vector<ServerConfig> >::iterator it = groups.begin(); it != groups.end(); ++it)
	{
		std::string host = it->first.first;
		int port = it->first.second;
		// fd leaks handled inside the catch block later
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
			throw std::runtime_error("Socket failed");
		listening_sockets.insert(fd);
		// adding listening sockets early just because if there is an error in the next functions i'll handle leaks in the catch block
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = inet_addr(host.c_str());
		if (addr.sin_addr.s_addr == INADDR_NONE)
			throw std::runtime_error("Host address failed");
		int opt = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    		throw std::runtime_error("setsockopt failed");
		if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
			throw std::runtime_error("Bind failed");
		if (listen(fd, SOMAXCONN) < 0)
			throw std::runtime_error("Listen failed");
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
			throw std::runtime_error("Fcntl failed");
		pollfd p = {fd, POLLIN, 0};
		fds.push_back(p);
		socket_servers[fd] = it->second;
	}
	run();
}

void Server::run()
{
	while (true){
		int ready = poll(fds.data(), fds.size(), -1);
		if (!ready)
			continue;
		if (ready < 0)
			throw std::runtime_error("Poll failed");
		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & (POLLHUP | POLLERR))
			{
				removeClient(i);
				continue;
			}
			if (fds[i].revents & POLLIN)
			{
				if (listening_sockets.count(fds[i].fd))
					acceptClient(i);
				else
					readRequest(i);
			}
		}
	}
}

void Server::acceptClient(size_t i)
{
	sockaddr client_addr = {};
	socklen_t len = sizeof(client_addr);
	int client_fd = accept(fds[i].fd, &client_addr, &len);
	if (client_fd < 0)
		return ;
	int flags = fcntl(client_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed");
	pollfd p = {client_fd, POLLIN, 0};
	fds.push_back(p);
}

void Server::readRequest(size_t& i)
{
	char buffer[4096];
	int fd = fds[i].fd;
	ssize_t bytes = read(fd, buffer, sizeof(buffer));
	if (bytes <= 0){
		removeClient(i);
		return ;
	}
	connections[fd].append(buffer, bytes);
	/// 3andaaaaaak
	if (connections[fd].find("\r\n\r\n") == std::string::npos)
		return ;
	HttpParser parse;
	HttpResponse response;
	try
	{
		if (parse.parseRequest(connections[fd]) == INCOMPLETE)
			throw std::runtime_error("JUST FOR TESTING:  Request is incomplete");
		std::cout << parse.getRequest().getMethod() << " " << parse.getRequest().getPath() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		std::string error_resp = response.errorResponse(static_cast<StatusCode>(parse.getErrorCode()));
		write(fd, error_resp.c_str(), error_resp.size());
		removeClient(i);
		return ;
	}
	std::string raw_resp = response.handleRequest(parse.getRequest());
	write(fd, raw_resp.c_str(), raw_resp.size());
	removeClient(i);
}

void Server::removeClient(size_t& i)
{
	close(fds[i].fd);
	connections.erase(fds[i].fd);
	fds.erase(fds.begin() + i);
	--i;
}
