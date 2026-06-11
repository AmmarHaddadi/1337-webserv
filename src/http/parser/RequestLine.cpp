#include "../../shared/utils.hpp"
#include "Parser.hpp"
#include <sstream>

using namespace http;

// TODO line must be limited in len/size
// @brief sets BAD_REQ is failed, COMPLETE if all good, method = INVALID if method is not supported
// also decodes the full URL
HttpRequest http::parseRequestLine(const std::string &line) {
	HttpRequest request;
	request.status = COMPLETE;
	request.keepAlive = false;

	std::istringstream iss(line);
	std::string methodStr;
	std::string path;
	std::string version;

	if (!(iss >> methodStr >> path >> version)) {
		request.status = BAD_REQ;
		return request;
	}

	// see if req line has +3 elements
	std::string extra;
	if (iss >> extra) { // NOLINT( readability-implicit-bool-conversion)
		request.status = BAD_REQ;
		return request;
	}

	// must have no tabs
	if (line.find('\t') != std::string::npos) {
		request.status = BAD_REQ;
		return request;
	}

	if (methodStr == "GET") {
		request.method = GET;
	} else if (methodStr == "POST") {
		request.method = POST;
	} else if (methodStr == "DELETE") {
		request.method = DELETE;
	} else {
		request.method = INVALID;
		return request;
	}

	size_t queryPos = path.find('?');
	if (queryPos != std::string::npos) {
		request.path = Utils::decodeURL(path.substr(0, queryPos));
		request.query = Utils::decodeURL(path.substr(queryPos + 1));
	} else {
		request.path = Utils::decodeURL(path);
	}

	request.version = version;

	return request;
}
