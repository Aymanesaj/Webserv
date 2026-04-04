#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include "../includes/HttpParser.hpp"
#include "../includes/HttpResponse.hpp"

Server::~Server()
{
	for (size_t i = 0; i < fds.size(); i++)
		close(fds[i].fd);
}

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
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
			throw std::runtime_error("Socket failed");
		listening_sockets.insert(fd);
		pollfd p = {fd, POLLIN, 0};
		fds.push_back(p);
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
			fds[i].revents = 0;
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
	connections[fd].assign(buffer, bytes);
    HttpResponse response;
    HttpRequest& request = parse[fd].getRequest();
	try
	{
		if (parse[fd].parseRequest(connections[fd]) == INCOMPLETE)
			return ;
		std::string cookie = request.getCookies();		
		std::cout
            << request.getMethod()
            << " " << request.getPath()
            << " " << request.getVersion()
            << " host=" << request.getHeaders().at("Host");
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << " : " << static_cast<StatusCode>(parse[fd].getErrorCode()) << std::endl;
		std::string error_resp = response.errorResponse(static_cast<StatusCode>(parse[fd].getErrorCode()));
		write(fd, error_resp.c_str(), error_resp.size());
		removeClient(i);
		return ;
	}
	this->sessions_manager.setUpSession(request);
	bool isLogout = (request.getPath() == "/logout" && request.getMethod() == "POST");
	std::string raw_resp = response.handleRequest(request);
	if (isLogout)
		this->sessions_manager.removeSession(request.getSession().getId());
	std::cout << " -> " << response.getStatusCode() << std::endl;
	write(fd, raw_resp.c_str(), raw_resp.size());
	if (parse[fd].getRequest().getHeaders().at("Connection") == "close")
		removeClient(i);
}

void Server::removeClient(size_t& i)
{
	close(fds[i].fd);
	connections.erase(fds[i].fd);
	fds.erase(fds.begin() + i);
	parse.erase(fds[i].fd);
	--i;
}
