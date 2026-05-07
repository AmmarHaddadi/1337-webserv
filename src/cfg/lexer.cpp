#include "cfg.hpp"
#include <fstream>
#include <sstream>

using namespace Config;

// @brief
// Skip whitespace
// Skip comments (# ... end of line)
// Recognize structure tokens: { }
// Recognize word tokens: sequences of non-whitespace characters that are not { or }
// Attach location: line/column (start position is enough)
std::vector<Config::KindVal> Lexer::tokenize(const std::string &filename) {
	std::vector<Config::KindVal> kvv;

	std::ifstream f(filename.c_str());
	if (!f.is_open())
		throw std::runtime_error("couldn't open " + filename);

	const std::string reservedChars = "{}#";
	std::string line;
	int lineNumber = 1;
	while (std::getline(f, line)) { // NOLINT

		for (size_t i = 0; i < line.size(); i++) {
			char c = line[i];

			if (std::isspace(static_cast<unsigned char>(c))) // NOLINT
				continue;

			// comment
			if (c == '#')
				break;

			KindVal kv;
			kv.col = i + 1;
			kv.line = lineNumber;

			if (c == '{') {
				kv.kind = OPEN_BRACE;
				kv.val = "{";
			}

			else if (c == '}') {
				kv.kind = CLOSE_BRACE;
				kv.val = "}";
			}

			else {
				while (i < line.size() &&
					   !std::isspace(static_cast<unsigned char>(line[i])) && // NOLINT
					   reservedChars.find(line[i]) == std::string::npos) {
					c = line[i];
					kv.val.push_back(c);
					i++;
				}

				if (kv.val.empty())
					continue;

				--i; // will be increased in the for loop
				kv.kind = WORD;
			}
			kvv.push_back(kv);
		}

		KindVal kvnl;
		kvnl.col = line.size();
		kvnl.line = lineNumber;
		kvnl.kind = NEW_LINE;
		kvnl.val = "NEW_LINE";
		kvv.push_back(kvnl);

		lineNumber++;
	}

	KindVal kveof;
	kveof.col = line.size();
	kveof.line = lineNumber;
	kveof.kind = EOF_TOKEN;
	kvv.push_back(kveof);

	return kvv;
}

std::string Lexer::pKind(const Config::Kind &e) {
	if (e == OPEN_BRACE)
		return "OPEN_BRACE";
	if (e == Config::CLOSE_BRACE)
		return "CLOSE_BRACE";
	if (e == Config::NEW_LINE)
		return "NEW_LINE";
	return "WORD";
}

std::string Lexer::debug(const KindVal &kv) {
	std::ostringstream oss;
	oss << kv.line << ":" << kv.col << " -> " << pKind(kv.kind) << " -> " << "`" << kv.val << "`";
	return oss.str();
}

// std::vector<std::string> serverDirectives;
// serverDirectives.push_back("root");
// serverDirectives.push_back("error_page");
// serverDirectives.push_back("max_body_sz");
// serverDirectives.push_back("handle");

// std::vector<std::string> routeDirectives;
// serverDirectives.push_back("methods");
// serverDirectives.push_back("default_file");
// serverDirectives.push_back("upload_enabled");
// serverDirectives.push_back("redirect");
// serverDirectives.push_back("file_server");
// serverDirectives.push_back("upload_enabled");
