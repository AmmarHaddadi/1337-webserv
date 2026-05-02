#include "../main.hpp"


HttpRequest parseRequestLine(const std::string &line) {
	HttpRequest request;
	request.method = UNKNOWN;
	
	std::istringstream iss(line);
	std::string methodStr;
	std::string path;
	std::string version;
	
	iss >> methodStr >> path >> version;
	
	if (methodStr == "GET") {
		request.method = GET;
	} else if (methodStr == "POST") {
		request.method = POST;
	} else if (methodStr == "DELETE") {
		request.method = DELETE;
	}
	
	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		request.path = path.substr(0, queryPos);
		request.query = path.substr(queryPos + 1);
	} else {
		request.path = path;
	}
	
	request.version = version;
	
	return request;
}
