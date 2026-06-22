#include "Server.hpp"

std::string	execCGI(char **argv, char *root)
{
	int pipe_in[2];
	int pipe_out[2];
	pid_t pid;

	if (pipe(pipe_in) == -1)
		throw std::runtime_error("pipe error.");
	if (pipe(pipe_out) == -1)
		throw std::runtime_error("pipe error.");
	pid = fork();
	if (pid == -1)
		throw std::runtime_error("fork error.");
	if (pid == 0)
	{
		chdir(root);
		dup2(pipe_in[0], STDIN_FILENO);
		dup2(pipe_out[1], STDOUT_FILENO);
		close(pipe_in[0]);
		close(pipe_in[1]);
		close(pipe_out[0]);
		close(pipe_out[1]);
		execve(argv[0], argv, NULL);
		_exit(1);
	}
	close(pipe_in[0]);
	close(pipe_out[1]);
	close(pipe_in[1]);
	std::string result;
	char buffer[1024];
	ssize_t bytes;
	while ((bytes = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
		result.append(buffer, bytes);
	close(pipe_out[0]);
	waitpid(pid, NULL, 0);
	return result;
}