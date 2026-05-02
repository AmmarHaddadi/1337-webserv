#include "../main.hpp"

void parseHeaders(const std::string &headerSection, HttpRequest &request) {
	
	size_t pos = 0;
	while (pos < headerSection.size()) {
		size_t lineEnd = headerSection.find("\r\n", pos);
		if (lineEnd == std::string::npos) {
			lineEnd = headerSection.size();
		}
		
		std::string line = headerSection.substr(pos, lineEnd - pos);
		
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		
		if (line.empty()) {
			break;
		}
		
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos) {
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 2); 
			request.headers[key] = value;
		}
		
		pos = lineEnd + 2;
	}
}