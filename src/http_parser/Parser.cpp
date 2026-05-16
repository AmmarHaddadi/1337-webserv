#include "../main.hpp"

HttpRequest parserHttp(std::string &buf, size_t maxBodySize) {

	HttpRequest request;

	size_t bodyStart = buf.find("\r\n\r\n");

	if (bodyStart == std::string::npos) {
		request.status = INCOMPLETE;
		return request;
	}

	std::string headerSection = buf.substr(0, bodyStart);

	size_t requestLineEnd = headerSection.find("\r\n");
	if (requestLineEnd == std::string::npos) {
		request.status = BAD_REQ;
		return request;
	}

	std::string requestLineToParse = headerSection.substr(0, requestLineEnd);

	std::string headersToParse = headerSection.substr(requestLineEnd + 2);

	request = parseRequestLine(requestLineToParse);

	if (request.method == INVALID) {
        request.status = NOT_IMPLEMENTED; 
        return request;
    }

	parseHeaders(headersToParse, request);

	if (request.headers.find("Content-Length") != request.headers.end()) {
		size_t contentLength = std::strtoul(request.headers["Content-Length"].c_str(), NULL, 10);

		if (contentLength > maxBodySize) {
			std::cout << "PAYLOAD_TOO_LARGE = 413";
            request.status = PAYLOAD_TOO_LARGE;
            return request;
        }

		std::string currentBody = buf.substr(bodyStart + 4);

		if (currentBody.size() < contentLength) {
			request.status = INCOMPLETE;
			return request;
		}

		parseBody(currentBody.substr(0, contentLength), request);
		request.status = COMPLETE;
	} else {
		request.status = COMPLETE;
	}

	if (request.status == COMPLETE) {
		size_t headerLength = bodyStart + 4;
		size_t bodyLength = request.body.size();

		size_t totalConsumed = headerLength + bodyLength;

		if (totalConsumed <= buf.size()) {
			buf.erase(0, totalConsumed);
		}
	}

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