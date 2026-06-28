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

extern CoreLogger coreLogger;
struct SocketMeta {
	explicit SocketMeta(Config::ServerConfig &scRef)
		: server(scRef), lastEvent(std::time(0)),
		  closeAfterResponse(true), isCgiPipe(false), cgiPipeFd(-1), clientFd(-1), cgiPid(-1) {}
	// int fd; // can be found in the sockets vector type pollFd
	// std::string port;
	Config::ServerConfig &server; // parent server config
	std::string requestBuf;
	std::string responseBuf;
	std::time_t lastEvent;
	bool closeAfterResponse;
	bool isCgiPipe;
	int cgiPipeFd;
	int clientFd;
	pid_t cgiPid;
	std::map<std::string, std::string> RespHeader;
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
			  std::map<std::string, std::string> &sessions, size_t &sIdx);
void handleRes(std::vector<pollfd> &sockets, std::map<int, struct SocketMeta> &socketsMeta,
			   size_t &sIdx);
