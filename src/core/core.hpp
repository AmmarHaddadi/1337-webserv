#pragma once

#include <ctime>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>

struct SocketMeta {
	int fd;
	std::time_t lastEvent;
	std::string requestBuf;
	std::string responseBuf;
};

// functions
int setNonblock(int listenerFd);
int accpetNewSocket(int mainFd, struct sockaddr *addr, socklen_t *addrLen);
int setupListener(unsigned port);
void closeDelSocket(std::vector<pollfd> &socketsPFd, std::vector<pollfd>::iterator &sPFdIter,
					std::map<int, struct SocketMeta> &socketsMeta);
