#include "Parser.hpp"
#include <sstream>

using namespace http;

void http::parseHeaders(const std::string &headerSection, HttpRequest &request) {
	std::istringstream iss(headerSection);
	std::string line;

	while (std::getline(iss, line, '\r')) { // NOLINT
		if (line.empty()) {
			break;
		}

		if (line[line.size() - 1] == '\n') {
			line = line.substr(0, line.size() - 1);
		}

		// NOTE RFC allows empty values but the `:` is mandatory
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string key = line.substr(0, colonPos);
			size_t valueStart = line.find_first_not_of(" \t", colonPos + 1);
			std::string value = (valueStart != std::string::npos) ? line.substr(valueStart) : "";
			request.headers[key] = value;
		} else {
			request.status = BAD_REQ;
			break;
		}
	}
}
