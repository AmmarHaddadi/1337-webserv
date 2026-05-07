#include "main.hpp"
#include "http_parser/Parser.hpp"

CoreLogger coreLogger("Core", CoreLogger::DEBUG);			   // NOLINT
HttpReqLogger httpReqLogger("Http Req", HttpReqLogger::DEBUG); // NOLINT

int main() {
	int listenerFd = setupListener(8080);
	if (listenerFd == -1) {
		return 1;
	}
	pollfd listenerPfd = {listenerFd, POLLIN, 0};

	std::map<int, struct SocketMeta> socketsMeta;
	std::vector<pollfd> socketsPFd;
	socketsPFd.push_back(listenerPfd);

	sockaddr_in address;
	int addrLen = sizeof(address);

	while (true) {
		if (poll(&socketsPFd[0], socketsPFd.size(), -1) == -1) {
			coreLogger.error("poll() failure: " + std::string(std::strerror(errno)));
			break;
		}

		// accept new connection + add to metadata map
		if ((socketsPFd[0].revents & POLLIN) != 0) {
			int newSocketFd =
				accpetNewSocket(listenerFd, reinterpret_cast<struct sockaddr *>(&address),
								reinterpret_cast<socklen_t *>(&addrLen));
			if (newSocketFd == -1) {
				coreLogger.warn("failed to accpet connection: " +
								std::string(std::strerror(errno)));
			} else {
				pollfd npfd = {newSocketFd, POLLIN, 0};
				socketsPFd.push_back(npfd);
				// metadata
				SocketMeta sMeta;
				sMeta.fd = newSocketFd;
				sMeta.lastEvent = std::time(0);
				socketsMeta.insert(std::make_pair(newSocketFd, sMeta));
			}
		}

		// existing connections
		for (std::vector<pollfd>::iterator sPFdIter = socketsPFd.begin() + 1;
			 sPFdIter != socketsPFd.end(); ++sPFdIter) {

			std::map<int, SocketMeta>::iterator metaIt = socketsMeta.find(sPFdIter->fd);
			struct SocketMeta &sMeta = metaIt->second;

			// if client hang or has err close
			if ((sPFdIter->revents & (POLLHUP | POLLERR)) != 0) {
				closeDelSocket(socketsPFd, sPFdIter, socketsMeta);
				continue;
			}
			if ((sPFdIter->revents & POLLIN) != 0) {
				char buf[1024];
				ssize_t totalRead = read(sPFdIter->fd, buf, 1024);
				// HINT 0 read = disconnect
				if (totalRead == 0) {
					closeDelSocket(socketsPFd, sPFdIter, socketsMeta);
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

					// HTTP PARSING
					
					HttpRequest req = parserHttp(sMeta.requestBuf);
					printHttpRequest(req);
					if (req.status == BAD_REQ) {
						closeDelSocket(socketsPFd, sPFdIter, socketsMeta);
						continue;
					}
					if (req.status == INCOMPLETE) {
						continue;
					}
					if (req.status == COMPLETE) {
						sMeta.responseBuf = fakeHttpRes();
						sPFdIter->events = POLLOUT;
					}
					printHttpRequest(req);

					// WARN end of temporary code
					sMeta.lastEvent = std::time(0);
				}
			} else if ((sPFdIter->revents & POLLOUT) != 0) {
				std::string &resbuf = sMeta.responseBuf;
				if (!resbuf.empty()) {
					size_t sent = send(sPFdIter->fd, resbuf.c_str(), resbuf.size(), 0);
					sMeta.lastEvent = std::time(0);
					resbuf.erase(0, sent);
				}
				if (resbuf.empty()) {
					sPFdIter->events = POLLIN;
				}
			}
			// close stale connection no read no write
			else if (std::difftime(std::time(0), sMeta.lastEvent) > TCP_TIMEOUT) {
				closeDelSocket(socketsPFd, sPFdIter, socketsMeta);
			}
		}
	}

	close(listenerFd);
}
