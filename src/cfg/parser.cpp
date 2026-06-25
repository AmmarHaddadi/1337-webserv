#include "../shared/utils.hpp"
#include "cfg.hpp"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace Config;

KindVal &Parser::peek(int offset) {
	if (m_pos + offset >= m_kvv.size())
		return m_kvv.back();
	return m_kvv[m_pos + offset];
}

// @brief returns current and advanced
KindVal Parser::advance() {
	KindVal &kv = peek();
	if (kv.kind != EOF_TOKEN)
		m_pos++;
	return kv;
}

// @brief if kind matches -> advance
bool Parser::match(const Kind &k) {
	if (peek().kind == k) {
		advance();
		return true;
	}
	return false;
}

KindVal Parser::expect(const Kind &k, const std::string &description) {
	KindVal &kv = peek();
	if (kv.kind == k)
		return advance();

	std::ostringstream finalMsg;
	finalMsg << "Config error at " << kv.line << ":" << kv.col << " expected " << description
			 << ", found: " << kv.val;

	throw std::runtime_error(finalMsg.str());
}

void Parser::skipNewLines() {
	while (match(NEW_LINE))
		;
}

// @brief parses the address from kv to sc
// throws on error
void Parser::parseAddress(const KindVal &kv, ServerConfig &sc) {
	std::vector<std::string> hostPort = Utils::split(kv.val, ':');
	if (hostPort.empty() || hostPort.size() != 2)
		throw std::runtime_error("bad <address>: " + kv.val);
	sc.host = hostPort[0].empty() ? "0.0.0.0" : hostPort[0]; //["host", "port"]
	if (!Utils::isAllNum(hostPort[1]))
		throw std::runtime_error("bad <address> port (" + hostPort[1] + ") must be all numbers");
	sc.port = hostPort[1];
}

void Parser::parseServerDirective(const KindVal &directive, ServerConfig &sc) {
	// std::ostringstream oss;
	// oss << directive.line << ":" << directive.col << "-> " << "`" << directive.val << "`" <<
	// "\n"; cfgLogger.debug("parsing: " + oss.str());

	if (directive.val == "root") {
		KindVal v = expect(WORD, "server root path");
		sc.root = v.val;
	} else if (directive.val == "error_page") {
		KindVal code = expect(WORD, "<code>");
		KindVal path = expect(WORD, "<path> to html file");
		int cd;
		std::istringstream iss(code.val);
		iss >> cd;
		sc.errorPages[cd] = path.val;
	} else if (directive.val == "max_body_sz") {
		KindVal sz = expect(WORD, "<kilobytes>");
		if (!Utils::isAllNum(sz.val))
			throw std::invalid_argument("bad max_body_sz");
		std::istringstream iss(sz.val);
		iss >> sc.maxBodySize;
	} else if (directive.val == "handle") {
		ServerConfig::RouteConfig rc(sc);
		KindVal path = expect(WORD, "<path>");
		rc.path = path.val;

		expect(OPEN_BRACE, "{");
		skipNewLines();

		while (peek().kind != CLOSE_BRACE && peek().kind != EOF_TOKEN) {
			KindVal rtDirective = expect(WORD, "<directive>");
			parseRouteDirective(rtDirective, rc);
			skipNewLines();
		}
		expect(CLOSE_BRACE, "}");

		sc.routes.push_back(rc);
	} else {
		std::ostringstream oss;
		oss << directive.line << ":" << directive.col << "-> " << "`" << directive.val << "`";
		throw std::runtime_error("unknown server directive: " + oss.str());
	}
	expect(NEW_LINE, "xnew line after directive");
}

void Parser::parseRouteDirective(const KindVal &directive, ServerConfig::RouteConfig &rc) {
	// cfgLogger.debug("parsing: " + directive.val);

	if (directive.val == "root") {
		KindVal path = expect(WORD, "route root path");
		rc.root = path.val;
	}

	else if (directive.val == "methods") {
		KindVal m = expect(WORD, "one route method at least");
		rc.allowedMethods.push_back(m.val);

		while (peek().kind == WORD) {
			rc.allowedMethods.push_back(peek().val);
			advance();
		}
	}

	else if (directive.val == "default_file") {
		if (std::find(rc.allowedMethods.begin(), rc.allowedMethods.end(), "GET") ==
			rc.allowedMethods.end()) {
			rc.allowedMethods.push_back("GET");
		}
		KindVal path = expect(WORD, "default file path");
		rc.default_file = path.val;
	}

	else if (directive.val == "upload_enabled") {
		if (std::find(rc.allowedMethods.begin(), rc.allowedMethods.end(), "POST") ==
			rc.allowedMethods.end()) {
			rc.allowedMethods.push_back("POST");
		}
		rc.uploadEnabled = true;
		// rc.uploadPath = expect(WORD, "").val; // deprecated in favor of route root for simplicity
	}

	else if (directive.val == "redirect") {
		rc.hasRedirect = true;
		KindVal code = expect(WORD, "a valid redirect code");
		std::istringstream ss(code.val);
		ss >> rc.redirectCode;
		KindVal path = expect(WORD, "<location>");
		rc.redirectLocation = path.val;
	}

	else if (directive.val == "file_server") {
		if (std::find(rc.allowedMethods.begin(), rc.allowedMethods.end(), "GET") ==
			rc.allowedMethods.end()) {
			rc.allowedMethods.push_back("GET");
		}
		rc.fileServer = true;
		if (peek().kind == WORD) {
			if (peek().val != "browse")
				expect(NEW_LINE, "browse"); // nasty line to throw useful error
			rc.fileBrowser = true;
			advance();
		}
	}

	else if (directive.val == "cgi") {
		KindVal xt = expect(WORD, "<extension>");
		KindVal runner = expect(WORD, "<path>");
		rc.cgi[xt.val] = runner.val;
	}

	else {
		throw std::runtime_error("unknown route directive: " + directive.val);
	}
	expect(NEW_LINE, "new line after directive");
}

std::vector<ServerConfig> Parser::parse() {

	std::vector<ServerConfig> scv;

	skipNewLines();
	while (peek().kind != EOF_TOKEN) {
		ServerConfig sc;

		KindVal kv = expect(WORD, "<address>");
		parseAddress(kv, sc);

		expect(OPEN_BRACE, "start of server block ({)");

		skipNewLines();
		while (peek().kind != EOF_TOKEN && peek().kind != CLOSE_BRACE) {
			kv = expect(WORD, "<directive>");
			parseServerDirective(kv, sc);
			skipNewLines();
		}

		expect(CLOSE_BRACE, "end of server block (})");
		scv.push_back(sc);
		skipNewLines();
	}

	return scv;
}
