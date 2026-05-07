#include "main.hpp"
#include <algorithm>
#include <cctype>
#include <string>

CoreLogger coreLogger("Core", CoreLogger::DEBUG); // NOLINT
CfgLogger cfgLogger("CFG", CfgLogger::DEBUG);	  // NOLINT

using namespace Utils;

std::string Utils::fakeHttpRes() {
	std::string html = "<!DOCTYPE HTML>\n"
					   "<html lang=\"en\">\n"
					   "<head>\n"
					   "<meta charset=\"utf-8\">\n"
					   "<style type=\"text/css\">:root { color-scheme: light dark; }</style>\n"
					   "<title>fake html page</title>\n"
					   "</head>\n"
					   "<body>\n"
					   "<h1>Big title</h1>\n"
					   "<p>small text here lorem ipsum sadfjhsadf</p>"
					   "</body>\n"
					   "</html>";

	std::ostringstream oss;
	// WARN end of temporary code
	oss << "HTTP/1.1 200 OK\r\n"
		<< "Server: webserv/1.0\r\n"
		<< "Content-Type: text/html; charset=utf-8\r\n"
		<< "Content-Length: " << html.length()
		<< "\r\n"
		// << "Connection: close\r\n"
		<< "\r\n" // The critical empty line
		<< html;

	return oss.str();
}

std::vector<std::string> Utils::split(const std::string &s, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(s);
	std::string token;

	while (std::getline(iss, token, delimiter)) { // NOLINT
		result.push_back(token);
	}
	return result;
}

bool Utils::isAllNum(std::string &s) {
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); i++)
		if (std::isdigit(static_cast<unsigned char>(s[i])) == 0)
			return false;
	return true;
}
