#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <fstream>
#include <sstream>
#include <string>

using namespace http;

void http::getFile(SocketMeta &sMeta, const std::string &systemPath, const HttpRequest &req) {
	std::ifstream file(systemPath.c_str());
	if (!file.is_open()) {
		sMeta.responseBuf = generateHttpResponse(
			NOT_FOUND, req.keepAlive, generateErrorPage(sMeta.server, NOT_FOUND), sMeta.RespHeader);
	} else {
		sMeta.RespHeader["Content-Type"] = Utils::getMimeType(systemPath);
		std::stringstream buffer;
		buffer << file.rdbuf();
		sMeta.responseBuf = generateHttpResponse(OK, req.keepAlive, buffer.str(), sMeta.RespHeader);
	}
}
