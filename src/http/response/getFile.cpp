#include "../../core/core.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <fstream>
#include <sstream>
#include <string>

using namespace http;

void http::getFile(SocketMeta &sMeta, HttpRequest &req) {
	// TODO change to server's root
	std::ifstream file(("/tmp" + req.path).c_str());
	if (!file.is_open()) {
		sMeta.responseBuf = generateHttpResponse(404, generateErrorPage(404));
	} else {
		std::map<std::string, std::string> hdr;
		hdr["Content-type"] = Utils::getMimeType(req.path);
		std::stringstream buffer;
		buffer << file.rdbuf();
		sMeta.responseBuf = generateHttpResponse(200, hdr, buffer.str());
	}
}
