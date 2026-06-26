#include "../../cfg/cfg.hpp"
#include "../../cgi/cgi.hpp"
#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <map>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace http;

// bro shof shi blasa akhra l had funcition
bool http::isCgi(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
				 std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
				 int clientFd, std::string &resolvedPath) {
	size_t pos = req.path.rfind('.');
	if (pos != std::string::npos) {
		std::string ext = req.path.substr(pos + 1);
		std::map<std::string, std::string>::iterator it = rc.cgi.find(ext);
		if (it == rc.cgi.end())
			return false;
		CGI::Cgi cgi(rc.cgi, req);
		try {
			int outFd = -1;
			int inFd = -1;
			pid_t pid = cgi.executeCGI(resolvedPath, outFd, inFd);
			if (!req.body.empty()) {
				size_t totalWritten = 0;
				while (totalWritten < req.body.size()) {
					ssize_t written = write(inFd, req.body.c_str() + totalWritten,
											req.body.size() - totalWritten);
					if (written > 0) {
						totalWritten += static_cast<size_t>(written);
						continue;
					}
					if (written < 0)
						continue;
					if (outFd == inFd) {
						close(outFd);
					} else {
						close(outFd);
						close(inFd);
					}
					waitpid(pid, NULL, WNOHANG);
					sMeta.responseBuf =
						generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
											 generateErrorPage(INTERNAL_SERVER_ERROR));
					return true;
				}
			}
			close(inFd);
			int socketFlags = fcntl(outFd, F_GETFL, 0);
			if (socketFlags != -1)
				fcntl(outFd, F_SETFL, socketFlags | O_NONBLOCK);
			pollfd cgiPfd = {outFd, POLLIN, 0};
			sockets.push_back(cgiPfd);
			SocketMeta pipeMeta(sMeta.server);
			pipeMeta.isCgiPipe = true;
			pipeMeta.clientFd = clientFd;
			pipeMeta.cgiPid = pid;
			socketsMeta.insert(std::make_pair(outFd, pipeMeta));
			sMeta.cgiPipeFd = outFd;
		} catch (const std::exception &e) {
			sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
													 generateErrorPage(INTERNAL_SERVER_ERROR));
		}
		return true;
	}
	return false;
}

std::string resolveSystemPath(const std::string &routePath, const std::string &routeRoot,
							  const std::string &reqPath) {
	std::string relativePath = reqPath;

	// strip route path (perifx) from request path (if not "/")
	// e.g: request path /images/img1.png to route of path /images will result in relative path of
	// /img1.png
	if (routePath != "/" && reqPath.compare(0, routePath.length(), routePath) == 0) {
		relativePath = reqPath.substr(routePath.length());
	}
	return routeRoot + relativePath;
}

// NOTE try to move raw response building into sub funcs and keep this highlevel
void http::respondToReq(SocketMeta &sMeta, HttpRequest &req, std::vector<pollfd> &sockets,
						std::map<int, struct SocketMeta> &socketsMeta, int clientFd) {
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

	std::string systemPath = resolveSystemPath(rc->path, rc->root, req.path);


	if (rc->hasRedirect) {
		std::map<std::string, std::string> headers;
		headers["Location"] = rc->redirectLocation;
		sMeta.responseBuf = generateHttpResponse(TEMPORARY_REDIRECT, req.keepAlive, "", headers);
		return;
	}

	if (isCgi(*rc, sMeta, req, sockets, socketsMeta, clientFd, systemPath))
		return;

	// check if file exists unless it's a POST (might be upload)
	struct stat st;
	bool exist = true;
	if (stat((systemPath).c_str(), &st) != 0) {
		exist = false;
		if (req.method != http::POST) {
			sMeta.responseBuf =
				generateHttpResponse(NOT_FOUND, req.keepAlive, generateErrorPage(NOT_FOUND));
			return;
		}
	}

	if (req.method == http::GET && rc->fileServer) {
		// if requested a file
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			getFile(sMeta, systemPath, req);
			return;
		}

		// if requested a directory
		// this shit needs some abstraction
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			try {
				if (defaultFile(*rc, sMeta, req, systemPath))
					return;
			} catch (const std::exception &e) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
														 generateErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
			if (rc->fileBrowser) {
				try {
					// pass real path
					directoryFiles(sMeta, req, systemPath);
					return;
				} catch (const std::exception &e) {
					sMeta.responseBuf =
						generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
											 generateErrorPage(INTERNAL_SERVER_ERROR));
					return;
				}
			}
		}
	}

	else if (req.method == http::POST) {
		uploadFile(*rc, sMeta, req, st, exist);

		return;
	}

	else if (req.method == http::DELETE) {
		if ((st.st_mode & S_IFMT) == S_IFREG) {
			if (unlink((systemPath).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
														 generateErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
		}
		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			if (rmdir((systemPath).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
														 generateErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
		}
		// the server.root + req.path can remain here for better UX
		sMeta.responseBuf =
			generateHttpResponse(OK, req.keepAlive, req.path + ": is removed");
		return;
	}

	// fallback
	sMeta.responseBuf =
		generateHttpResponse(NOT_FOUND, req.keepAlive, generateErrorPage(NOT_FOUND));
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
	codeMessages[TEMPORARY_REDIRECT] = "Temporary Redirect";
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
	codeMessages[TEMPORARY_REDIRECT] = "Temporary Redirect";
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

std::string http::generateHttpResponseDirectory(const std::string &path,
												std::vector<std::string> &files) {
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
