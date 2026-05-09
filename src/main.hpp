#pragma once
#include "core/core.hpp"
#include "shared/logger.hpp"
#include "utils.hpp"
#include <cstddef>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include "./http_parser/Parser.hpp"
#include <cstdlib>

// how many seconds to keep a stale connection before killing it
#define TCP_TIMEOUT 5
// how many connection request to keep in queue
// for a real server this must be very high
#define CLIENT_QUEUE 10

// struct serverSettings {
// 	int tcpKeepAliveSeconds;
// };

// EXTERNS
extern CoreLogger coreLogger;
extern HttpReqLogger httpReqLogger;
