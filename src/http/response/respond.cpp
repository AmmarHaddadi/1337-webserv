#include "../../cfg/cfg.hpp"
#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace http;

// NOTE try to move raw response building into sub funcs and keep this highlevel
void http::respondToReq(SocketMeta &sMeta, HttpRequest &req) {
	if (req.version != HTTP_VER) {
		sMeta.responseBuf =
			generateHttpResponse(METHOD_NOT_ALLOWED, generateErrorPage(METHOD_NOT_ALLOWED));
		return;
	}

	if (req.method == INVALID) {
		sMeta.responseBuf =
			generateHttpResponse(METHOD_NOT_ALLOWED, generateErrorPage(METHOD_NOT_ALLOWED));
		return;
	}

	if (req.method == POST && req.headers.find("Content-Length") != req.headers.end()) {
		size_t contentLength = std::strtoul(req.headers["Content-Length"].c_str(), NULL, 10);
		if (contentLength > sMeta.server.maxBodySize) {
			sMeta.responseBuf =
				generateHttpResponse(PAYLOAD_TOO_LARGE, generateErrorPage(PAYLOAD_TOO_LARGE));
			return;
		}
	}

	// finding correspondent route config
	Config::ServerConfig::RouteConfig *rc = NULL;
	for (unsigned i = 0; i < sMeta.server.routes.size(); i++) {
		if (req.path.find(sMeta.server.routes[i].path) == 0) {
			if (rc == NULL || sMeta.server.routes[i].path.length() > rc->path.length())
				rc = &sMeta.server.routes[i];
		}
	}

	if (rc == NULL) {
		sMeta.responseBuf = generateHttpResponse(NOT_FOUND, generateErrorPage(NOT_FOUND));
		return;
	}

	if (std::find(rc->allowedMethods.begin(), rc->allowedMethods.end(),
				  Utils::httpMethodToString(req.method)) == rc->allowedMethods.end()) {
		sMeta.responseBuf =
			generateHttpResponse(METHOD_NOT_ALLOWED, generateErrorPage(METHOD_NOT_ALLOWED));
		return;
	}

	if (req.method == http::GET) {
		// more checks
		struct stat st;
		if (stat((sMeta.server.root + req.path).c_str(), &st) != 0) {
			sMeta.responseBuf = generateHttpResponse(NOT_FOUND, generateErrorPage(NOT_FOUND));
			return;
		}

		// HINT S_IFREG = file S_IFDIR = dir
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			getFile(sMeta, req);
			return;
		}

		sMeta.responseBuf = generateHttpResponse(NOT_FOUND, generateErrorPage(NOT_FOUND));
		return;
	}

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

std::string http::generateErrorPage(HTTPCode code) {
	// TODO move to a global scope & init once
	std::map<HTTPCode, std::string> codeMessages;
	codeMessages[OK] = "OK";
	codeMessages[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
	codeMessages[METHOD_NOT_ALLOWED] = "Method Not Allowed";
	codeMessages[BAD_REQUEST] = "Bad Request";
	codeMessages[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	codeMessages[NOT_IMPLEMENTED] = "Not Implemented";
	codeMessages[NOT_FOUND] = "Not Found";
	codeMessages[INTERNAL_SERVER_ERROR] = "INTERNAL_SERVER_ERROR";

	std::ostringstream oss;
	oss << code;
	std::string body = "<h1>" + oss.str() + " : " + codeMessages[code] + "</h1>";
	return generateHtmlPage(codeMessages[code], body);
}

// NOTE Content-Length is appended automatically if not found
std::string http::generateHttpResponse(HTTPCode code,
									   const std::map<std::string, std::string> &headers,
									   const std::string &body) {
	std::map<HTTPCode, std::string> codeMessages;
	codeMessages[OK] = "OK";
	codeMessages[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
	codeMessages[METHOD_NOT_ALLOWED] = "Method Not Allowed";
	codeMessages[BAD_REQUEST] = "Bad Request";
	codeMessages[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	codeMessages[NOT_IMPLEMENTED] = "Not Implemented";
	codeMessages[NOT_FOUND] = "Not Found";
	codeMessages[INTERNAL_SERVER_ERROR] = "INTERNAL_SERVER_ERROR";
	std::ostringstream oss;

	// status line
	oss << HTTP_VER << " " << code << " " << codeMessages[code] << "\r\n";

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

std::string http::generateHttpResponse(HTTPCode code, const std::string &body) {
	std::map<int, std::string> codeMessages;
	codeMessages[BAD_REQ] = "Bad Request";
	codeMessages[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	codeMessages[NOT_IMPLEMENTED] = "Not Implemented";
	codeMessages[NOT_FOUND] = "Not Found";
	codeMessages[COMPLETE] = "OK";
	std::ostringstream oss;

	// status line
	oss << HTTP_VER << " " << code << " " << codeMessages[code] << "\r\n";

	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "\r\n";

	// body
	oss << body;

	return oss.str();
}
