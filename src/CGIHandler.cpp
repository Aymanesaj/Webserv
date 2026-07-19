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
