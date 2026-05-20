#pragma once

#include <sstream>
#include <string>
#include <vector>
#include "./http_parser/Parser.hpp"
#include "./cfg/cfg.hpp"

namespace Utils {

std::string fakeHttpRes(std::vector<Config::ServerConfig> &servers, HttpRequest &req);
std::vector<std::string> split(const std::string &s, char delimiter);
bool isAllNum(const std::string &s);

} // namespace Utils
