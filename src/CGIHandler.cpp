#include "../includes/CGI.hpp"

std::string CGI::execute()
{
	int pi1[2], pi2[2], pip;
	pid_t pid;
	char **envp;
	buildEnv();

	if (pipe(pi1) == -1 || pipe(pi2) == -1)
	{
		freeEnvArray(envp);
		return (INTERNAL_SERVER_ERROR);
	}
	pid = fork();
	if (pid == 0)
	{
		
	}

}
std::string HttpResponse::handleCGI(HttpRequest& request)
{
	LocationConfig location = ConfigParser::findLocation(request.getPath(), _server);
	
	if (!location.has_cgi_path)
		return errorResponse(NOT_FOUND);
	if (!location.has_cgi_extension)
		return errorResponse(FORBIDDEN);
	
}