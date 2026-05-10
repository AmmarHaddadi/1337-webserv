#include "../main.hpp"

void parseHeaders(const std::string &headerSection, HttpRequest &request) {

	size_t pos = 0;
	while (pos < headerSection.size()) {
		size_t lineEnd = headerSection.find("\r\n", pos);
		if (lineEnd == std::string::npos) {
			lineEnd = headerSection.size();
		}

		std::string line = headerSection.substr(pos, lineEnd - pos);

		if (!line.empty() && line[line.size() - 1] == '\r') {
			line = line.substr(0, line.size() - 1);
		}

		if (line.empty()) {
			break;
		}

		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string key = line.substr(0, colonPos);
			size_t valueStart = colonPos + 1;
			while (valueStart < line.size() &&
				   (line[valueStart] == ' ' || line[valueStart] == '\t')) {
				++valueStart;
			}
			std::string value = (valueStart < line.size()) ? line.substr(valueStart) : "";
			request.headers[key] = value;
		}

		pos = lineEnd + 2;
	}
}