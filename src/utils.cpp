#include "main.hpp"
#include <cctype>
#include <string>

CoreLogger coreLogger("Core", CoreLogger::DEBUG);			   // NOLINT
CfgLogger cfgLogger("CFG", CfgLogger::DEBUG);				   // NOLINT
HttpReqLogger httpReqLogger("Http Req", HttpReqLogger::DEBUG); // NOLINT

using namespace Utils;

std::string Utils::fakeHttpRes(std::vector<Config::ServerConfig> &servers, HttpRequest &req) {
	try {
		Cgi	cgi(servers[0].routes[0].cgi, req);
		std::string html = cgi.executeCGI();

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
	} catch (const std::exception &e) {
		std::ostringstream oss;
		oss << "HTTP/1.1 500 Internal Server Error\r\n"
			<< "Content-Type: text/html;\r\n"
			<< "Content-Length: " << req.body.length()
			<< "\r\n"
			<< "<!doctype html>\n"
			<< "<html lang=\"en\">\n"
			<< "<head>\n"
			<< "  <title>500 Internal Server Error</title>\n"
			<< "</head>\n"
			<< "<body>\n"
			<< "  <h1>Internal Server Error</h1>\n"
			<< "  <p>The server failed to process the request.</p>\n"
			<< "</body>\n"
			<< "</html>";
		return oss.str();
	}
}

std::vector<std::string> Utils::split(const std::string &s, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(s);
	std::string token;

	while (std::getline(iss, token, delimiter)) // NOLINT
		result.push_back(token);

	return result;
}

bool Utils::isAllNum(const std::string &s) {
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); i++)
		if (std::isdigit(static_cast<unsigned char>(s[i])) == 0)
			return false;
	return true;
}
