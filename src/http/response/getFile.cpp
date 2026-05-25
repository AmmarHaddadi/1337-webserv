#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <fstream>
#include <sstream>
#include <string>

using namespace http;

void http::getFile(SocketMeta &sMeta, HttpRequest &req) {
	std::ifstream file((sMeta.server.root + req.path).c_str());
	if (!file.is_open()) {
		sMeta.responseBuf = generateHttpResponse(NOT_FOUND, generateErrorPage(NOT_FOUND));
	} else {
		std::map<std::string, std::string> hdr;
		hdr["Content-type"] = Utils::getMimeType(req.path);
		std::stringstream buffer;
		buffer << file.rdbuf();
		sMeta.responseBuf = generateHttpResponse(OK, hdr, buffer.str());
	}
}
