#pragma once

#include "../main.hpp"
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <map>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class Cgi {
  private:
	std::map<std::string, std::string> &cgiMap;
	HttpRequest &structRequest;
	int outPipe[2];
	int inPipe[2];
	static std::string getRequestMethod(HttpMethod requestMethod);
	void closePipe();
	std::vector<std::string> buildEnvp() const;
	std::string findRunner() const;

  public:
	Cgi(std::map<std::string, std::string> &initCgiMap, HttpRequest &initStructRequest);
	~Cgi();
	std::string executeCGI();
};
