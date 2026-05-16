#pragma once

#include <sstream>
#include <string>
#include <vector>
#include "../src/http_parser/Parser.hpp"
namespace Utils {

std::string fakeHttpRes();
std::vector<std::string> split(const std::string &s, char delimiter);
bool isAllNum(const std::string &s);
std::string responseFile(HttpRequest &req);
std::string generateErrorPage(int status);
std::string generateHtmlErrorPage(int code, const std::string& msg);

} // namespace Utils
