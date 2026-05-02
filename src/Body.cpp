#include "../header/Parser.hpp"

void parseBody(const std::string &bodySection, HttpRequest &request) {
	request.body = bodySection;
}