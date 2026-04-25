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
int setupListener(unsigned port) {
	int listenerFd = socket(AF_INET, SOCK_STREAM, 0);
	if (setNonblock(listenerFd) == -1) {
		std::cerr << "failed to make socket non blocking" << '\n';
		return -1;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces
	address.sin_port = htons(port);		  // Port 8080

	if (bind(listenerFd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) == -1) {
		std::cerr << "failed to bind socket to port" << '\n';
		return -1;
	}
	if (listen(listenerFd, 3) == -1)
		return -1;
	return listenerFd;
}

int accpetNewSocket(int mainFd, struct sockaddr *addr, socklen_t *addrLen) {
// biba linoks
#ifdef __linux__
	return accept4(mainFd, addr, addrLen, SOCK_NONBLOCK);
// fak l macos
#else
	coreLogger.debug("macos detected");
	int newSocket = accept(mainFd, addr, addrLen);
	if (newSocket == -1) {
		coreLogger.warn("failed to accept new connection");
		return -1;
	}
	if (setNonblock(newSocket) == -1) {
		coreLogger.warn("failed to make socket non blocking");
		return 1;
	}

	return newSocket;
#endif
}

// @brief closes a scoket, removes is from scoketPFd vector and metadata array, meta iterator
// decremenets to before current so loop re increments
// WARN SIDE EFFECT: sPFdIter decrements
void closeDelSocket(std::vector<pollfd> &socketsPFd, std::vector<pollfd>::iterator &sPFdIter,
					std::map<int, struct SocketMeta> &socketsMeta) {
	int fd = sPFdIter->fd;
	close(fd);
	socketsMeta.erase(fd);
	sPFdIter = socketsPFd.erase(sPFdIter); // returns next iterator
	--sPFdIter;							   // loop will increment again
}
