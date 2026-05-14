#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <string>

enum HttpMethod { GET, POST, DELETE, INVALID };

enum HttpStatus { COMPLETE, INCOMPLETE, BAD_REQ };

struct HttpRequest {
	HttpMethod method;
	std::string path;
	std::string query;
	std::string version;
	HttpStatus status;
	std::string tmp_buf;

	std::map<std::string, std::string> headers;

	std::string body;
};

HttpRequest parserHttp(std::string &buf);

HttpRequest parseRequestLine(const std::string &line);
void parseHeaders(const std::string &headerSection, HttpRequest &request);
void parseBody(const std::string &bodySection, HttpRequest &request);

// Print function to display parsed request
void printHttpRequest(const HttpRequest &request);
