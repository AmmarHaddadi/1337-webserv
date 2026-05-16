#include "Cgi.hpp"

Cgi::Cgi( std::map<std::string, std::string> &initCgiMap, HttpRequest &initStructRequest ) : cgiMap(initCgiMap), structRequest(initStructRequest){
	inPipe[0] = -1;
	inPipe[1] = -1;
	outPipe[0] = -1;
	outPipe[1] = -1;
}

std::string Cgi::getRequestMethod( HttpMethod requestMethod ) const{
	switch (requestMethod){
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		default:
			return "INVALID";
	}
}

void    Cgi::closePipe( void ){
	if (inPipe[0] > 2) close(inPipe[0]); inPipe[0] = -1;
	if (inPipe[1] > 2) close(inPipe[1]); inPipe[1] = -1;
	if (outPipe[0] > 2) close(outPipe[0]); outPipe[0] = -1;
	if (outPipe[1] > 2) close(outPipe[1]); outPipe[1] = 1;
}

std::vector<std::string>	Cgi::buildEnvp( void ){
	std::vector<std::string>	envp;
	std::ostringstream			numberString;

	numberString << structRequest.body.length();
	envp.push_back("REQUEST_METHOD=" + getRequestMethod(structRequest.method));
	envp.push_back("QUERY_STRING=" + structRequest.query);
	envp.push_back("CONTENT_LENGTH=" + numberString.str());
	envp.push_back("SCRIPT_NAME=" + structRequest.path);
	return (envp);
}

std::string		Cgi::findRunner( void ) const{
	std::string		runnerScript;
	size_t			pos = structRequest.path.rfind('.');

	if (pos == std::string::npos)
		throw std::runtime_error("Run-time Error: extension not found");
	runnerScript = cgiMap[structRequest.path.substr(pos + 1)];
	if (runnerScript.empty())
		throw std::runtime_error("Run-time Error: runner not found");
	return (runnerScript);
}

std::string Cgi::executeCGI( void ){
	std::string					responseScript;
	std::string					runnerScript = findRunner();
	char						*argv[] = {const_cast<char *>(runnerScript.c_str()), const_cast<char *>(structRequest.path.c_str()), NULL};
	std::vector<std::string>	helpBuildEnvp = buildEnvp();
	char						*envp[] = { const_cast<char *>(helpBuildEnvp[0].c_str()), const_cast<char *>(helpBuildEnvp[1].c_str()),
											const_cast<char *>(helpBuildEnvp[2].c_str()), const_cast<char *>(helpBuildEnvp[3].c_str()), NULL
	};

	if (pipe(outPipe) == -1 || pipe(inPipe) == -1) throw std::runtime_error("Run-time Error: pipe() failed");
	pid_t	pid = fork();
	if (pid == -1)
		throw std::runtime_error("Run-time Error: fork() failed");
	else if (pid == 0) {
		if (dup2(outPipe[1], STDOUT_FILENO) == -1 || dup2(inPipe[0], STDIN_FILENO) == -1){
			closePipe();
			exit(1);
		}
		closePipe();
		alarm(TCP_TIMEOUT);
		if (execve(argv[0], argv, envp) == -1)
			exit(2);
	}
	else {
		close(inPipe[0]); close(outPipe[1]);
		if (!structRequest.body.empty())
			write(inPipe[1], structRequest.body.c_str(), structRequest.body.length());
		char		buf[1024];
		ssize_t		n;
		int			status;
		while ((n = read(outPipe[0], buf, sizeof(buf))) > 0)
			responseScript.append(buf, n);
		close(outPipe[0]); close(inPipe[1]);
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)){
			if (WEXITSTATUS(status) == 1) throw std::runtime_error("Run-time Error: dup2() failed");
			if (WEXITSTATUS(status) == 2) throw std::runtime_error("Run-time Error: execve() failed");
		}
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) throw std::runtime_error("Script timeout");
	}
	return (responseScript);
}

Cgi::~Cgi( void ){
	closePipe();
}