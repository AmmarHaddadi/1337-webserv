#include "../../cfg/cfg.hpp"
#include "response.hpp"
#include <fstream>
#include <sstream>

using namespace http;

bool createDirectory(const std::string &srcPath) {
	struct stat st;
	bool dir = 0;
    std::stringstream ss(srcPath);
    std::string buffer;
    std::string fnl;
	std::string oldDir;

    while (getline(ss, buffer, '/')) {
        fnl.append(buffer + "/");
		if (dir == 1) {
			int ret = mkdir(oldDir.c_str(), 0755);
			if (ret == -1)
				return 0;
			dir = 0;
		}
		if (stat(fnl.c_str(), &st) != 0)
			dir = 1;
		oldDir = fnl;
    }
	return 1;
}

void http::uploadFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
					  struct stat &st, std::string &resolvedPath, bool exist) {
	if (rc.uploadEnabled && req.path[req.path.length() - 1] != '/') {
		if (exist && (st.st_mode & S_IFMT) == S_IFREG) {
			if (unlink((resolvedPath).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
														 generateErrorPage(sMeta.server, INTERNAL_SERVER_ERROR),
														 sMeta.RespHeader);
				return;
			}
		}
		if (!createDirectory(resolvedPath))
			sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
													 generateErrorPage(sMeta.server, INTERNAL_SERVER_ERROR),
													 sMeta.RespHeader);
		std::ofstream file((resolvedPath).c_str(), std::ios::binary);
		if (!file) {
			sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
													 generateErrorPage(sMeta.server, INTERNAL_SERVER_ERROR),
													 sMeta.RespHeader);
			return;
		}
		file.write(req.body.c_str(), static_cast<unsigned int>(req.body.length()));
		file.close();
		sMeta.responseBuf = generateHttpResponse(
			OK, req.keepAlive, req.path.substr(req.path.rfind('/') + 1) + ": is created", sMeta.RespHeader);
	} else
		sMeta.responseBuf =
			generateHttpResponse(BAD_REQUEST, req.keepAlive, generateErrorPage(sMeta.server, BAD_REQUEST),
								 sMeta.RespHeader);
}
