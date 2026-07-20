#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include "../includes/HttpParser.hpp"
#include "../includes/CGI.hpp"

volatile sig_atomic_t Server::flag = 1;

Server::~Server()
{
	for (size_t i = 0; i < fds.size(); i++)
		close(fds[i].fd);
}

ServerConfig& Server::getServer(const std::string hostHeader, int client_fd)
{
    int listening_fd = client_to_server_socket[client_fd];
    std::vector<ServerConfig>& servers = socket_servers[listening_fd];
    std::string hostname = hostHeader;

    size_t colonPos = hostname.find(':');
    if (colonPos != std::string::npos)
        hostname = hostname.substr(0, colonPos);
    for (size_t i = 0; i < hostname.size(); ++i)
        hostname[i] = std::tolower(hostname[i]);
    for (size_t i = 0; i < servers.size(); ++i)
    {
        std::string serverName = servers[i].server_name;
        for (size_t j = 0; j < serverName.size(); ++j)
            serverName[j] = std::tolower(serverName[j]);
        if (serverName == hostname)
            return servers[i];
    }
    return servers[0];
}

size_t Server::getClientMaxBodySize(const std::string& hostHeader, int client_fd)
{
    return getServer(hostHeader, client_fd).client_max_body_size;
}

void Server::signal_handler(int)
{
	flag = 0;
}

void Server::init(const std::vector<ServerConfig>& servers)
{
	signal(SIGINT, signal_handler);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, signal_handler);
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
	while (flag){
		// short timeout to check CGI timeouts periodically
		int timeout = cgi_processes.empty() ? -1 : 500;
		int ready = poll(fds.data(), fds.size(), timeout);
		if (!ready)
		{
			checkCgiTimeouts();
			continue;
		}
		if (ready < 0)
			continue;
		for (size_t i = 0; i < fds.size(); ++i)
		{
			if (fds[i].revents & (POLLHUP | POLLERR))
			{
				if (cgi_pipe_fds.count(fds[i].fd))
				{
					handleCgiRead(i);
					continue;
				}
				if (cgi_in_to_out.count(fds[i].fd))
				{
					handleCgiWrite(i);
					continue;
				}
				if (!listening_sockets.count(fds[i].fd))
					removeClient(i);
				continue;
			}
			if (fds[i].revents & POLLOUT)
			{
				if (cgi_in_to_out.count(fds[i].fd))
					handleCgiWrite(i);
				else
					writeResponse(i);
			}
			if (fds[i].revents & POLLIN)
			{
				if (listening_sockets.count(fds[i].fd))
					acceptClient(i);
				else if (cgi_pipe_fds.count(fds[i].fd))
					handleCgiRead(i);
				else
				{
					std::map<int, ClientState>::iterator it = clients.find(fds[i].fd);
					if (it == clients.end() || it->second.outbuf.empty())
						readRequest(i);
				}
			}
			fds[i].revents = 0;
		}
		checkCgiTimeouts();
	}
}

