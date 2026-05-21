#pragma once

#include "../cfg/cfg.hpp"
#include <cerrno>
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

struct SocketMeta {
	int fd;
	std::string port;
	std::time_t lastEvent;
	std::string requestBuf;
	std::string responseBuf;
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
