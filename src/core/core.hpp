#pragma once

#include "../cfg/cfg.hpp"
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <map>
#include <netdb.h>
#include <string>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <cerrno>

extern CoreLogger coreLogger;
struct SocketMeta {
	explicit SocketMeta(Config::ServerConfig &scRef)
		: lastEvent(std::time(0)), closeAfterResponse(true), server(scRef) {};
	// int fd; // can be found in the sockets vector type pollFd
	// std::string port;
	std::time_t lastEvent;
	std::string requestBuf;
	std::string responseBuf;
	bool closeAfterResponse;
	// TODO XXX has to be const in a way or other
	Config::ServerConfig &server; // parent server config
};

// functions
int setNonblock(int listenerFd);
int accpetNewSocket(int mainFd);
int setupListener(const std::string &host, const std::string &port);
void closeDelSocket(std::vector<pollfd> &sockets, size_t sIdx,
					std::map<int, struct SocketMeta> &socketsMeta);
void acceptNewClients(std::vector<pollfd> &sockets, std::vector<Config::ServerConfig> &servers,
					  std::map<int, struct SocketMeta> &socketsMeta);
int handleReq(std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
			  size_t &sIdx, std::vector<Config::ServerConfig> &servers);
void handleRes(std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
			   size_t &sIdx);
