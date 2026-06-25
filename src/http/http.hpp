#pragma once

#include "../shared/logger.hpp"
#include <map>
#include <string>

#define HTTP_VER "HTTP/1.1"

extern HttpReqLogger httpReqLogger;

namespace http {

enum HttpMethod { GET, POST, DELETE, INVALID };

// TODO COMPLETE is not always 200
// bad trip here, need different logic
enum RequestStatus {
	INCOMPLETE,
	COMPLETE,
	TOO_LARGE,
	BAD_REQ,
};

enum HTTPCode {
	OK = 200,
	TEMPORARY_REDIRECT = 307,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	PAYLOAD_TOO_LARGE = 413,
	INTERNAL_SERVER_ERROR = 500,
	BAD_REQUEST = 400,
	NOT_IMPLEMENTED = 501,
	HTTP_VERSION_NOT_SUPPORTED = 505,
};

struct HttpRequest {
	RequestStatus status;
	HttpMethod method;
	std::string path;
	std::string query;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
	bool keepAlive;
};

} // namespace http