void Server::acceptClient(size_t& i)
{
	sockaddr client_addr = {};
	socklen_t len = sizeof(client_addr);
	int client_fd = accept(fds[i].fd, &client_addr, &len);
	if (client_fd < 0)
		return ;
	client_to_server_socket[client_fd] = fds[i].fd;
	clients[client_fd] = ClientState();
	int flags = fcntl(client_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("Fcntl failed");
	pollfd p = {client_fd, POLLIN, 0};
	fds.push_back(p);
	parse[client_fd].setServerContext(this, client_fd);
}

void Server::queueResponse(size_t& i, const std::string& resp, bool closeAfter)
{
	int fd = fds[i].fd;
	clients[fd].outbuf += resp;
	clients[fd].closeAfterWrite = closeAfter;
	fds[i].events |= POLLOUT;
	writeResponse(i);
}

void Server::writeResponse(size_t& i)
{
	int fd = fds[i].fd;
	std::map<int, ClientState>::iterator it = clients.find(fd);
	if (it == clients.end() || it->second.outbuf.empty())
	{
		fds[i].events &= ~POLLOUT;
		return ;
	}
	ClientState& state = it->second;
	ssize_t n = write(fd, state.outbuf.c_str(), state.outbuf.size());
	if (n > 0)
		state.outbuf.erase(0, n);
	else if (n < 0)
		return ;
	if (!state.outbuf.empty())
		return ;
	fds[i].events &= ~POLLOUT;
	if (state.closeAfterWrite)
		removeClient(i);
	else{
		fds[i].events |= POLLIN;
		parse[fd].clearRequest();
	}
}

void Server::readRequest(size_t& i)
{
	char buffer[65536];
	int fd = fds[i].fd;
	ssize_t bytes = read(fd, buffer, sizeof(buffer));
	if (bytes == 0)
	{
		removeClient(i);
		return ;
	}
	if (bytes < 0)
		return ;
    HttpResponse response;
    HttpRequest& request = parse[fd].getRequest();
	try
	{
		parse[fd].resolveMaxBodySize();
		if (parse[fd].parseRequest(std::string(buffer, bytes)) == INCOMPLETE)
			return ;
		std::cout << request.getMethod()
            << " " << request.getPath()
            << " " << request.getVersion()
            << " host=" << request.getHeaders().at("Host");
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << " : " << static_cast<StatusCode>(parse[fd].getErrorCode()) << std::endl;
		std::string error_resp = response.errorResponse(static_cast<StatusCode>(parse[fd].getErrorCode()));
		queueResponse(i, error_resp, true);
		return ;
	}
	this->sessions_manager.setUpSession(request);
	bool isLogout = (request.getPath() == "/logout" && request.getMethod() == "POST");
	response.setServer(getServer(request.getHeaders().at("Host"), fd));
	bool closeConn = (parse[fd].getRequest().getHeaders().at("Connection") == "close");

	std::string cgiPath = request.getPath();
	size_t qpos = cgiPath.find('?');
	if (qpos != std::string::npos)
		cgiPath = cgiPath.substr(0, qpos);
	LocationConfig location = ConfigParser::findLocation(cgiPath, getServer(request.getHeaders().at("Host"), fd));

	std::string physPath = location.root + cgiPath;
	if (request.getMethod() != "DELETE" && Utils::is_Directory(physPath) && location.has_index && !location.index.empty()) {
		if (cgiPath[cgiPath.length() - 1] != '/')
			cgiPath += "/";
		cgiPath += location.index;
		std::string newReqPath = cgiPath;
		if (qpos != std::string::npos)
			newReqPath += request.getPath().substr(qpos);
		request.setPath(newReqPath);
	}

	if (response.isCGI(request.getPath(), location))
	{
		if (!location.isMethodAllowed(request.getMethod()))
		{
			std::string err = response.errorResponse(METHOD_NOT_ALLOWED);
			std::cout << " -> " << response.getStatusCode() << std::endl;
			queueResponse(i, err, closeConn);
			return ;
		}
		std::string path = location.root + cgiPath;
        std::string interpreter;
        for (std::map<std::string, std::string>::const_iterator it = location.cgi.begin(); it != location.cgi.end(); ++it) {
            if (path.size() >= it->first.size() && path.compare(path.size() - it->first.size(), it->first.size(), it->first) == 0) {
                interpreter = it->second;
                break;
            }
        }

		if (interpreter.empty() || access(interpreter.c_str(), X_OK) != 0)
		{
			std::string err = response.errorResponse(INTERNAL_SERVER_ERROR);
			std::cout << " -> " << response.getStatusCode() << std::endl;
			queueResponse(i, err, closeConn);
			return ;
		}
		std::ifstream check(path.c_str());
		if (!check.is_open())
		{
			std::string err = response.errorResponse(NOT_FOUND);
			std::cout << " -> " << response.getStatusCode() << std::endl;
			queueResponse(i, err, closeConn);
			return ;
		}
		check.close();

		CGI cgi(request, location, path, response);
		CgiProcessInfo info = cgi.startProcess();
		if (!info.ok)
		{
			std::string err = response.errorResponse(INTERNAL_SERVER_ERROR);
			std::cout << " -> " << response.getStatusCode() << std::endl;
			queueResponse(i, err, closeConn);
			return ;
		}

		// Register the CGI pipe fd in the poll loop
		CgiState state;
		state.pid = info.pid;
		state.pipeFd = info.pipeFd;
		state.clientFd = fd;
		state.startTime = time(NULL);
		state.response = response;
		state.request = &request;
		state.closeConn = closeConn;

		if (info.pipeFdIn != -1)
		{
			state.pipeFdIn = info.pipeFdIn;
			state.bodyFd = open(request.getBodyFilePath().c_str(), O_RDONLY);
			if (state.bodyFd < 0) {
				// Fallback if open fails
				close(info.pipeFdIn);
				state.pipeFdIn = -1;
			}
		}

		cgi_processes[info.pipeFd] = state;
		cgi_pipe_fds.insert(info.pipeFd);

		pollfd p = {info.pipeFd, POLLIN, 0};
		fds.push_back(p);

		if (state.pipeFdIn != -1)
		{
			cgi_in_to_out[state.pipeFdIn] = info.pipeFd;
			pollfd p_in = {state.pipeFdIn, POLLOUT, 0};
			fds.push_back(p_in);
		}

		// Stop listening for reads on the client fd while CGI runs
		fds[i].events &= ~POLLIN;

		if (isLogout)
			this->sessions_manager.removeSession(request.getSession().getId());
		return ;
	}

	std::string raw_resp = response.handleRequest(request);
	if (isLogout)
		this->sessions_manager.removeSession(request.getSession().getId());
	std::cout << " -> " << response.getStatusCode() << std::endl;
	queueResponse(i, raw_resp, closeConn);
}

void Server::removeClient(size_t& i)
{
	int fd = fds[i].fd;
	close(fd);
	fds.erase(fds.begin() + i);
	parse.erase(fd);
	client_to_server_socket.erase(fd);
	clients.erase(fd);
	--i;
}

size_t Server::findFdIndex(int fd)
{
	for (size_t i = 0; i < fds.size(); ++i)
	{
		if (fds[i].fd == fd)
			return i;
	}
	return fds.size(); // not found
}

void Server::handleCgiRead(size_t& i)
{
	int pipeFd = fds[i].fd;
	std::map<int, CgiState>::iterator it = cgi_processes.find(pipeFd);
	if (it == cgi_processes.end())
		return ;

	CgiState& state = it->second;
	char buffer[4096];
	ssize_t bytesRead = read(pipeFd, buffer, sizeof(buffer));

	if (bytesRead > 0)
	{
		state.outputBuf.append(buffer, bytesRead);
		return ;
	}

	int status = 0;
	pid_t ret = waitpid(state.pid, &status, WNOHANG);
	if (ret == 0)
	{
		kill(state.pid, SIGKILL);
		waitpid(state.pid, &status, 0);
	}
	else if (ret == -1)
		status = -1;

	finalizeCgiResponse(pipeFd, status);
}

void Server::handleCgiWrite(size_t& i)
{
	int pipeFdIn = fds[i].fd;
	std::map<int, int>::iterator it_map = cgi_in_to_out.find(pipeFdIn);
	if (it_map == cgi_in_to_out.end())
		return ;

	int pipeFd = it_map->second;
	std::map<int, CgiState>::iterator it_state = cgi_processes.find(pipeFd);
	if (it_state == cgi_processes.end())
		return ;

	CgiState& state = it_state->second;

	if (state.inputBuf.empty() && state.bodyFd != -1)
	{
		char buffer[8192];
		ssize_t bytesRead = read(state.bodyFd, buffer, sizeof(buffer));
		if (bytesRead > 0)
			state.inputBuf.append(buffer, bytesRead);
		else
		{
			// EOF or error reading temp file
			close(state.bodyFd);
			state.bodyFd = -1;
		}
	}

	if (!state.inputBuf.empty())
	{
		ssize_t bytesWritten = write(pipeFdIn, state.inputBuf.c_str(), state.inputBuf.size());
		if (bytesWritten > 0)
			state.inputBuf.erase(0, bytesWritten);
		else if (bytesWritten < 0)
			return ; // EAGAIN or error, try again later
	}

	if (state.inputBuf.empty() && state.bodyFd == -1)
	{
		// Done writing all data
		close(pipeFdIn);
		fds.erase(fds.begin() + i);
		--i;
		cgi_in_to_out.erase(pipeFdIn);
		state.pipeFdIn = -1;
	}
}

void Server::checkCgiTimeouts()
{
	time_t now = time(NULL);
	std::vector<int> timedOut;

	for (std::map<int, CgiState>::iterator it = cgi_processes.begin();
		 it != cgi_processes.end(); ++it)
	{
		if (now - it->second.startTime >= 5)
			timedOut.push_back(it->first);
	}

	for (size_t j = 0; j < timedOut.size(); ++j)
	{
		int pipeFd = timedOut[j];
		CgiState& state = cgi_processes[pipeFd];

		kill(state.pid, SIGKILL);
		int status;
		waitpid(state.pid, &status, 0);

		std::string err = state.response.errorResponse(GATEWAY_TIMEOUT);
		size_t clientIdx = findFdIndex(state.clientFd);
		if (clientIdx < fds.size())
		{
			fds[clientIdx].events |= POLLIN;
			queueResponse(clientIdx, err, state.closeConn);
		}

		size_t pipeIdx = findFdIndex(pipeFd);
		if (pipeIdx < fds.size())
		{
			close(pipeFd);
			fds.erase(fds.begin() + pipeIdx);
		}
		if (state.pipeFdIn != -1)
		{
			size_t inIdx = findFdIndex(state.pipeFdIn);
			if (inIdx < fds.size())
				fds.erase(fds.begin() + inIdx);
			close(state.pipeFdIn);
			cgi_in_to_out.erase(state.pipeFdIn);
			state.pipeFdIn = -1;
		}
		if (state.bodyFd != -1)
		{
			close(state.bodyFd);
			state.bodyFd = -1;
		}

		cgi_pipe_fds.erase(pipeFd);
		cgi_processes.erase(pipeFd);
	}
}

void Server::finalizeCgiResponse(int pipeFd, int exitStatus)
{
	std::map<int, CgiState>::iterator it = cgi_processes.find(pipeFd);
	if (it == cgi_processes.end())
		return ;

	CgiState& state = it->second;
	std::string raw_resp;

	bool processKilled = WIFSIGNALED(exitStatus);
	bool processFailed = exitStatus == -1 || (processKilled && !state.outputBuf.size());
	bool exitedWithError = WIFEXITED(exitStatus) && WEXITSTATUS(exitStatus) != 0
	                       && state.outputBuf.empty();

	if (processFailed || exitedWithError)
	{
		raw_resp = state.response.errorResponse(INTERNAL_SERVER_ERROR);
	}
	else
	{
		std::string reqPath = state.request->getPath();
		size_t qp = reqPath.find('?');
		if (qp != std::string::npos)
			reqPath = reqPath.substr(0, qp);
		LocationConfig location = ConfigParser::findLocation(
			reqPath,
			getServer(state.request->getHeaders().at("Host"), state.clientFd));
		std::string path = location.root + reqPath;
		CGI cgi(*state.request, location, path, state.response);
		raw_resp = cgi.parseOutput(state.outputBuf);
	}

	std::cout << " -> " << state.response.getStatusCode() << std::endl;
	size_t clientIdx = findFdIndex(state.clientFd);
	if (clientIdx < fds.size())
	{
		fds[clientIdx].events |= POLLIN;
		queueResponse(clientIdx, raw_resp, state.closeConn);
	}

	size_t pipeIdx = findFdIndex(pipeFd);
	if (pipeIdx < fds.size())
	{
		close(pipeFd);
		fds.erase(fds.begin() + pipeIdx);
	}

	if (state.pipeFdIn != -1)
	{
		size_t inIdx = findFdIndex(state.pipeFdIn);
		if (inIdx < fds.size())
			fds.erase(fds.begin() + inIdx);
		close(state.pipeFdIn);
		cgi_in_to_out.erase(state.pipeFdIn);
		state.pipeFdIn = -1;
	}
	if (state.bodyFd != -1)
	{
		close(state.bodyFd);
		state.bodyFd = -1;
	}

	cgi_pipe_fds.erase(pipeFd);
	cgi_processes.erase(pipeFd);
}
