#include "main.hpp"

std::string fakeHttpRes() {
	std::string html = "<!DOCTYPE HTML>\n"
					   "<html lang=\"en\">\n"
					   "<head>\n"
					   "<meta charset=\"utf-8\">\n"
					   "<style type=\"text/css\">:root { color-scheme: light dark; }</style>\n"
					   "<title>fake html page</title>\n"
					   "</head>\n"
					   "<body>\n"
					   "<h1>Big title</h1>\n"
					   "<p>small text here lorem ipsum sadfjhsadf</p>"
					   "</body>\n"
					   "</html>";

	std::ostringstream oss;
	// WARN end of temporary code
	oss << "HTTP/1.1 200 OK\r\n"
		<< "Server: webserv/1.0\r\n"
		<< "Content-Type: text/html; charset=utf-8\r\n"
		<< "Content-Length: " << html.length()
		<< "\r\n"
		// << "Connection: close\r\n"
		<< "\r\n" // The critical empty line
		<< html;

	return oss.str();
}
