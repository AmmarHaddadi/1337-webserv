#include "http_parser/Parser.hpp"
#include "main.hpp"
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

CoreLogger coreLogger("Core", CoreLogger::DEBUG);			   // NOLINT
CfgLogger cfgLogger("CFG", CfgLogger::DEBUG);				   // NOLINT
HttpReqLogger httpReqLogger("Http Req", HttpReqLogger::DEBUG); // NOLINT

using namespace Utils;

std::string Utils::generateErrorPage(int status) {
	std::string body;
	std::string msg;
	std::string statusLine;

	switch (status) {
	case BAD_REQ:
		msg = "Bad REQ";
		statusLine = "HTTP/1.1 400 \r\n";
		break;
	case PAYLOAD_TOO_LARGE:
		msg = "Payload Too Large";
		statusLine = "HTTP/1.1 413 \r\n";
		break;
	default:
		msg = "Not Implemented";
		statusLine = "HTTP/1.1 501 \r\n";
	}

	body = Utils::generateHtmlErrorPage(status, msg);

	std::ostringstream response;
	response << statusLine << "Content-Type: text/html\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "Server: webserv/1.0\r\n"
			 << "\r\n"
			 << body;

	return response.str();
}

std::string Utils::responseFile(HttpRequest &req) {
	std::ifstream file(("/tmp" + req.path).c_str());
	std::string body;
	std::string statusLine;

	if (!file.is_open()) {
		statusLine = "HTTP/1.1 404 Not Found\r\n";
		body = Utils::generateHtmlErrorPage(404, "Not Found :" + req.path);
		req.status = NOT_FOUND;
	} else {
		std::stringstream buffer;
		buffer << file.rdbuf();
		body = buffer.str();
		statusLine = "HTTP/1.1 200 OK\r\n";
		req.status = COMPLETE;
	}

	std::ostringstream response;
	response << statusLine << "Content-Type: text/html\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "\r\n"
			 << body;

	return response.str();
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

std::string Utils::generateHtmlErrorPage(int code, const std::string &msg) {
	std::ostringstream oss;
	oss << "<!DOCTYPE HTML>\n"
		<< "<html>\n"
		<< "<head><title>" << code << " : " << msg << "</title></head>\n"
		<< "<body style=\"font-family:sans-serif; text-align:center; padding-top:50px;\">\n"
		<< "<h1>" << code << " : " << msg << "</h1>\n"
		<< "</body>\n"
		<< "</html>";
	return oss.str();
}

