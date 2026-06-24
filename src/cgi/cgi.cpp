#include "cgi.hpp"
#include "../shared/utils.hpp"
#include <fcntl.h>

using namespace CGI;

namespace {
void setPipeNonblock(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags != -1)
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
} // namespace

Cgi::Cgi(std::map<std::string, std::string> &initCgiMap, http::HttpRequest &initStructRequest)
	: cgiMap(initCgiMap), structRequest(initStructRequest) {
	inPipe[0] = -1;
	inPipe[1] = -1;
	outPipe[0] = -1;
	outPipe[1] = -1;
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
	std::vector<std::string> envp;
	std::ostringstream numberString;
	std::string scriptName;
	size_t pos = structRequest.path.rfind('/');

	scriptName = structRequest.path.substr(pos + 1);
	numberString << structRequest.body.length();
	envp.push_back("REQUEST_METHOD=" + Utils::httpMethodToString(structRequest.method));
	envp.push_back("QUERY_STRING=" + structRequest.query);
	envp.push_back("CONTENT_LENGTH=" + numberString.str());
	envp.push_back("SCRIPT_NAME=" + scriptName);
	for (std::map<std::string, std::string>::iterator it = structRequest.headers.begin();
		 it != structRequest.headers.end(); it++)
		envp.push_back(it->first + "=" + it->second);
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

pid_t Cgi::executeCGI(const std::string &root, int &outReadFd, int &inWriteFd) {
	std::string runnerScript = findRunner();
	std::string fixPath = root + structRequest.path;
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
	outReadFd = -1;
	inWriteFd = -1;
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
		alarm(CGI_TIMEOUT);
		if (execve(argv[0], argv, &envp[0]) == -1)
			_exit(2);
	} else {
		close(inPipe[0]);
		close(outPipe[1]);
		outReadFd = outPipe[0];
		inWriteFd = inPipe[1];
		inPipe[0] = -1;
		outPipe[0] = -1;
		inPipe[1] = -1;
		outPipe[1] = -1;
		setPipeNonblock(outReadFd);
		setPipeNonblock(inWriteFd);
	}
	return pid;
}

Cgi::~Cgi() { closePipe(); }
