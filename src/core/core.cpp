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
		std::cerr << "failed to make socket non blocking" << '\n';
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
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);
	if (listen(listenerFd, 3) == -1)
		return -1;
	return listenerFd;
}

int accpetNewSocket(int mainFd, struct sockaddr *addr, socklen_t *addrLen) {
#ifdef __linux__
	return accept4(mainFd, addr, addrLen, SOCK_NONBLOCK);
#else
	coreLogger.debug("macos detected");
	int newSocket = accept(mainFd, addr, addrLen);
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
void closeDelSocket(std::vector<pollfd> &socketsPFd, size_t sIdx,
					std::map<int, struct SocketMeta> &socketsMeta) {
	std::vector<pollfd>::iterator sPFd =
		socketsPFd.begin() + static_cast<std::vector<pollfd>::difference_type>(sIdx);
	close(sPFd->fd);
	socketsMeta.erase(sPFd->fd);
	socketsPFd.erase(sPFd);
}
