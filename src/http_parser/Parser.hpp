#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>


enum HttpMethod {
	GET,
	POST,
	DELETE,
	INVALID
};

enum HttpStatus {
	INCOMPLETE = 0,
    COMPLETE = 200,
    BAD_REQ = 400,
	NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    PAYLOAD_TOO_LARGE = 413,
    NOT_IMPLEMENTED = 501
};

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

HttpRequest parserHttp(std::string &buf, size_t maxBodySize);

HttpRequest parseRequestLine(const std::string &line);
void parseHeaders(const std::string &headerSection, HttpRequest &request);
void parseBody(const std::string &bodySection, HttpRequest &request);

// Print function to display parsed request
void printHttpRequest(const HttpRequest &request);

