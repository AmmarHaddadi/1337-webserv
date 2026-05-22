#include "Parser.hpp"
#include "../http.hpp"
#include <cstdlib>
#include <sstream>

using namespace http;

HttpRequest http::parseHttp(std::string &reqBuf, size_t maxBodySize) {
	size_t firstLineEnd = reqBuf.find("\r\n");
	if (firstLineEnd == std::string::npos || firstLineEnd == 0) {
		HttpRequest req;
		req.status = BAD_REQ;
		return req;
	}

	// start of body & end of headers if no body exists
	size_t headersEnd = reqBuf.find("\r\n\r\n");
	if (headersEnd == std::string::npos) {
		HttpRequest req;
		req.status = INCOMPLETE;
		return req;
	}

	std::string requestLineToParse = reqBuf.substr(0, firstLineEnd);
	size_t headersStart = firstLineEnd + 2;
	std::string headersToParse = reqBuf.substr(headersStart, headersEnd - headersStart);

	HttpRequest request = parseRequestLine(requestLineToParse);
	// TODO empty method isn't like an unknown one
	if (request.method == INVALID) {
		request.status = NOT_IMPLEMENTED;
		// NOTE only clear up to body end
		//  reqBuf.clear();
		// return request;
	}

	// could fail silently, sets status to BAD_REQ, burried inside, nasty code
	parseHeaders(headersToParse, request);

	// BODY
	if (request.headers.find("Content-Length") != request.headers.end()) {
		size_t contentLength = std::strtoul(request.headers["Content-Length"].c_str(), NULL, 10);

		if (contentLength > maxBodySize) {
			request.status = PAYLOAD_TOO_LARGE;
			reqBuf.clear();
			return request;
		}

		if (reqBuf.size() < headersEnd + 4 + contentLength) {
			request.status = INCOMPLETE;
			return request;
		}

		request.body = reqBuf.substr(headersEnd + 4, contentLength);
	}

	size_t headerLength = headersEnd + 4;
	size_t bodyLength = request.body.size();
	size_t totalConsumed = headerLength + bodyLength;

	if (totalConsumed <= reqBuf.size()) {
		reqBuf.erase(0, totalConsumed);
	}

	return request;
}

// Print function to display parsed HTTP request
void http::printHttpRequest(const HttpRequest &request) {
	// std::cout << "\n======== HTTP REQUEST ========" << '\n';

	// Print method
	std::ostringstream oss;

	oss << "\nMETHOD: ";
	switch (request.method) {
	case GET:
		oss << "GET" << '\n';
		break;
	case POST:
		oss << "POST" << '\n';
		break;
	case DELETE:
		oss << "DELETE" << '\n';
		break;
	case INVALID:
		oss << "INVALID" << '\n';
		break;
	}

	// Print path and query
	oss << "PATH: " << request.path << '\n';
	if (!request.query.empty()) {
		oss << "QUERY: " << request.query << '\n';
	}

	// Print version
	oss << "VERSION: " << request.version << '\n';

	// Print headers
	oss << "\nHEADERS:" << '\n';
	if (request.headers.empty()) {
		oss << "  (none)" << '\n';
	} else {
		for (std::map<std::string, std::string>::const_iterator it = request.headers.begin();
			 it != request.headers.end(); ++it) {
			oss << "  " << it->first << ": " << it->second << '\n';
		}
	}

	// Print body
	oss << "\nBODY:" << '\n';
	if (request.body.empty()) {
		oss << "  (empty)" << '\n';
	} else {
		oss << "  " << request.body << '\n';
	}

	httpReqLogger.debug(oss.str());

	// std::cout << "==============================\n" << '\n';
}
