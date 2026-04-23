#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
	// AF_INET = ipv4 SOCK_STREAM = tcp
	int listenerFd = socket(AF_INET, SOCK_STREAM, 0);

	// LISTENER
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // listen on all interfaces
	address.sin_port = htons(8080);		  // Port 8080

	if (bind(listenerFd, (struct sockaddr *)&address, sizeof(address)) == -1)
		std::cerr << "failed to bind socket to port" << std::endl;

	listen(listenerFd, 3);

	// accepting new connections
	int addrLen = sizeof(address);

	while (1) {
		int newSocket = accept(listenerFd, (struct sockaddr *)&address,
							   (socklen_t *)&addrLen);
		char buf[1024] = {0};
		read(newSocket, buf, 1024);
		std::cout << "Client says: " << buf << std::endl;

		const char *res = "HTTP/1.1 200 OK\r\nContent-Type: "
						  "text/plain\r\nContent-Length: 2\r\n\r\nOK";
		send(newSocket, res, strlen(res), 0);
		close(newSocket);
	}

	close(listenerFd);
}
