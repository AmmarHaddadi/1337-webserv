#include "../../cfg/cfg.hpp"
#include "../../cgi/cgi.hpp"
#include "../../core/core.hpp"
#include "../http.hpp"
#include "response.hpp"
#include <fcntl.h>
#include <map>

using namespace http;

bool http::isCgi(Config::ServerConfig::RouteConfig &rc, SocketMeta &sMeta, HttpRequest &req,
				 std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
				 int clientFd, std::string &resolvedPath) {
	size_t pos = req.path.rfind('.');
	if (pos != std::string::npos) {
		std::string ext = req.path.substr(pos + 1);
		std::map<std::string, std::string>::iterator it = rc.cgi.find(ext);
		if (it == rc.cgi.end())
			return false;
		CGI::Cgi cgi(rc.cgi, req);
		try {
			int outFd = -1;
			int inFd = -1;
			pid_t pid = cgi.executeCGI(resolvedPath, outFd, inFd);
			if (!req.body.empty()) {
				size_t totalWritten = 0;
				while (totalWritten < req.body.size()) {
					ssize_t written = write(inFd, req.body.c_str() + totalWritten,
											req.body.size() - totalWritten);
					if (written > 0) {
						totalWritten += static_cast<size_t>(written);
						continue;
					}
					if (written < 0)
						continue;
					if (outFd == inFd) {
						close(outFd);
					} else {
						close(outFd);
						close(inFd);
					}
					waitpid(pid, NULL, WNOHANG);
					sMeta.responseBuf =
						generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
											 generateErrorPage(INTERNAL_SERVER_ERROR));
					return true;
				}
			}
			close(inFd);
			int socketFlags = fcntl(outFd, F_GETFL, 0);
			if (socketFlags != -1)
				fcntl(outFd, F_SETFL, socketFlags | O_NONBLOCK);
			pollfd cgiPfd = {outFd, POLLIN, 0};
			sockets.push_back(cgiPfd);
			SocketMeta pipeMeta(sMeta.server);
			pipeMeta.isCgiPipe = true;
			pipeMeta.clientFd = clientFd;
			pipeMeta.cgiPid = pid;
			socketsMeta.insert(std::make_pair(outFd, pipeMeta));
			sMeta.cgiPipeFd = outFd;
		} catch (const std::exception &e) {
			sMeta.responseBuf = generateHttpResponse(INTERNAL_SERVER_ERROR, req.keepAlive,
													 generateErrorPage(INTERNAL_SERVER_ERROR));
		}
		return true;
	}
	return false;
}