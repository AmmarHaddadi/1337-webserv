#pragma once

#include "../shared/logger.hpp"
#include <cstddef>
#include <cstdlib>
#include <map>
#include <string>
#include <sys/socket.h>
#include <vector>

extern CfgLogger cfgLogger;

namespace Config {

class ServerConfig {
  public:
	ServerConfig() : maxBodySize(1024) {};

	class RouteConfig {
	  public:
		explicit RouteConfig(const ServerConfig &server)
			: root(server.root), default_file("index.html"), uploadEnabled(false),
			  hasRedirect(false), fileServer(false), fileBrowser(false) {};
		std::string path; // matcher, no regex, /a = /a/, longest wins
		std::string root;
		std::vector<std::string> allowedMethods;
		std::string default_file;

		bool uploadEnabled;
		std::string uploadPath;

		bool hasRedirect;
		int redirectCode;
		std::string redirectLocation;

		bool fileServer;
		bool fileBrowser;
	};

	std::string host;
	std::string port;
	std::string root;
	std::map<int, std::string> errorPages; // code : page path
	size_t maxBodySize;
	std::vector<RouteConfig> routes;
};

enum Kind { OPEN_BRACE, CLOSE_BRACE, WORD, NEW_LINE, EOF_TOKEN };

struct KindVal {
	Kind kind;
	std::string val;
	unsigned line;
	unsigned col;
};

namespace Lexer {

// @return vector of tokens, if fail throws exception
std::vector<KindVal> tokenize(const std::string &filename);

std::string debug(const KindVal &kv);
std::string pKind(const Config::Kind &e);

}; // namespace Lexer

class Parser {
  public:
	explicit Parser(std::vector<KindVal> &kvv) : m_pos(0), m_kvv(kvv) {};

	// @brief if fail throws exception
	std::vector<ServerConfig> parse();

	// helpers
	KindVal &peek(int offset = 0);
	KindVal advance();
	bool match(const Kind &k);
	KindVal expect(const Kind &k, const std::string &description);
	void skipNewLines();

	static void parseAddress(const KindVal &kv, ServerConfig &sc);
	void parseServerDirective(const KindVal &directive, ServerConfig &sc);
	void parseRouteDirective(const KindVal &directive, ServerConfig::RouteConfig &rc);

  private:
	unsigned m_pos;
	std::vector<KindVal> &m_kvv;
};

namespace Checker {
void check(const std::vector<ServerConfig> &scv);
} // namespace Checker

}; // namespace Config
