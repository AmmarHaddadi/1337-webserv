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

std::string Utils::responseFile(SocketMeta &sMeta, HttpRequest &req) {
	std::ifstream file((sMeta.server.root + req.path).c_str());
	std::string body;

	if (req.status == BAD_REQ || req.status == PAYLOAD_TOO_LARGE || req.status == NOT_IMPLEMENTED)
		body = Utils::generateHtmlErrorPage(req.status);
	else if (!file.is_open())
		body = Utils::generateHtmlErrorPage(NOT_FOUND);
	else {
		std::stringstream buffer;
		buffer << file.rdbuf();
		body = buffer.str();
		req.status = COMPLETE;
	}

	std::ostringstream response;
	response << "HTTP/1.1 " << req.status << " \r\n"
			 << "Content-Type: text/html\r\n"
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

std::string Utils::generateHtmlErrorPage(int code) {
	std::string msg;

	switch (code) {
	case BAD_REQ:
		msg = "Bad REQ";
		break;
	case PAYLOAD_TOO_LARGE:
		msg = "Payload Too Large";
		break;
	case NOT_IMPLEMENTED:
		msg = "Not Implemented";
		break;
	case NOT_FOUND:
		msg = "Not Found";
		break;
	default:
		msg = "ok";
	}

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
