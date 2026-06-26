#pragma once

#include "../../core/core.hpp"
#include "../http.hpp"
#include <string>
#include <sys/stat.h>

namespace http {
void respondToReq(SocketMeta &sMeta, HttpRequest &req, std::vector<pollfd> &sockets,
				  std::map<int, struct SocketMeta> &socketsMeta, int clientFd);
void respondToReq(SocketMeta &sMeta, const HttpRequest &req);
void getFile(SocketMeta &sMeta, const std::string &systemPath, const HttpRequest &req);

// helpers
std::string generateHtmlPage(const std::string &title, const std::string &body);
std::string generateErrorPage(HTTPCode code);

std::string generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body);
std::string generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body,
								 std::map<std::string, std::string> headers);
std::string generateHttpResponseDirectory(const std::string &path, std::vector<std::string> &files);
bool isCgi(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
		   std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
		   int clientFd, std::string &resolvedPath);
void directoryFiles(SocketMeta &sMeta, HttpRequest &req, std::string &systemPath);
bool defaultFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
				 std::string &resolvedPath);
void uploadFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
				struct stat &st, std::string &resolvedPath, bool exist);
} // namespace http
