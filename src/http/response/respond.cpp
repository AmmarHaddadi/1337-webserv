#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <fcntl.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

using namespace http;

// NOTE try to move raw response building into sub funcs and keep this highlevel
void http::respondToReq(SocketMeta &sMeta, HttpRequest &req) {
	if (req.status != COMPLETE) {
		sMeta.responseBuf =
			generateHttpResponse(INTERNAL_SERVER_ERROR, generateErrorPage(INTERNAL_SERVER_ERROR));
		return;
	}

	// TODO
	// do route/path matching and find the correspondant route/path
	// if none is found err
	// if not supported method err
	// etc...

	if (req.method == http::GET) {
		// more checks
		struct stat st;
		if (stat((sMeta.server.root + req.path).c_str(), &st) != 0) {
			sMeta.responseBuf = generateHttpResponse(404, generateErrorPage(404));
			return;
		}

		// HINT S_IFREG = file S_IFDIR = dir
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			getFile(sMeta, req);
		}
	}

	// comments to be deleted nshallah
	// check final resolved path is in allowed route (/../../abc)
	// if is file and exists
	//   if ends with cgi extension run cgi
	//   else serve file (if GET and file_serve is true)
	// else if path
	//   if default file is defined and exists serve it
	//   else if file browser is true serve the file browser
}

std::string http::generateHtmlPage(const std::string &title, const std::string &body) {
	std::ostringstream oss;
	oss << "<!DOCTYPE HTML><html><head><title>" << title << "</title></head>"
		<< "<body style=\"font-family:sans-serif; text-align:center; padding-top:50px;\">" << body
		<< "</body></html>";
	return oss.str();
}

std::string http::generateErrorPage(int code) {
	// TODO move to a global scope & init once
	std::map<int, std::string> codeMessages;
	codeMessages[BAD_REQ] = "Bad Request";
	codeMessages[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	codeMessages[NOT_IMPLEMENTED] = "Not Implemented";
	codeMessages[NOT_FOUND] = "Not Found";
	codeMessages[INTERNAL_SERVER_ERROR] = "INTERNAL_SERVER_ERROR";
	codeMessages[COMPLETE] = "OK";

	std::ostringstream oss;
	oss << code;
	std::string body = "<h1>" + oss.str() + " : " + codeMessages[code] + "</h1>";
	return generateHtmlPage(codeMessages[code], body);
}

// NOTE Content-Length is appended automatically if not found
std::string http::generateHttpResponse(int httpCode,
									   const std::map<std::string, std::string> &headers,
									   const std::string &body) {
	std::map<int, std::string> codeMessages;
	codeMessages[BAD_REQ] = "Bad Request";
	codeMessages[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	codeMessages[NOT_IMPLEMENTED] = "Not Implemented";
	codeMessages[NOT_FOUND] = "Not Found";
	codeMessages[INTERNAL_SERVER_ERROR] = "INTERNAL_SERVER_ERROR";
	codeMessages[COMPLETE] = "OK";
	std::ostringstream oss;

	// status line
	oss << HTTP_VER << " " << httpCode << " " << codeMessages[httpCode] << "\r\n";

	// headers
	std::map<std::string, std::string>::const_iterator it = headers.begin();
	for (; it != headers.end(); it++)
		oss << it->first << ": " << it->second << "\r\n";
	if (headers.find("Content-Length") == headers.end())
		oss << "Content-Length: " << body.size() << "\r\n";
	oss << "\r\n";

	// body
	oss << body;

	return oss.str();
}

std::string http::generateHttpResponse(int httpCode, const std::string &body) {
	std::map<int, std::string> codeMessages;
	codeMessages[BAD_REQ] = "Bad Request";
	codeMessages[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	codeMessages[NOT_IMPLEMENTED] = "Not Implemented";
	codeMessages[NOT_FOUND] = "Not Found";
	codeMessages[COMPLETE] = "OK";
	std::ostringstream oss;

	// status line
	oss << HTTP_VER << " " << httpCode << " " << codeMessages[httpCode] << "\r\n";

	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "\r\n";

	// body
	oss << body;

	return oss.str();
}
