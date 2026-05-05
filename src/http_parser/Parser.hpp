#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>


enum GetMethod {
	GET,
	POST,
	DELETE,
	FALSE,
	UNKNOWN
};

enum GetStatus {
	COMPLETE,
	INCOMPLETE,
	BAD_REQ
};

struct HttpRequest {

	GetMethod method;
	std::string path;
	std::string query;
	std::string version;
	GetStatus status;
	

	std::map<std::string, std::string> headers;

	std::string body;

};

HttpRequest parserHttp(std::string &buf);

HttpRequest parseRequestLine(const std::string &line);
void parseHeaders(const std::string &headerSection, HttpRequest &request);
void parseBody(const std::string &bodySection, HttpRequest &request);

// Print function to display parsed request
void printHttpRequest(const HttpRequest &request);

