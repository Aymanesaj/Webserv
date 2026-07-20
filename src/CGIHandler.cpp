#include "../includes/Server.hpp"
#include "../includes/Config.hpp"
#include "../includes/HttpParser.hpp"
#include "../includes/CGI.hpp"


void Server::handleCgiRequest(size_t& i, HttpRequest& request, HttpResponse& response,
                              LocationConfig& location, const std::string& cgiPath,
                              int fd, bool closeConn, bool isLogout)
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

CgiProcessInfo CGI::startProcess()
{
    CgiProcessInfo info;
    int pi_in[2], pi_out[2];
    char *args[3];
    std::string dir;

    buildEnv();
    buildEnvArray();

    if (pipe(pi_in) == -1)
        return info;
    if (pipe(pi_out) == -1)
    {
        close(pi_in[0]);
        close(pi_in[1]);
        return info;
    }
    pid_t pid = fork();
    if (pid < 0)
    {
        close(pi_in[0]);
        close(pi_in[1]);
        close(pi_out[0]);
        close(pi_out[1]);
        return info;
    }
    if (pid == 0)
    {
        close(pi_in[1]);
        close(pi_out[0]);
        dup2(pi_in[0], STDIN_FILENO);
        close(pi_in[0]);
        dup2(pi_out[1], STDOUT_FILENO);
        close(pi_out[1]);
        int devNull = open("/dev/null", O_WRONLY);
        if (devNull != -1) {
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }
        dir = _script_path.substr(0, _script_path.find_last_of('/'));
        std::string script_name = _script_path.substr(_script_path.find_last_of('/') + 1);
        chdir(dir.c_str());
        args[0] = const_cast<char*>(_interpreter.c_str());
        args[1] = const_cast<char*>(script_name.c_str());
        args[2] = NULL;
        execve(args[0], args, &_envPointers[0]);
        exit(1);
    }
    close(pi_in[0]);
    close(pi_out[1]);

    if (_request.getMethod() == "POST")
    {
        int flags_in = fcntl(pi_in[1], F_GETFL, 0);
        if (flags_in >= 0)
            fcntl(pi_in[1], F_SETFL, flags_in | O_NONBLOCK);
        info.pipeFdIn = pi_in[1];
    }
    else
    {
        close(pi_in[1]);
        info.pipeFdIn = -1;
    }

    int flags = fcntl(pi_out[0], F_GETFL, 0);
    if (flags >= 0)
        fcntl(pi_out[0], F_SETFL, flags | O_NONBLOCK);

    info.pid = pid;
    info.pipeFd = pi_out[0];
    info.ok = true;
    return info;
}
