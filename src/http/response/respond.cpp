#include "../../cfg/cfg.hpp"
#include "../../cgi/cgi.hpp"
#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace http;

bool http::isCgi(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req) {
	size_t pos = req.path.rfind('.');
	if (pos != std::string::npos) {
		std::string ext = req.path.substr(pos + 1);
		std::map<std::string, std::string>::iterator it = rc.cgi.find(ext);
		if (it == rc.cgi.end())
			return false;
		CGI::Cgi cgi(rc.cgi, req);
		try {
			std::string resCgi = cgi.executeCGI(sMeta.server.root);
			sMeta.responseBuf = generateHttpResponse(OK, resCgi);
		} catch (const std::exception &e) {
			sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR,
													 generateErrorPage(INTERNAL_SERVER_ERROR));
		}
		return true;
	}
	return false;
}

// NOTE try to move raw response building into sub funcs and keep this highlevel
void http::respondToReq(SocketMeta &sMeta, const HttpRequest &req) {
	if (req.version != HTTP_VER) {
		sMeta.responseBuf = generateHttpResponse(HTTP_VERSION_NOT_SUPPORTED, req.keepAlive,
												 generateErrorPage(HTTP_VERSION_NOT_SUPPORTED));
		return;
	}
	if (req.method == INVALID) {
		sMeta.responseBuf = generateHttpResponse(METHOD_NOT_ALLOWED, req.keepAlive,
												 generateErrorPage(METHOD_NOT_ALLOWED));
		return;
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
		sMeta.responseBuf =
			generateHttpResponse(NOT_FOUND, req.keepAlive, generateErrorPage(NOT_FOUND));
		return;
	}

	if (std::find(rc->allowedMethods.begin(), rc->allowedMethods.end(),
				  Utils::httpMethodToString(req.method)) == rc->allowedMethods.end()) {
		sMeta.responseBuf = generateHttpResponse(METHOD_NOT_ALLOWED, req.keepAlive,
												 generateErrorPage(METHOD_NOT_ALLOWED));
		return;
	}

	if (isCgi(*rc, sMeta, req))
		return;

	struct stat st;
	bool exist = true;
	if (stat((sMeta.server.root + req.path).c_str(), &st) != 0) {
		exist = false;
		if (req.method != http::POST) {
			sMeta.responseBuf = generateHttpResponse(NOT_FOUND, req.keepAlive, generateErrorPage(NOT_FOUND));
			return;
		}
	}

	if (req.method == http::GET && rc->fileServer) {
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			getFile(sMeta, req);
			return;
		}
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			if (!rc->default_file.empty()) {
				try {
					if (defaultFile(*rc, sMeta, req))
						return;
				} catch (const std::exception &e) {
					sMeta.responseBuf = generateHttpResponse(
						INTERNAL_SERVER_ERROR, req.keepAlive, generateErrorPage(INTERNAL_SERVER_ERROR));
					return;
				}
			}
			if (rc->fileBrowser) {
				try {
					directoryFiles(sMeta, req);
					return;
				} catch (const std::exception &e) {
					sMeta.responseBuf = generateHttpResponse(
						INTERNAL_SERVER_ERROR, req.keepAlive, generateErrorPage(INTERNAL_SERVER_ERROR));
					return;
				}
			}
		}
	} else if (req.method == http::POST) {
		uploadFile(*rc, sMeta, req, st, exist);
		return;
	} else if (req.method == http::DELETE) {
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			if (unlink((sMeta.server.root + req.path).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive, generateErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
		}
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			if (rmdir((sMeta.server.root + req.path).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,  generateErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
		}
		sMeta.responseBuf = generateHttpResponse(OK, req.keepAlive, sMeta.server.root + req.path + ": is removed");
		return;
	}
	sMeta.responseBuf = generateHttpResponse(NOT_FOUND, req.keepAlive, generateErrorPage(NOT_FOUND));
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

std::string http::generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body) {
	std::map<std::string, std::string> hdr;
	return generateHttpResponse(code, keepAlive, body, hdr);
}

// NOTE Content-Length is appended automatically if not found
// automatically picks up the body from the explicit response payload
std::string http::generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body,
									   std::map<std::string, std::string> headers) {
	headers["connection"] = keepAlive ? "keep-alive" : "close";
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

std::string http::generateHttpResponseDirectory(const std::string &path, std::vector<std::string> &files) {
	std::ostringstream oss;

	for (size_t i = 0; i < files.size(); i++) {
		oss << "<p><a href=\"";
		oss << path + files[i];
		oss << "\">";
		oss << files[i];
		oss << "</a></p>";
	}
	return (generateHtmlPage("Index directory " + path, oss.str()));
}
