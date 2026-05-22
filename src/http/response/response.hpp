#pragma once

#include "../../core/core.hpp"
#include "../http.hpp"
#include <string>

namespace http {
void respondToReq(SocketMeta &sMeta, HttpRequest &req);
void getFile(SocketMeta &sMeta, HttpRequest &req);

// helpers
std::string generateHtmlPage(const std::string &title, const std::string &body);
std::string generateErrorPage(int code);
std::string generateHttpResponse(int httpCode, const std::map<std::string, std::string> &headers,
								 const std::string &body);
std::string generateHttpResponse(int httpCode, const std::string &body);
} // namespace http
