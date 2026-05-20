#pragma once

#include "../src/http_parser/Parser.hpp"
#include "core/core.hpp"
#include <sstream>
#include <string>
#include <vector>
namespace Utils {

std::vector<std::string> split(const std::string &s, char delimiter);
bool isAllNum(const std::string &s);
std::string responseFile(SocketMeta &sMeta, HttpRequest &req);
std::string generateHtmlErrorPage(int code);

} // namespace Utils
