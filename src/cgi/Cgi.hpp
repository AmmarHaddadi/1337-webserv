#pragma once

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include "../main.hpp"

class   Cgi {
	private:
		std::map<std::string, std::string>	&cgiMap;
		HttpRequest							&structRequest;
		int									outPipe[2];
		int									inPipe[2];
		std::string					getRequestMethod( HttpMethod requestMethod ) const;
		void						closePipe( void );
		std::vector<std::string>	buildEnvp( void );
		std::string					findRunner( void ) const;

	public:
		Cgi( std::map<std::string, std::string> &initCgiMap, HttpRequest &initStructRequest );
		~Cgi( );
		std::string executeCGI( void );
};