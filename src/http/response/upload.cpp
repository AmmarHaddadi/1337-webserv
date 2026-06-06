#include "../../cfg/cfg.hpp"
#include "response.hpp"
#include <fstream>
#include <sstream>

using namespace http;

void http::uploadFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
					  struct stat &st, bool exist) {
	if (rc.uploadEnabled && req.path[req.path.length() - 1] != '/') {
		if (exist && (st.st_mode & S_IFMT) == S_IFREG) {
			if (unlink((sMeta.server.root + req.path).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive, generateErrorPage(INTERNAL_SERVER_ERROR));
				return;
			}
		}
		std::ofstream file((sMeta.server.root + req.path).c_str(), std::ios::binary);
		if (!file) {
			sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive, generateErrorPage(INTERNAL_SERVER_ERROR));
			return;
		}
		file.write(req.body.c_str(), static_cast<unsigned int>(req.body.length()));
		file.close();
		sMeta.responseBuf =
			generateHttpResponse(OK, req.keepAlive, req.path.substr(req.path.rfind('/') + 1) + ": is created");
	} else
		sMeta.responseBuf = generateHttpResponse(BAD_REQUEST, req.keepAlive, generateErrorPage(BAD_REQUEST));
}
