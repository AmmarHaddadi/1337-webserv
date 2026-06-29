#include "../../cfg/cfg.hpp"
#include "response.hpp"
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>

using namespace http;

// Helper to recursively create directories in the target path if they do not exist.
bool createDirectory(const std::string &srcPath) {
	struct stat st;
	bool dir = 0;
	std::stringstream ss(srcPath);
	std::string buffer;
	std::string fnl;
	std::string oldDir;

	while (getline(ss, buffer, '/')) {
		fnl.append(buffer + "/");
		if (dir == 1) {
			int ret = mkdir(oldDir.c_str(), 0755);
			if (ret == -1)
				return 0;
			dir = 0;
		}
		if (stat(fnl.c_str(), &st) != 0)
			dir = 1;
		oldDir = fnl;
	}
	return 1;
}

// Trims spaces, tabs, carriage returns, and line feeds from both ends of a string.
static std::string trim(const std::string &str) {
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

// Extracts the boundary string from the request headers if the Content-Type is multipart/form-data.
static std::string getBoundary(const HttpRequest &req) {
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin();
		 it != req.headers.end(); ++it) {
		std::string key = it->first;
		for (size_t i = 0; i < key.size(); ++i) {
			key[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[i])));
		}
		if (key == "content-type") {
			std::string val = it->second;
			std::string lowerVal = val;
			for (size_t j = 0; j < lowerVal.size(); ++j) {
				lowerVal[j] = static_cast<char>(std::tolower(static_cast<unsigned char>(lowerVal[j])));
			}
			// Verify that the content-type actually specifies multipart/form-data
			if (lowerVal.find("multipart/form-data") == std::string::npos) {
				continue;
			}
			size_t pos = val.find("boundary=");
			if (pos != std::string::npos) {
				std::string boundary = val.substr(pos + 9);
				// Truncate any parameters that might appear after the boundary string (separated by semicolon)
				size_t semi = boundary.find(';');
				if (semi != std::string::npos) {
					boundary = boundary.substr(0, semi);
				}
				// Trim any whitespaces, tabs, or newlines from both sides of the boundary
				boundary = trim(boundary);
				// Strip surrounding quotes if the client wrapped the boundary in quotes
				if (boundary.size() >= 2 && boundary[0] == '"' && boundary[boundary.size() - 1] == '"') {
					boundary = boundary.substr(1, boundary.size() - 2);
				}
				return boundary;
			}
		}
	}
	return "";
}

// Parses the multipart/form-data body to locate and extract the uploaded file part.
static std::string parseMultipartBody(const std::string &body, const std::string &boundary) {
	std::string divider = "--" + boundary;
	size_t pos = body.find(divider);
	if (pos == std::string::npos)
		return body;

	// Loop through all parts in the multipart body to find the file part (which has a filename parameter).
	while (pos != std::string::npos) {
		size_t nextPos = body.find(divider, pos + divider.length());
		if (nextPos == std::string::npos)
			break;

		size_t partStart = pos + divider.length();
		// Skip the Carriage Return (\r) and Line Feed (\n) characters that follow the
		// boundary line. According to HTTP multipart specs, a boundary is separated
		// from its part headers by a CRLF. If not skipped, it corrupts header parsing.
		if (partStart < body.size() && body[partStart] == '\r') partStart++;
		if (partStart < body.size() && body[partStart] == '\n') partStart++;

		// Locate the blank line delimiter (\r\n\r\n) which separates part headers from the part content.
		size_t headersEnd = body.find("\r\n\r\n", partStart);
		if (headersEnd != std::string::npos && headersEnd < nextPos) {
			std::string headers = body.substr(partStart, headersEnd - partStart);
			// We look for the part that contains "filename=" to identify the uploaded file part.
			if (headers.find("filename=") != std::string::npos) {
				size_t contentStart = headersEnd + 4;
				size_t contentEnd = nextPos;
				// Trim the trailing CRLF (which is part of the boundary separator) from the end of the file data.
				if (contentEnd >= 2 && body[contentEnd - 2] == '\r' && body[contentEnd - 1] == '\n') {
					contentEnd -= 2;
				} else if (contentEnd >= 1 && body[contentEnd - 1] == '\n') {
					contentEnd -= 1;
				}
				if (contentEnd > contentStart) {
					return body.substr(contentStart, contentEnd - contentStart);
				}
			}
		}
		pos = nextPos;
	}

	// Fallback mechanism: if no part specifies "filename=", default to extracting the first part's body.
	pos = body.find(divider);
	size_t partStart = pos + divider.length();
	// Skip the CRLF following the boundary separator.
	if (partStart < body.size() && body[partStart] == '\r') partStart++;
	if (partStart < body.size() && body[partStart] == '\n') partStart++;
	size_t headersEnd = body.find("\r\n\r\n", partStart);
	if (headersEnd != std::string::npos) {
		size_t nextPos = body.find(divider, headersEnd);
		if (nextPos != std::string::npos) {
			size_t contentStart = headersEnd + 4;
			size_t contentEnd = nextPos;
			// Trim the trailing CRLF preceding the next boundary separator.
			if (contentEnd >= 2 && body[contentEnd - 2] == '\r' && body[contentEnd - 1] == '\n') {
				contentEnd -= 2;
			} else if (contentEnd >= 1 && body[contentEnd - 1] == '\n') {
				contentEnd -= 1;
			}
			if (contentEnd > contentStart) {
				return body.substr(contentStart, contentEnd - contentStart);
			}
		}
	}

	return body;
}

// Entrypoint for handling file uploads. Decides whether to parse as multipart or write raw body.
void http::uploadFile(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
					  struct stat &st, std::string &resolvedPath, bool exist) {
	if (rc.uploadEnabled && req.path[req.path.length() - 1] != '/') {
		if (exist && (st.st_mode & S_IFMT) == S_IFREG) {
			if (unlink((resolvedPath).c_str()) != 0) {
				sMeta.responseBuf = generateHttpResponse(
					INTERNAL_SERVER_ERROR, req.keepAlive,
					generateErrorPage(sMeta.server, INTERNAL_SERVER_ERROR), sMeta.RespHeader);
				return;
			}
		}
		if (!createDirectory(resolvedPath))
			sMeta.responseBuf = generateHttpResponse(
				INTERNAL_SERVER_ERROR, req.keepAlive,
				generateErrorPage(sMeta.server, INTERNAL_SERVER_ERROR), sMeta.RespHeader);
		std::ofstream file((resolvedPath).c_str(), std::ios::binary);
		if (!file) {
			sMeta.responseBuf = generateHttpResponse(
				INTERNAL_SERVER_ERROR, req.keepAlive,
				generateErrorPage(sMeta.server, INTERNAL_SERVER_ERROR), sMeta.RespHeader);
			return;
		}

		std::string dataToWrite = req.body;
		std::string boundary = getBoundary(req);
		// If a multipart boundary was successfully identified, extract only the file data.
		if (!boundary.empty()) {
			dataToWrite = parseMultipartBody(req.body, boundary);
		}

		file.write(dataToWrite.c_str(), static_cast<unsigned int>(dataToWrite.length()));
		file.close();
		sMeta.responseBuf = generateHttpResponse(
			OK, req.keepAlive, req.path.substr(req.path.rfind('/') + 1) + ": is created",
			sMeta.RespHeader);
	} else
		sMeta.responseBuf =
			generateHttpResponse(BAD_REQUEST, req.keepAlive,
								 generateErrorPage(sMeta.server, BAD_REQUEST), sMeta.RespHeader);
}
