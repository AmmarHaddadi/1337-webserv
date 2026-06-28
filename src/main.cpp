#include "main.hpp"
#include "cfg/cfg.hpp"
#include "cgi/cgi.hpp"
#include "core/core.hpp"
#include "http/response/response.hpp"
#include <cerrno>
#include <exception>
#include <iostream>
#include <sys/ioctl.h>

namespace {
const size_t CGI_READ_BUFFER_SIZE = 2048;

size_t findSocketIndex(std::vector<pollfd> &sockets, int fd) {
	for (size_t i = 0; i < sockets.size(); ++i) {
		if (sockets[i].fd == fd)
			return i;
	}
	return sockets.size();
}

void handleCgiPipe(std::vector<pollfd> &sockets,
                   std::map<int, SocketMeta> &socketsMeta,
                   size_t &sIdx)
{
    pollfd &socket = sockets[sIdx];

    std::map<int, SocketMeta>::iterator metaIt = socketsMeta.find(socket.fd);
    if (metaIt == socketsMeta.end())
        return;

    SocketMeta &pipeMeta = metaIt->second;
    if (!pipeMeta.isCgiPipe)
        return;

    std::map<int, SocketMeta>::iterator clientIt =
        socketsMeta.find(pipeMeta.clientFd);

    SocketMeta &sMeta = metaIt->second;

    if (socket.revents & POLLERR)
    {
        if (clientIt != socketsMeta.end())
        {
            clientIt->second.cgiPipeFd = -1;
            clientIt->second.responseBuf = http::generateHttpResponse(
                http::INTERNAL_SERVER_ERROR,
                !clientIt->second.closeAfterResponse,
                http::generateErrorPage(sMeta.server, http::INTERNAL_SERVER_ERROR));

            size_t clientIdx = findSocketIndex(sockets, pipeMeta.clientFd);
            if (clientIdx < sockets.size())
                sockets[clientIdx].events = POLLOUT;
        }

        closeDelSocket(sockets, sIdx, socketsMeta);
        --sIdx;
        return;
    }

    if (socket.revents & POLLIN)
    {
        int available = 0;

        if (ioctl(socket.fd, FIONREAD, &available) == 0 && available > 0)
        {
            char buf[CGI_READ_BUFFER_SIZE];

            while (available > 0)
            {
                ssize_t toRead = available > (int)sizeof(buf)
                                 ? (ssize_t)sizeof(buf)
                                 : (ssize_t)available;

                ssize_t n = read(socket.fd, buf, toRead);

                if (n <= 0)
                    break;

                if (clientIt != socketsMeta.end())
                    clientIt->second.responseBuf.append(buf, n);

                available -= n;
            }
        }
    }

    if ((socket.revents & POLLHUP))
    {
        waitpid(pipeMeta.cgiPid, NULL, WNOHANG);

        if (clientIt != socketsMeta.end())
        {
            clientIt->second.cgiPipeFd = -1;
            clientIt->second.responseBuf = http::generateHttpResponse(
                http::OK,
                !clientIt->second.closeAfterResponse,
                clientIt->second.responseBuf);

            size_t clientIdx = findSocketIndex(sockets, pipeMeta.clientFd);
            if (clientIdx < sockets.size())
                sockets[clientIdx].events = POLLOUT;
        }

        closeDelSocket(sockets, sIdx, socketsMeta);
        --sIdx;
    }
}
} // namespace

int main(int ac, char **av) {
	srand(time(NULL));
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
#ifdef CFG_DEBUG
		for (size_t i = 0; i < servers.size(); ++i) {
			std::cout << "=== Server " << i << " ===\n";
			Config::debugServerConfig(servers[i]);
		}
#endif

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
	std::map<std::string, std::string> sessions;

	while (true) {
		if (poll(&sockets[0], sockets.size(), -1) == -1) {
			coreLogger.error("poll() failure: " + std::string(std::strerror(errno)));
			break;
		}

		acceptNewClients(sockets, servers, socketsMeta);

		for (size_t sIdx = servers.size(); sIdx < sockets.size(); ++sIdx) {
			pollfd &socket = sockets[sIdx];
			SocketMeta &sMeta = (socketsMeta.find(socket.fd))->second; // shouldn't fail

			if (sMeta.isCgiPipe) {
				handleCgiPipe(sockets, socketsMeta, sIdx);
				continue;
			}

			if ((socket.revents & (POLLHUP | POLLERR)) != 0) {
				closeDelSocket(sockets, sIdx, socketsMeta);
				--sIdx;
				continue;
			}

			if ((socket.revents & POLLIN) != 0) {
				if (handleReq(sockets, socketsMeta, sessions, sIdx) == 1)
					continue;
			}

			bool handled = false;
			if ((socket.revents & POLLOUT) != 0) {
				handleRes(sockets, socketsMeta, sIdx);
				handled = true;
			}
			// close stale connection
			if (!handled && sMeta.cgiPipeFd == -1 &&
				std::difftime(std::time(0), sMeta.lastEvent) > TCP_TIMEOUT) {
				closeDelSocket(sockets, sIdx, socketsMeta);
				--sIdx;
			}
		}
	}

	// closing all listener and client sockets (probably unnecessary)
	for (size_t s = 0; s < sockets.size(); s++)
		close(sockets[s].fd);
}
