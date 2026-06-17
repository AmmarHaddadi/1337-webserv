#include "Parser.hpp"
#include "../../shared/utils.hpp"
#include "../http.hpp"
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <sstream>

using namespace http;

namespace {

bool parseHexSize(const std::string &hexStr, size_t &out) {
	if (hexStr.empty())
		return false;
	errno = 0;
	char *end = NULL;
	unsigned long val = std::strtoul(hexStr.c_str(), &end, 16);
	if (errno == ERANGE || *end != '\0')
		return false;
	out = static_cast<size_t>(val);
	return true;
}

struct ChunkedDecodeParams {
	size_t bodyStart;
	size_t maxBodySize;
};

RequestStatus decodeChunkedBody(const std::string &reqBuf, const ChunkedDecodeParams &params,
								std::string &body, size_t &consumedBytes) {
	size_t pos = params.bodyStart;
	body.clear();
	consumedBytes = 0;

	while (true) {
		size_t lineEnd = reqBuf.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			return INCOMPLETE;

		std::string chunkLine = reqBuf.substr(pos, lineEnd - pos);
		size_t semiColon = chunkLine.find(';');
		std::string chunkSizeStr =
			(semiColon != std::string::npos) ? chunkLine.substr(0, semiColon) : chunkLine;

		size_t chunkSize = 0;
		if (!parseHexSize(chunkSizeStr, chunkSize))
			return BAD_REQ;

		if (chunkSize == 0) {
			pos = lineEnd + 2;
			break;
		}

		if (body.size() > params.maxBodySize - chunkSize)
			return TOO_LARGE;

		pos = lineEnd + 2;
		if (reqBuf.size() < pos + chunkSize + 2)
			return INCOMPLETE;

		body.append(reqBuf, pos, chunkSize);
		pos += chunkSize;
		if (reqBuf.compare(pos, 2, "\r\n") != 0)
			return BAD_REQ;
		pos += 2;
	}

	while (true) {
		size_t lineEnd = reqBuf.find("\r\n", pos);
		if (lineEnd == std::string::npos)
			return INCOMPLETE;
		if (lineEnd == pos) {
			pos += 2;
			break;
		}
		pos = lineEnd + 2;
	}

	consumedBytes = pos - params.bodyStart;
	return COMPLETE;
}

} // namespace

