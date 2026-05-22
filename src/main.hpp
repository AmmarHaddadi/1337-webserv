#pragma once

// how many seconds to keep a stale connection before killing it
#define TCP_TIMEOUT 20
// how many connection request to keep in queue
// for a real server this must be very high
#define CLIENT_QUEUE 10

// struct serverSettings {
// 	int tcpKeepAliveSeconds;
// };

// EXTERNS
// extern HttpReqLogger httpReqLogger;
