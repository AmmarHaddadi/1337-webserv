#include "../cfg/cfg.hpp"
#include "../main.hpp"

int setNonblock(int listenerFd) {
	int flags = fcntl(listenerFd, F_GETFL, 0);
	if (flags == -1)
		return -1;

	if (fcntl(listenerFd, F_SETFL, flags | O_NONBLOCK) == -1) {
		return -1;
	}
	return 0;
}

// @brief setup main listener socket used to accept new connections
// @return -1 on error or the fd of listener
// NOTE "0.0.0.0" is * for IPV4, "127.0.0.1" = localhost
int setupListener(const std::string &host, const std::string &port) {
	// AF_INET = ipv4, SOCK_STREAM = TCP
	int listenerFd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenerFd == -1)
		return -1;
	if (setNonblock(listenerFd) == -1) {
		coreLogger.error("fatal error: failed to make listener socket non blocking");
		close(listenerFd);
		return -1;
	}

	// transforming host and port to machine form
	// getaddrinfo is used to generate the struct sockaddr
	struct addrinfo *res;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0)
		return -1;

	// This tells the OS “I know this port was recently used, let me reuse it anyway.”
	int opt = 1;
	setsockopt(listenerFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (bind(listenerFd, res->ai_addr, res->ai_addrlen) == -1) {
		coreLogger.error("Failed to bind socket to " + host + ":" + port);
		close(listenerFd);
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);
	if (listen(listenerFd, 3) == -1) {
		close(listenerFd);
		return -1;
	}
	return listenerFd;
}

int accpetNewSocket(int mainFd) {
#ifdef __linux__
	return accept4(mainFd, NULL, NULL, SOCK_NONBLOCK);
#else
	coreLogger.debug("macos detected");
	int newSocket = accept(mainFd, NULL, NULL);
	if (newSocket == -1) {
		coreLogger.warn("failed to accept new connection");
		return -1;
	}
	if (setNonblock(newSocket) == -1) {
		coreLogger.warn("failed to make socket non blocking");
		return -1;
	}
	return newSocket;
#endif
}

// @brief closes a scoket, removes is from scoketPFd vector and metadata array
void closeDelSocket(std::vector<pollfd> &sockets, size_t sIdx,
					std::map<int, struct SocketMeta> &socketsMeta) {
	std::vector<pollfd>::iterator sPFd =
		sockets.begin() + static_cast<std::vector<pollfd>::difference_type>(sIdx);
	close(sPFd->fd);
	socketsMeta.erase(sPFd->fd);
	sockets.erase(sPFd);
}

// @brief for each server, accept new clients connecting to its listener pollFD, andd add them to
// sockets vector and socketsMeta
void acceptNewClients(std::vector<pollfd> &sockets, std::vector<Config::ServerConfig> &servers,
					  std::map<int, struct SocketMeta> &socketsMeta) {
	// a sockaddr_in address may be used to determine client's port and ip, to be used for rate
	// limiting for example

	for (size_t p = 0; p < servers.size(); p++) {
		if ((sockets[p].revents & POLLIN) != 0) {
			int newSocketFd = accpetNewSocket(sockets[p].fd);
			if (newSocketFd == -1) {
				coreLogger.warn("failed to accpet connection: " +
								std::string(std::strerror(errno)));
			} else {
				pollfd npfd = {newSocketFd, POLLIN, 0};
				sockets.push_back(npfd);
				// metadata
				SocketMeta newSocketMeta;
				newSocketMeta.fd = newSocketFd;
				newSocketMeta.lastEvent = std::time(0);
				newSocketMeta.port = servers[p].port;
				socketsMeta.insert(std::make_pair(newSocketFd, newSocketMeta));
			}
		}
	}
}

// @brief takes references, treates incoming requests, if it's bad or incmplete it signals to skip
// till next loop if all good and a response was written it returns 0 so the res is sent
// @return 1 if should continue
int handleReq(std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
			  size_t &sIdx, std::vector<Config::ServerConfig> &servers) {
	pollfd &socketPFd = sockets[sIdx];
	struct SocketMeta &sMeta = socketsMeta[socketPFd.fd];

	char buf[1024];
	ssize_t totalRead = read(socketPFd.fd, buf, 1024);

	// HINT 0 read = disconnect
	if (totalRead == 0) {
		closeDelSocket(sockets, sIdx, socketsMeta);
		--sIdx;
	} else if (totalRead > 0) {
		sMeta.lastEvent = std::time(0);
		sMeta.requestBuf.append(buf, totalRead);

		HttpRequest req = parserHttp(sMeta.requestBuf);
		if (req.status == BAD_REQ) {
			closeDelSocket(sockets, sIdx, socketsMeta);
			return 1; // continue
		}
		if (req.status == INCOMPLETE) {
			return 1; // continue
		}
		if (req.status == COMPLETE) {
			sMeta.responseBuf = Utils::fakeHttpRes(servers, req);
			socketPFd.events = POLLOUT;
		}
		printHttpRequest(req);
	}
	return 0;
}

void handleRes(std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
			   size_t &sIdx) {
	pollfd &socketPFd = sockets[sIdx];
	struct SocketMeta &sMeta = socketsMeta[socketPFd.fd];

	std::string &resbuf = sMeta.responseBuf;
	if (!resbuf.empty()) {
		ssize_t sent = send(socketPFd.fd, resbuf.c_str(), resbuf.size(), 0);
		sMeta.lastEvent = std::time(0);
		resbuf.erase(0, sent);
	}
	if (resbuf.empty())
		socketPFd.events = POLLIN;
}
