#pragma once

#include "../core/core.hpp"
#include "../http/http.hpp"
#include <string>
#include <vector>

#define HTTP_VER "HTTP/1.1"

namespace Utils {

std::vector<std::string> split(const std::string &s, char delimiter);
bool isAllNum(const std::string &s);
std::string getMimeType(const std::string &filePath);

} // namespace Utils
