#include "cfg.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace Config {

// @brief checks:
// 	1. exists
// 	2. is dir
// 	3. has read access
void checkRoot(std::string &root) {
	struct stat st;
	if (stat(root.c_str(), &st) != 0)
		throw std::runtime_error(
			"failed to access (does not exist or no permissions for parent dir or...)" + root);
	if ((st.st_mode & S_IFMT) != S_IFDIR) // man 7 inode
		throw std::runtime_error(root + " is not a directory and can't be used as root");
	if (access(root.c_str(), R_OK) != 0)
		throw std::runtime_error("no read access for " + root);
}

void checkErrorPages(std::string &serverRoot, std::map<int, std::string> &eps) {
	for (std::map<int, std::string>::iterator it = eps.begin(); it != eps.end(); it++) {
		std::ostringstream oss;
		oss << it->first;
		if (it->first >= 600 || it->first <= 0)
			throw std::runtime_error("bad error_page code " + oss.str());
		if (it->second.empty())
			throw std::runtime_error("bad error_page path for code " + oss.str());
		if (access((serverRoot + "/" + it->second).c_str(), R_OK) != 0)
			throw std::runtime_error("no read access for the error_page file " + it->second);
	}
}

void checkMaxBodySz(size_t sz) {
	// yeah error msgs are shit you're suppoed to know the line and which server etc blabla
	if (sz == 0)
		// throw std::runtime_error("request body size can't be 0");
		// the user knows best ¯\_(ツ)_/¯
		cfgLogger.warn("maximum request body size is 0");
	else if (sz > 102400)
		cfgLogger.warn("your max body size is above 100MB");
	else if (sz < 1024)
		cfgLogger.warn("your max body size is below the recommended 1MB");
}

// route checks

void checkRoutePath(std::string &s) {
	if (s[0] != '/')
		throw std::invalid_argument("bad route: " + s + ": routes must start with /");
}

void checkMethods(std::vector<std::string> &mtds) {
	for (size_t i = 0; i < mtds.size(); i++) {
		if (mtds[i] != "GET" && mtds[i] != "POST" && mtds[i] != "DELETE")
			throw std::invalid_argument(mtds[i] + " is not supported as a method");
	}
}

void checkUploadPath(std::string &up) {
	struct stat st;
	if (stat(up.c_str(), &st) != 0)
		throw std::runtime_error("bad upload path: failed to access " + up);
	if ((st.st_mode & S_IFMT) != S_IFDIR) // man 7 inode
		throw std::runtime_error(up + " is not a directory and can't be used as upload path");
	if (access(up.c_str(), W_OK) != 0)
		throw std::runtime_error("no write access for upload path " + up);
}

void checkRedirect(int redirectCode) {
	int allowedCodes[] = {301, 302, 307, 308};
	int *found = std::find(allowedCodes, allowedCodes + 4, redirectCode);
	if (found == allowedCodes + 4) // not found
		throw std::invalid_argument("bad redirection code");
}

// @brief throws if smtn feels
void Checker::check(const std::vector<ServerConfig> &scv) {
	if (scv.empty())
		throw std::runtime_error("config file has no servers");
	for (size_t i = 0; i < scv.size(); i++) {
		ServerConfig sc = scv[i];
		// bad host and port will fail
		checkRoot(sc.root);
		checkErrorPages(sc.root, sc.errorPages);
		checkMaxBodySz(sc.maxBodySize);

		// checking routes
		for (size_t rIdx = 0; rIdx < sc.routes.size(); rIdx++) {
			ServerConfig::RouteConfig rc = sc.routes[rIdx];

			checkRoutePath(rc.path);
			checkRoot(rc.root);
			checkMethods(rc.allowedMethods);
			if (rc.uploadEnabled)
				checkUploadPath(rc.uploadPath);
			if (rc.hasRedirect)
				checkRedirect(rc.redirectCode);
		}
	}
}
} // namespace Config
