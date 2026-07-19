#include "../includes/CGI.hpp"

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

std::string CGI::execute()
{
    CgiProcessInfo info = startProcess();
    if (!info.ok)
        return _response.errorResponse(INTERNAL_SERVER_ERROR);

    std::string output;
    char buffer[4096];
    ssize_t bytesRead;
    int status;
    time_t start = time(NULL);

    int flags = fcntl(info.pipeFd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(info.pipeFd, F_SETFL, flags & ~O_NONBLOCK);

    while (true)
    {
        bytesRead = read(info.pipeFd, buffer, sizeof(buffer));
        if (bytesRead > 0)
            output.append(buffer, bytesRead);

        pid_t ret = waitpid(info.pid, &status, WNOHANG);
        if (ret == info.pid)
            break;
        if (ret == -1)
        {
            close(info.pipeFd);
            return _response.errorResponse(INTERNAL_SERVER_ERROR);
        }
        if (time(NULL) - start >= 5)
        {
            kill(info.pid, SIGKILL);
            waitpid(info.pid, &status, 0);
            close(info.pipeFd);
            return _response.errorResponse(GATEWAY_TIMEOUT);
        }
        usleep(10000);
    }
    while ((bytesRead = read(info.pipeFd, buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytesRead);
    close(info.pipeFd);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return _response.errorResponse(INTERNAL_SERVER_ERROR);
    return parseOutput(output);
}

std::string HttpResponse::handleCGI(HttpRequest& request)
{
    std::string reqPath = request.getPath();
    size_t qpos = reqPath.find('?');
    if (qpos != std::string::npos)
        reqPath = reqPath.substr(0, qpos);
    LocationConfig location = ConfigParser::findLocation(reqPath, _server);
    std::string path = location.root + reqPath;
    if (!location.has_cgi_path || !location.has_cgi_extension)
        return (errorResponse(INTERNAL_SERVER_ERROR));
    if (access(location.cgi_path.c_str(), X_OK) != 0)
        return errorResponse(INTERNAL_SERVER_ERROR);
    if (!isCGI(request.getPath(), location))
        return (errorResponse(NOT_FOUND));
    std::ifstream check(path.c_str());
    if (!check.is_open())
        return (errorResponse(NOT_FOUND));
    check.close();
    CGI cgi(request, location, path, *this);
    return (cgi.execute());
}