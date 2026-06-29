*This project has been created as part of the 42 curriculum by [ ssallami [ ahaddadi, [ mbarhoun ]]].*

## Description

This project is about writing your own HTTP server in C++ 98. The primary function of this web server is to store, process, and deliver web pages to clients using the Hypertext Transfer Protocol (HTTP). It is capable of serving a fully static website, handling file uploads, and executing CGI (like PHP or Python) based on file extensions. The server remains non-blocking at all times and uses only 1 `poll()` (or an equivalent function like `select`, `kqueue`, or `epoll`) for all the I/O operations between the clients and the server.

## Instructions

### Compilation

A `Makefile` is provided to compile the source files without unnecessary relinking.
The code is compiled with `c++` using the mandatory flags `-Wall -Wextra -Werror`.
The Makefile contains the required rules: `$(NAME)`, `all`, `clean`, `fclean`, and `re`.

### Execution

The executable generated is named `webserv`.
You can run the server using the following command:
`./webserv [configuration file]`
The program relies on this configuration file (provided as an argument or found in a default path) to define ports, routes, default error pages, and directory rules.

## Resources

* **HTTP Protocol:** The HTTP 1.0 RFC was used as a primary reference point to understand the protocol's intricacies, client-server communication, and status codes.


* **Reference & Testing:** NGINX was used to compare headers and answer behaviours, while `telnet` and standard web browsers were used for testing.


* **AI Usage:** AI tools were utilized strictly to reduce repetitive tasks and assist in exploring technical concepts. All AI-generated content was critically assessed, systematically checked, and tested. Peer review was systematically sought to ensure full understanding of the generated logic and to prevent blind spots before integration.