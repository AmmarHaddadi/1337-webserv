#pragma once

#include "../core/core.hpp"
#include "../http/http.hpp"
#include "../http/parser/Parser.hpp"
#include "../http/response/response.hpp"
#include <string>
#include <vector>

namespace Utils {

std::vector<std::string> split(const std::string &s, char delimiter);
bool isAllNum(const std::string &s);
std::string getMimeType(const std::string &filePath);
std::string decodeURL(const std::string &url);
std::string httpMethodToString(const http::HttpMethod &method);
void addSlash(std::string &ref);
} // namespace Utils
