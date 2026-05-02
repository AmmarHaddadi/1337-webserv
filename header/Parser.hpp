#ifndef PARSER_HPP
#define PARSER_HPP

#include <string>
#include <map>
#include <iostream>
#include <sstream>

using namespace std;

enum Method {
	GET,
	POST,
	DELETE,
	UNKNOWN
};

struct HttpRequest {

	Method method;
	std::string path;
	std::string query;
	std::string version;

	std::map<std::string, std::string> headers;

	std::string body;

};

HttpRequest parserHttp(const std::string &buf);

HttpRequest parseRequestLine(const std::string &line);
void parseHeaders(const std::string &headerSection, HttpRequest &request);
void parseBody(const std::string &bodySection, HttpRequest &request);

// Print function to display parsed request
void printHttpRequest(const HttpRequest &request);

#endif
