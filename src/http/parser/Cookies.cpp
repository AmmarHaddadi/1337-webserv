#include "Parser.hpp"
#include <sstream>

using namespace http;

// Cookie: session=abc123; theme=dark

void set_cookies(HttpRequest &req, std::string &part) {
	size_t pos = part.find('=');
	if (pos == std::string::npos)
		return;

	std::string key = part.substr(0, pos);
	std::string value = part.substr(pos + 1);

	// trim key spaces
	size_t i = 0;
	size_t j = 0;
	while (i < key.length() && std::isspace(key[i]))
		i++;
	key.erase(0, i);

	j = key.length() - 1;
	while (j > 0 && std::isspace(key[j]))
		j--;
	key.erase(j + 1);
	i = 0;

	// check if key is empty
	if (key.empty() || value.empty())
		return;

	// trim value
	while (i < value.length() && std::isspace(value[i]))
		i++;
	value.erase(0, i);
	j = value.length() - 1;
	while (j > 0 && std::isspace(value[j]))
		j--;
	value.erase(j + 1);

	// NOTE cookie key's value can be empty RFC 6265
	req.cookies[key] = value;
}

void http::parseCookies(HttpRequest &req) {
	std::map<std::string, std::string>::iterator it = req.headers.find("Cookie");
	if (it == req.headers.end())
		return;
	std::string cookies = it->second;
	while (true) {
		size_t pos = cookies.find(";");
		std::string part = cookies.substr(0, pos);
		set_cookies(req, part);
		if (pos == std::string::npos)
			break;
		cookies = cookies.substr(pos + 1);
	}
}