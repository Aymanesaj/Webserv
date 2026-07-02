#include "../includes/CGI.hpp"

std::string CGI::execute()
{
    int         pi_in[2], pi_out[2], status;
    std::string dir, output;
    pid_t       pid;
    char**      envp;
    char*       args[3];
    char        buffer[4096];
    ssize_t     bytesRead;

    buildEnv();
    envp = getEnvArray();

    if (pipe(pi_in) == -1 || pipe(pi_out) == -1)
    {
        freeEnvArray(envp);
        return errorResponse(INTERNAL_SERVER_ERROR);
    }

    pid = fork();

    if (pid < 0)
    {
        freeEnvArray(envp);
        return errorResponse(INTERNAL_SERVER_ERROR);
    }

    if (pid == 0)
    {
        // Child
        close(pi_in[1]);
        close(pi_out[0]);

        dup2(pi_in[0], STDIN_FILENO);
        dup2(pi_out[1], STDOUT_FILENO);

        close(pi_in[0]);
        close(pi_out[1]);

        dir = _script_path.substr(0, _script_path.find_last_of('/'));
        chdir(dir.c_str());

        args[0] = (char *)_interpreter.c_str();
        args[1] = (char *)_script_path.c_str();
        args[2] = NULL;

        execve(args[0], args, envp);

        freeEnvArray(envp);
        exit(1);
    }

    // Parent
    close(pi_in[0]);
    close(pi_out[1]);

    // Make stdout pipe non-blocking
    fcntl(pi_out[0], F_SETFL, O_NONBLOCK);

    // Send POST body to CGI
    if (_request.getMethod() == "POST")
    {
        char bodyBuffer[4096];
        ssize_t n;
        int bodyFd = _request.getBody();

        while ((n = read(bodyFd, bodyBuffer, sizeof(bodyBuffer))) > 0)
            write(pi_in[1], bodyBuffer, n);
    }

    // Tell CGI stdin is finished
    close(pi_in[1]);

    time_t start = time(NULL);

    while (true)
    {
        bytesRead = read(pi_out[0], buffer, sizeof(buffer));

        if (bytesRead > 0)
            output.append(buffer, bytesRead);

        pid_t ret = waitpid(pid, &status, WNOHANG);

        if (ret == pid)
            break;

        if (ret == -1)
        {
            close(pi_out[0]);
            freeEnvArray(envp);
            return errorResponse(INTERNAL_SERVER_ERROR);
        }

        if (time(NULL) - start >= 5)
        {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);

            close(pi_out[0]);
            freeEnvArray(envp);

            return errorResponse(GATEWAY_TIMEOUT);
        }

        usleep(10000); // 10 ms
    }

    // Read remaining output after child exits
    while ((bytesRead = read(pi_out[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytesRead);

    close(pi_out[0]);

    freeEnvArray(envp);

    if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        return errorResponse(INTERNAL_SERVER_ERROR);

    return parseOutput(output);
}
std::string HttpResponse::handleCGI(HttpRequest& request)
{
    LocationConfig location = ConfigParser::findLocation(
        request.getPath(), _server);
    std::string path = location.root + request.getPath();

    if (!location.has_cgi_path)
        return (errorResponse(NOT_FOUND));
    if (!location.has_cgi_extension)
        return (errorResponse(FORBIDDEN));
    if (access(location.cgi_path.c_str(), X_OK) != 0)
        return errorResponse(INTERNAL_SERVER_ERROR);
    std::ifstream check(path.c_str());
    if (!check.is_open())
        return (errorResponse(NOT_FOUND));
    check.close();
    if (access(path.c_str(), F_OK) != 0)
        return errorResponse(NOT_FOUND);
    CGI cgi(request, location, path);
    return (cgi.execute());
}