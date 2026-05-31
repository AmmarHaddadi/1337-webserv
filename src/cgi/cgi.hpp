#pragma once

#include "../http/http.hpp"
#include "../main.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace CGI {

class Cgi {
  private:
	std::map<std::string, std::string> &cgiMap;
	http::HttpRequest &structRequest;
	int outPipe[2];
	int inPipe[2];
	void closePipe();
	std::vector<std::string> buildEnvp() const;
	std::string findRunner() const;

  public:
	Cgi(std::map<std::string, std::string> &initCgiMap, http::HttpRequest &initStructRequest);
	~Cgi();
	std::string executeCGI(const std::string &root);
};

} // namespace CGI
