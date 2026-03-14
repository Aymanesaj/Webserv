#include "Server.hpp"
#include "Config.hpp"

void	server_init(std::vector<ServerConfig> servers){
	std::vector<int>	listening_sockets;
	
	std::vector<pollfd> fds;
	for (size_t i = 0; i < servers.size(); ++i)
	{
		// fd leaks handled inside the catch block later
		int fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
			throw std::runtime_error("Socket failed");
		sockaddr_in serverAddress = {0};
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_port = htons(servers[i].listen_port);
		serverAddress.sin_addr.s_addr = inet_addr((servers[i].host).c_str());  // changing inet_addr later
		if (serverAddress.sin_addr.s_addr == INADDR_NONE)
			throw std::runtime_error("Host address failed");
		int opt = 1;
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    		throw std::runtime_error("setsockopt failed");
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
			throw std::runtime_error("Fcntl failed");
		if (bind(fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // also grouping later after understanding the whole concept
			throw std::runtime_error("Bind failed");
		if (listen(fd, SOMAXCONN) < 0)
			throw std::runtime_error("Listen failed");
		listening_sockets.push_back(fd);
		pollfd pollfds = {0};
		pollfds.fd = fd;
		pollfds.events = POLLIN;
		fds.push_back(pollfds);
	}
	// std::map<int, Connection> connection;
	while (true){
		int ready = poll(fds.data(), fds.size(), -1);
		if (!ready)
			continue;
		if (ready < 0)
			throw std::runtime_error("Poll failed");
		for (size_t i = 0; i < fds.size(); ++i)
		{
			sockaddr client_addr;
			socklen_t len;
			if (fds[i].revents & POLLIN){
				if (std::find(listening_sockets.begin(), listening_sockets.end(), fds[i].fd) !=  listening_sockets.end())
				{
					int client_fd = accept(fds[i].fd, &client_addr, &len);
					if (client_fd < 0)
						continue ;
					pollfd p;
					p.fd = client_fd;
					p.events = POLLIN;
					fds.push_back(p);
					continue ;
				}
				// here read and parse request and write response
			}
		}
		
	}
}