HttpRequest http::parseHttp(std::string &reqBuf, size_t maxBodySize) {
	size_t firstLineEnd = reqBuf.find("\r\n");
	if (firstLineEnd == std::string::npos || firstLineEnd == 0) {
		HttpRequest req;
		req.status = BAD_REQ;
		return req;
	}

	size_t headersEnd = reqBuf.find("\r\n\r\n");
	if (headersEnd == std::string::npos) {
		HttpRequest req;
		req.status = INCOMPLETE;
		return req;
	}

	std::string requestLineToParse = reqBuf.substr(0, firstLineEnd);
	size_t headersStart = firstLineEnd + 2;
	std::string headersToParse = reqBuf.substr(headersStart, headersEnd - headersStart);

	HttpRequest req = parseRequestLine(requestLineToParse);
	if (req.status == BAD_REQ)
		return req;

	parseHeaders(headersToParse, req);
	if (req.status == BAD_REQ)
		return req;

	std::string transferEncoding;
	std::string contentLength;
	std::string connectionValue;
	bool hasChunked = false;
	bool hasContentLen = false;
	bool hasConnection = false;

	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin();
		 it != req.headers.end(); ++it) {
		if (it->first.empty())
			continue;
		std::string key = it->first;
		for (size_t i = 0; i < key.size(); ++i) {
			unsigned char ch = static_cast<unsigned char>(key[i]);
			key[i] = static_cast<char>(std::tolower(ch));
		}

		if (key == "transfer-encoding") {
			transferEncoding = it->second;
			hasChunked = (transferEncoding == "chunked");
		} else if (key == "content-length") {
			contentLength = it->second;
			hasContentLen = true;
		} else if (key == "connection") {
			connectionValue = it->second;
			hasConnection = true;
		}
	}
	req.keepAlive = false;
	if (hasConnection) {
		std::string lowerVal = connectionValue;
		for (size_t i = 0; i < lowerVal.size(); ++i) {
			unsigned char ch = static_cast<unsigned char>(lowerVal[i]);
			lowerVal[i] = static_cast<char>(std::tolower(ch));
		}
		if (lowerVal.find("close") != std::string::npos)
			req.keepAlive = false;
		else if (lowerVal.find("keep-alive") != std::string::npos)
			req.keepAlive = true;
	}

	if (hasChunked) {
		if (hasContentLen) {
			req.status = BAD_REQ;
			return req;
		}
		ChunkedDecodeParams params;
		params.bodyStart = headersEnd + 4;
		params.maxBodySize = maxBodySize;
		size_t consumedBytes = 0;
		req.status = decodeChunkedBody(reqBuf, params, req.body, consumedBytes);
		if (req.status != COMPLETE)
			return req;

		size_t totalConsumed = headersEnd + 4 + consumedBytes;
		if (totalConsumed <= reqBuf.size())
			reqBuf.erase(0, totalConsumed);
		return req;
	}

	if (hasContentLen) {
		errno = 0;
		char *end = NULL;
		unsigned long parsed = std::strtoul(contentLength.c_str(), &end, 10);
		if (errno == ERANGE || *end != '\0') {
			req.status = BAD_REQ;
			return req;
		}

		size_t contentLenSize = static_cast<size_t>(parsed);
		if (contentLenSize > maxBodySize) {
			req.status = TOO_LARGE;
			return req;
		}

		if (reqBuf.size() < headersEnd + 4 + contentLenSize) {
			req.status = INCOMPLETE;
			return req;
		}

		req.body = reqBuf.substr(headersEnd + 4, contentLenSize);
		size_t totalConsumed = headersEnd + 4 + contentLenSize;
		if (totalConsumed <= reqBuf.size())
			reqBuf.erase(0, totalConsumed);

		return req;
	}

	size_t totalConsumed = headersEnd + 4;
	if (totalConsumed <= reqBuf.size()) {
		reqBuf.erase(0, totalConsumed);
	}

	return req;
}

// Print function to display parsed HTTP request
void http::printHttpRequest(const HttpRequest &request) {
	std::time_t now = std::time(0);
	std::string dt = std::ctime(&now);
	if (!dt.empty() && dt[dt.length() - 1] == '\n') {
		dt.erase(dt.length() - 1);
	}

	std::cout << dt << " -> " << Utils::httpMethodToString(request.method) << " " << request.path
			  << "\n";

	std::ostringstream oss;

	oss << "\nMETHOD: ";
	switch (request.method) {
	case GET:
		oss << "GET" << '\n';
		break;
	case POST:
		oss << "POST" << '\n';
		break;
	case DELETE:
		oss << "DELETE" << '\n';
		break;
	case INVALID:
		oss << "INVALID" << '\n';
		break;
	}

	oss << "PATH: " << request.path << '\n';
	if (!request.query.empty()) {
		oss << "QUERY: " << request.query << '\n';
	}

	oss << "VERSION: " << request.version << '\n';

	oss << "\nHEADERS:" << '\n';
	if (request.headers.empty()) {
		oss << "  (none)" << '\n';
	} else {
		for (std::map<std::string, std::string>::const_iterator it = request.headers.begin();
			 it != request.headers.end(); ++it) {
			oss << "  " << it->first << ": " << it->second << '\n';
		}
	}

	oss << "\nBODY:" << '\n';
	if (request.body.empty()) {
		oss << "  (empty)" << '\n';
	} else {
		oss << "  " << request.body << '\n';
	}

	httpReqLogger.debug(oss.str());
}
