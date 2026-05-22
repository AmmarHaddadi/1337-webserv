#include "utils.hpp"
#include "../http/http.hpp"
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
