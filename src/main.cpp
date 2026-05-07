#include "main.hpp"
#include "cfg/cfg.hpp"
#include <exception>

int main(int ac, char **av) {
	std::string cfgFilePath = ac > 2 ? av[1] : "server.conf";
	std::vector<Config::ServerConfig> scv;
	try {
		std::vector<Config::KindVal> tokens = Config::Lexer::tokenize(cfgFilePath);
		Config::Parser parser(tokens);
		scv = parser.parse();
		Config::Checker::check(scv);
	} catch (std::exception &e) {
		cfgLogger.error(e.what());
		return 1;
	}

	std::vector<pollfd> socketsPFd;
	for (size_t sIdx = 0; sIdx < scv.size(); ++sIdx) {
		int listenerFd = setupListener(scv[sIdx].host, scv[sIdx].port);
		if (listenerFd == -1) {
			return 1;
		}
		pollfd listenerPfd = {listenerFd, POLLIN, 0};
		socketsPFd.push_back(listenerPfd);
	}

	std::map<int, struct SocketMeta> socketsMeta;

	sockaddr_in address;
	int addrLen = sizeof(address);

	while (true) {
		if (poll(&socketsPFd[0], socketsPFd.size(), -1) == -1) {
			coreLogger.error("poll() failure: " + std::string(std::strerror(errno)));
			break;
		}

		// accept new connections + add to metadata map
		for (size_t p = 0; p < scv.size(); p++) {
			if ((socketsPFd[p].revents & POLLIN) != 0) {
				int newSocketFd =
					accpetNewSocket(socketsPFd[p].fd, reinterpret_cast<struct sockaddr *>(&address),
									reinterpret_cast<socklen_t *>(&addrLen));
				if (newSocketFd == -1) {
					coreLogger.warn("failed to accpet connection: " +
									std::string(std::strerror(errno)));
				} else {
					pollfd npfd = {newSocketFd, POLLIN, 0};
					socketsPFd.push_back(npfd);
					// metadata
					SocketMeta newSocketMeta;
					newSocketMeta.fd = newSocketFd;
					newSocketMeta.lastEvent = std::time(0);
					newSocketMeta.port = scv[p].port;
					socketsMeta.insert(std::make_pair(newSocketFd, newSocketMeta));
				}
			}
		}

		// existing connections
		for (size_t sIdx = scv.size(); sIdx < socketsPFd.size(); ++sIdx) {
			pollfd &socketPFd = socketsPFd[sIdx];
			struct SocketMeta &sMeta = socketsMeta[socketPFd.fd];

			// if client hang or has err close
			if ((socketPFd.revents & (POLLHUP | POLLERR)) != 0) {
				closeDelSocket(socketsPFd, sIdx, socketsMeta);
				--sIdx;
				continue;
			}
			if ((socketPFd.revents & POLLIN) != 0) {
				char buf[1024];
				ssize_t totalRead = read(socketPFd.fd, buf, 1024);
				// HINT 0 read = disconnect
				if (totalRead == 0) {
					closeDelSocket(socketsPFd, sIdx, socketsMeta);
					--sIdx;
				} else if (totalRead > 0) {
					sMeta.requestBuf.append(buf, totalRead);
					// TODO next code must only run if request buffer has a valid request
					// if (hasHttpReq(readBuffer)) {
					//	do http logic
					//  save in writeBuffer
					// 	remove req from req buffer
					//  change socket fd event to POLLOUT to be picked in next iteration NOT THIS
					//  ONE
					// }

					// WARN temporary code
					sMeta.requestBuf.clear();
					sMeta.responseBuf = Utils::fakeHttpRes();
					socketPFd.events = POLLOUT;
					// WARN end of temporary code
					sMeta.lastEvent = std::time(0);
				}
			} else if ((socketPFd.revents & POLLOUT) != 0) {
				std::string &resbuf = sMeta.responseBuf;
				if (!resbuf.empty()) {
					size_t sent = send(socketPFd.fd, resbuf.c_str(), resbuf.size(), 0);
					sMeta.lastEvent = std::time(0);
					resbuf.erase(0, sent);
				}
				if (resbuf.empty()) {
					socketPFd.events = POLLIN;
				}
			}
			// close stale connection no read no write
			else if (std::difftime(std::time(0), sMeta.lastEvent) > TCP_TIMEOUT) {
				closeDelSocket(socketsPFd, sIdx, socketsMeta);
				--sIdx;
			}
		}
	}

	// closing all listener and client sockets (probably unnecessary)
	for (size_t s = 0; s < socketsPFd.size(); s++)
		close(socketsPFd[s].fd);
}
