#pragma once

#include "../shared/logger.hpp"
#include <map>
#include <string>

extern HttpReqLogger httpReqLogger;

namespace http {

enum HttpMethod { GET, POST, DELETE, INVALID };

// TODO COMPLETE is not always 200
// bad trip here, need different logic
enum HttpStatus {
	INCOMPLETE = 0,
	COMPLETE = 200,
	BAD_REQ = 400,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	PAYLOAD_TOO_LARGE = 413,
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501
};

struct HttpRequest {
	HttpStatus status;
	HttpMethod method;
	std::string path;
	std::string query;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;
};

} // namespace http
