#include "../../cfg/cfg.hpp"
#include "../../shared/utils.hpp"
#include "response.hpp"
#include <dirent.h>

using namespace http;

bool http::defaultFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req, std::string &resolvedPath) {
	std::string fullPath = resolvedPath;
	Utils::addSlash(fullPath);
	const char *pathDir = fullPath.c_str();
	DIR *dr = opendir(pathDir);
	struct dirent *en;
	struct stat st;
	if (dr != 0) {
		while ((en = readdir(dr)) != NULL) {
			if (std::string(en->d_name) == rc.default_file) {
				fullPath += en->d_name;
				if (stat(fullPath.c_str(), &st) != 0) {
					closedir(dr);
					throw std::runtime_error("Runtime error: bad file");
				}
				if ((st.st_mode & S_IFMT) != S_IFREG) {
					sMeta.responseBuf = generateHttpResponse(NOT_FOUND, req.keepAlive,
															 generateErrorPage(sMeta.server, NOT_FOUND));
					closedir(dr);
					return true;
				}
				// Utils::addSlash(req.path);
				// req.path.append(std::string(en->d_name));
				getFile(sMeta, fullPath, req);
				closedir(dr);
				return true;
			}
		}
		closedir(dr);
	} else
		throw std::runtime_error("Runtime error opendir failed");
	return false;
}

void http::directoryFiles(SocketMeta &sMeta, HttpRequest &req, std::string &systemPath) {
	std::vector<std::string> files;
	std::string fullPath = systemPath;
	const char *pathDir = fullPath.c_str();
	DIR *dr = opendir(pathDir);
	struct dirent *en;
	if (dr != 0) {
		while ((en = readdir(dr)) != NULL) {
			if (std::string(en->d_name) == "." || std::string(en->d_name) == "..")
				continue;
			files.push_back(std::string(en->d_name));
		}
		closedir(dr);
		Utils::addSlash(req.path);
		std::string html = generateHttpResponseDirectory(req.path, files);
		sMeta.responseBuf = generateHttpResponse(OK, req.keepAlive, html);
	} else
		throw std::runtime_error("Runtime error opendir failed");
}
