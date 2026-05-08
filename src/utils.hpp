#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace Utils {

std::string fakeHttpRes();
std::vector<std::string> split(const std::string &s, char delimiter);
bool isAllNum(const std::string &s);

} // namespace Utils
