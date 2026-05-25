#include "Parser.hpp"
#include "../../shared/utils.hpp"
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

	// sets BAD_REQ if failed to parse the req line
	HttpRequest req = parseRequestLine(requestLineToParse);
	if (req.status == BAD_REQ)
		return req;

	// could fail silently, sets status to BAD_REQ, burried inside, nasty code
	parseHeaders(headersToParse, req);
	if (req.status == BAD_REQ)
		return req;

	// BODY
	if (req.headers.find("Content-Length") != req.headers.end()) {
		size_t contentLength = std::strtoul(req.headers["Content-Length"].c_str(), NULL, 10);

		if (contentLength > maxBodySize) {
			// req.status = PAYLOAD_TOO_LARGE; // moved to reponsder
			return req;
		}

		if (reqBuf.size() < headersEnd + 4 + contentLength) {
			req.status = INCOMPLETE;
			return req;
		}

		req.body = reqBuf.substr(headersEnd + 4, contentLength);
	}

	size_t bodyLength = req.body.size();
	size_t totalConsumed = headersEnd + 4 + bodyLength;

	if (totalConsumed <= reqBuf.size()) {
		reqBuf.erase(0, totalConsumed);
	}

	return req;
}

// Print function to display parsed HTTP request
void http::printHttpRequest(const HttpRequest &request) {
	std::time_t now = std::time(0);
	std::string dt = std::ctime(&now);
	if (!dt.empty() && dt[dt.length() - 1] == '\n') {
		dt.erase(dt.length() - 1);
	}

	std::cout << dt << " -> " << Utils::httpMethodToString(request.method) << " " << request.path
			  << "\n";
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
