#include "main.hpp"
#include "cfg/cfg.hpp"
#include "cgi/Cgi.hpp"
#include "core/core.hpp"
#include "cgi/Cgi.hpp"
#include <exception>
#include <iostream>

int main(int ac, char **av) {
	if (ac > 2) {
		std::cerr << "Error: only supported param is an optional <path/to/file.conf> \n";
		return 1;
	}
	std::string cfgFilePath = ac > 1 ? av[1] : "server.conf";
	std::vector<Config::ServerConfig> servers;
	try {
		std::vector<Config::KindVal> tokens = Config::Lexer::tokenize(cfgFilePath);
		Config::Parser parser(tokens);
		servers = parser.parse();
		Config::Checker::check(servers);
	} catch (std::exception &e) {
		cfgLogger.error(e.what());
		return 1;
	}

	// sockets has the serves at first, then the clients
	std::vector<pollfd> sockets;
	for (size_t scvIdx = 0; scvIdx < servers.size(); ++scvIdx) {
		int listenerFd = setupListener(servers[scvIdx].host, servers[scvIdx].port);
		if (listenerFd == -1)
			return 1;
		pollfd listenerPfd = {listenerFd, POLLIN, 0};
		sockets.push_back(listenerPfd);
	}

	std::map<int, struct SocketMeta> socketsMeta;

	while (true) {
		if (poll(&sockets[0], sockets.size(), -1) == -1) {
			coreLogger.error("poll() failure: " + std::string(std::strerror(errno)));
			break;
		}

		acceptNewClients(sockets, servers, socketsMeta);

		for (size_t sIdx = servers.size(); sIdx < sockets.size(); ++sIdx) {
			pollfd &socket = sockets[sIdx];
			SocketMeta &sMeta = (socketsMeta.find(socket.fd))->second; // shouldn't fail

			if ((socket.revents & (POLLHUP | POLLERR)) != 0) {
				closeDelSocket(sockets, sIdx, socketsMeta);
				--sIdx;
				continue;
			}

			if ((socket.revents & POLLIN) != 0) {
				if (handleReq(sockets, socketsMeta, sIdx) == 1)
					continue;

			} else if ((socket.revents & POLLOUT) != 0) {
				handleRes(sockets, socketsMeta, sIdx);
			}
			// close stale connection
			else if (std::difftime(std::time(0), sMeta.lastEvent) > TCP_TIMEOUT) {
				closeDelSocket(sockets, sIdx, socketsMeta);
				--sIdx;
			}
		}
	}

	// closing all listener and client sockets (probably unnecessary)
	for (size_t s = 0; s < sockets.size(); s++)
		close(sockets[s].fd);
}
