#pragma once

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include <csignal>
#include <exception>
#include "../main.hpp"

class   Cgi {
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
