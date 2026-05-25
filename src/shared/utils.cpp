#include "utils.hpp"
#include "../http/http.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

CoreLogger coreLogger("Core", CoreLogger::DEBUG);			   // NOLINT
CfgLogger cfgLogger("CFG", CfgLogger::DEBUG);				   // NOLINT
HttpReqLogger httpReqLogger("Http Req", HttpReqLogger::DEBUG); // NOLINT

using namespace Utils;

std::vector<std::string> Utils::split(const std::string &s, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(s);
	std::string token;

	while (std::getline(iss, token, delimiter)) // NOLINT
		result.push_back(token);

	return result;
}

bool Utils::isAllNum(const std::string &s) {
	if (s.empty())
		return false;
	for (size_t i = 0; i < s.size(); i++)
		if (std::isdigit(static_cast<unsigned char>(s[i])) == 0)
			return false;
	return true;
}

std::string Utils::getMimeType(const std::string &filePath) {
	size_t dotPos = filePath.rfind('.');
	if (dotPos == std::string::npos || dotPos == filePath.length() - 1)
		return "application/octet-stream";

	std::string ext = filePath.substr(dotPos + 1);

	// Convert to lowercase for comparison
	for (size_t i = 0; i < ext.length(); ++i)
		ext[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(ext[i])));

	// MIME type mappings
	if (ext == "html" || ext == "htm")
		return "text/html";
	if (ext == "css")
		return "text/css";
	if (ext == "js")
		return "application/javascript";
	if (ext == "json")
		return "application/json";
	if (ext == "xml")
		return "application/xml";
	if (ext == "txt")
		return "text/plain";
	if (ext == "csv")
		return "text/csv";
	if (ext == "png")
		return "image/png";
	if (ext == "jpg" || ext == "jpeg")
		return "image/jpeg";
	if (ext == "gif")
		return "image/gif";
	if (ext == "svg")
		return "image/svg+xml";
	if (ext == "webp")
		return "image/webp";
	if (ext == "ico")
		return "image/x-icon";
	if (ext == "pdf")
		return "application/pdf";
	if (ext == "zip")
		return "application/zip";
	if (ext == "mp4")
		return "video/mp4";
	if (ext == "mpeg" || ext == "mp3")
		return "audio/mpeg";
	if (ext == "webm")
		return "video/webm";

	return "application/octet-stream";
}

std::string Utils::httpMethodToString(const http::HttpMethod &method) {
	switch (method) {
	case http::GET:
		return "GET";
	case http::POST:
		return "POST";
	case http::DELETE:
		return "DELETE";
	case http::INVALID:
		return "INVALID";
	default:
		return "UNKNOWN";
	}
}

std::string Utils::decodeURL(const std::string &url) {
	std::string decoded;
	for (unsigned i = 0; i < url.length(); i++) {
		if (url[i] != '%') {
			decoded.push_back(url[i]);
			continue;
		}
		if (i + 2 >= url.length()) {
			decoded.push_back('%');
			continue;
		}

		bool decodedSomething = false; // Track if we hit a valid match

		if (url[i + 1] == '2') {
			char c2 = static_cast<char>(std::tolower(url[i + 2]));
			decodedSomething = true; // Assume true, reset if we miss

			if (c2 == '0')
				decoded.push_back(' ');
			else if (c2 == '1')
				decoded.push_back('!');
			else if (c2 == '2')
				decoded.push_back('"');
			else if (c2 == '3')
				decoded.push_back('#');
			else if (c2 == '4')
				decoded.push_back('$');
			else if (c2 == '5')
				decoded.push_back('%');
			else if (c2 == '6')
				decoded.push_back('&');
			else if (c2 == '7')
				decoded.push_back('\'');
			else if (c2 == '8')
				decoded.push_back('(');
			else if (c2 == '9')
				decoded.push_back(')');
			else if (c2 == 'a')
				decoded.push_back('*'); // %2a is '*'
			else if (c2 == 'b')
				decoded.push_back('+'); // %2b is '+'
			else if (c2 == 'c')
				decoded.push_back(','); // %2c is ','
			else if (c2 == 'd')
				decoded.push_back('-'); // %2d is '-'
			else if (c2 == 'e')
				decoded.push_back('.'); // %2e is '.'
			else if (c2 == 'f')
				decoded.push_back('/'); // %2f is '/'
			else
				decodedSomething = false;
		} else if (url[i + 1] == '3') {
			char c2 = static_cast<char>(std::tolower(url[i + 2]));
			decodedSomething = true;

			if (c2 == 'a')
				decoded.push_back(':'); // %3a is ':'
			else if (c2 == 'b')
				decoded.push_back(';'); // %3b is ';'
			else if (c2 == 'c')
				decoded.push_back('<'); // %3c is '<'
			else if (c2 == 'd')
				decoded.push_back('='); // %3d is '='
			else if (c2 == 'e')
				decoded.push_back('>'); // %3e is '>'
			else if (c2 == 'f')
				decoded.push_back('?'); // %3f is '?'
			else
				decodedSomething = false;
		}

		if (decodedSomething) {
			i += 2;
		} else {
			// Fallback: If it was a hex code we don't handle (like %40),
			// just treat the '%' as a literal character and move on by 1.
			decoded.push_back('%');
		}
	}
	return decoded;
}
