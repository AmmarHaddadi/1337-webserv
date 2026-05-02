#include "../header/Parser.hpp"
#include "shared/logger.hpp"
#include <ostream>
#include "main.hpp"

HttpRequest parserHttp(const std::string &buf) {
	HttpRequest request;
    request.method = UNKNOWN;

	size_t bodyStart = buf.find("\r\n\r\n");
	
	if (bodyStart == std::string::npos) {
		std::cout << "Incomplete HTTP request (no body separator found)" << '\n';
		return request;
	}
	
	std::string headerSection = buf.substr(0, bodyStart);
	
	std::string bodySection = buf.substr(bodyStart + 4);
	
	size_t requestLineEnd = headerSection.find("\r\n");
	if (requestLineEnd == std::string::npos) {
		std::cout << "Invalid HTTP request (no request line found)" << '\n';
		return request;
	}
	
	std::string requestLineToParse = headerSection.substr(0, requestLineEnd);
	
	std::string headersToParse = headerSection.substr(requestLineEnd + 2);
	
	request = parseRequestLine(requestLineToParse);
	parseHeaders(headersToParse, request);
	parseBody(bodySection, request);

	return request;
}

// Print function to display parsed HTTP request
void printHttpRequest(const HttpRequest &request) {
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
		case UNKNOWN:
			oss << "UNKNOWN" << '\n';
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