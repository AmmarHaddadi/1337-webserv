#pragma once

#include "../http.hpp"

namespace http {
HttpRequest parseHttp(std::string &buf, size_t maxBodySize);

HttpRequest parseRequestLine(const std::string &line);
void parseHeaders(const std::string &headerSection, HttpRequest &request);
void parseCookies(HttpRequest &request);
void printHttpRequest(const HttpRequest &request);
} // namespace http
