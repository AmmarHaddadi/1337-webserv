#include "Parser.hpp"
#include <cctype>
#include <string>

using namespace http;

std::string toLowerCase(std::string str) {
	for (size_t i = 0; i < str.length(); ++i) {
		str[i] = static_cast<char>(std::tolower(str[i]));
	}
	return str;
}

void http::parseHeaders(const std::string &headerSection, HttpRequest &request) {
	size_t pos = 0;
	while (pos < headerSection.size()) {
		size_t lineEnd = headerSection.find("\r\n", pos);
		if (lineEnd == std::string::npos) {
			lineEnd = headerSection.size();
		}

		std::string line = headerSection.substr(pos, lineEnd - pos);

		if (line.empty()) {
			break;
		}

		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string key = line.substr(0, colonPos);
			std::string keyLowerCase = toLowerCase(key);

			size_t valueStart = line.find_first_not_of(" \t", colonPos + 1);
			std::string value = (valueStart != std::string::npos) ? line.substr(valueStart) : "";

			request.headers[keyLowerCase] = value;
		} else {
			request.status = BAD_REQ;
			break;
		}

		pos = lineEnd + 2;
	}
}
