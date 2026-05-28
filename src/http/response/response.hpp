#pragma once

#include "../../core/core.hpp"
#include "../http.hpp"
#include <string>

namespace http {
void respondToReq(SocketMeta &sMeta, const HttpRequest &req);
void getFile(SocketMeta &sMeta, const HttpRequest &req);

// helpers
std::string generateHtmlPage(const std::string &title, const std::string &body);
std::string generateErrorPage(HTTPCode code);

std::string generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body);
std::string generateHttpResponse(HTTPCode code, bool keepAlive, const std::string &body,
								 std::map<std::string, std::string> headers);
} // namespace http
