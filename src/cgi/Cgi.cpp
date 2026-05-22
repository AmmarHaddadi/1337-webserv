#include "Cgi.hpp"

Cgi::Cgi(std::map<std::string, std::string> &initCgiMap, HttpRequest &initStructRequest)
	: cgiMap(initCgiMap), structRequest(initStructRequest) {
	inPipe[0] = -1;
	inPipe[1] = -1;
	outPipe[0] = -1;
	outPipe[1] = -1;
}

std::string Cgi::getRequestMethod(HttpMethod requestMethod) {
	switch (requestMethod) {
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

void Cgi::closePipe() {
	if (inPipe[0] > 2) {
		close(inPipe[0]);
		inPipe[0] = -1;
	}
	if (inPipe[1] > 2) {
		close(inPipe[1]);
		inPipe[1] = -1;
	}
	if (outPipe[0] > 2) {
		close(outPipe[0]);
		outPipe[0] = -1;
	}
	if (outPipe[1] > 2) {
		close(outPipe[1]);
		outPipe[1] = -1;
	}
}

std::vector<std::string> Cgi::buildEnvp() const {
	std::map<std::string, std::string>::iterator it;
	std::vector<std::string> envp;
	std::ostringstream numberString;
	std::string scriptName;
	size_t pos = structRequest.path.rfind('/');

	scriptName = structRequest.path.substr(pos + 1);
	numberString << structRequest.body.length();
	envp.push_back("REQUEST_METHOD=" + getRequestMethod(structRequest.method));
	envp.push_back("QUERY_STRING=" + structRequest.query);
	envp.push_back("CONTENT_LENGTH=" + numberString.str());
	envp.push_back("SCRIPT_NAME=" + scriptName);
	it = structRequest.headers.find("Accept");
	if (it != structRequest.headers.end() && !it->second.empty())
		envp.push_back("HTTP_ACCEPT=" + it->second);
	it = structRequest.headers.find("Content-Type");
	if (it != structRequest.headers.end() && !it->second.empty())
		envp.push_back("CONTENT_TYPE=" + it->second);
	it = structRequest.headers.find("Host");
	if (it != structRequest.headers.end() && !it->second.empty()) {
		std::string host = it->second;
		size_t posOfPort = host.find(':');
		std::string port = host.substr(posOfPort + 1);
		envp.push_back("SERVER_PORT=" + port);
		envp.push_back("HTTP_HOST=" + host.substr(0, posOfPort));
	}
	it = structRequest.headers.find("User-Agent");
	if (it != structRequest.headers.end() && !it->second.empty())
		envp.push_back("HTTP_USER_AGENT=" + it->second);
	it = structRequest.headers.find("custhdr");
	if (it != structRequest.headers.end() && !it->second.empty())
		envp.push_back("HTTP_CUSTHDR=" + it->second);
	return (envp);
}

std::string Cgi::findRunner() const {
	std::string runnerScript;
	size_t pos = structRequest.path.rfind('.');
	std::map<std::string, std::string>::const_iterator it;

	if (pos == std::string::npos)
		throw std::runtime_error("Run-time Error: extension not found");
	it = cgiMap.find(structRequest.path.substr(pos + 1));
	if (it == cgiMap.end() || it->second.empty())
		throw std::runtime_error("Run-time Error: runner not found");
	runnerScript = it->second;
	return (runnerScript);
}

std::string Cgi::executeCGI() {
	std::string responseScript;
	std::string runnerScript = findRunner();
	std::string fixPath = "." + structRequest.path;
	std::vector<std::string> helpBuildEnvp = buildEnvp();
	char *argv[] = {
		const_cast<char *>(runnerScript.c_str()),
		const_cast<char *>(fixPath.c_str()),
		NULL,
	};
	std::vector<char *> envp;
	envp.reserve(helpBuildEnvp.size() + 1);
	for (size_t i = 0; i < helpBuildEnvp.size(); i++)
		envp.push_back(const_cast<char *>(helpBuildEnvp[i].c_str()));
	envp.push_back(NULL);
	if (pipe(outPipe) == -1 || pipe(inPipe) == -1) {
		closePipe();
		throw std::runtime_error("Run-time Error: pipe() failed");
	}
	pid_t pid = fork();
	if (pid == -1) {
		closePipe();
		throw std::runtime_error("Run-time Error: fork() failed");
	}
	if (pid == 0) {
		if (dup2(outPipe[1], STDOUT_FILENO) == -1 || dup2(inPipe[0], STDIN_FILENO) == -1) {
			closePipe();
			_exit(1);
		}
		closePipe();
		alarm(TCP_TIMEOUT);
		if (execve(argv[0], argv, &envp[0]) == -1)
			_exit(2);
	} else {
		close(inPipe[0]);
		close(outPipe[1]);
		if (!structRequest.body.empty())
			write(inPipe[1], structRequest.body.c_str(), structRequest.body.length());
		close(inPipe[1]);
		inPipe[1] = -1;
		char buf[1024];
		ssize_t bytesRead;
		int status;
		while ((bytesRead = read(outPipe[0], buf, sizeof(buf))) > 0)
			responseScript.append(buf, bytesRead);
		close(outPipe[0]);
		outPipe[0] = -1;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 1)
				throw std::runtime_error("Run-time Error: dup2() failed");
			if (WEXITSTATUS(status) == 2)
				throw std::runtime_error("Run-time Error: execve() failed");
		}
		if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM)
			throw std::runtime_error("Script timeout");
	}
	return (responseScript);
}

Cgi::~Cgi() { closePipe(); }
