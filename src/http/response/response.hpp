#pragma once

#include "../../core/core.hpp"
#include "../http.hpp"
#include <string>
#include <sys/stat.h>

namespace http {
void respondToReq(SocketMeta &sMeta, HttpRequest &req, std::vector<pollfd> &sockets,
				  std::map<int, struct SocketMeta> &socketsMeta, int clientFd);
void getFile(SocketMeta &sMeta, const HttpRequest &req);

// helpers
std::string generateHtmlPage(const std::string &title, const std::string &body);
std::string generateErrorPage(HTTPCode code);

std::string generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body);
std::string generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body,
								 std::map<std::string, std::string> headers);
std::string generateHttpResponseDirectory(const std::string &path, std::vector<std::string> &files);
bool isCgi(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
		   std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
		   int clientFd);
void directoryFiles(SocketMeta &sMeta, HttpRequest &req);
bool defaultFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req);
void uploadFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
				struct stat &st, bool exist);
} // namespace http
