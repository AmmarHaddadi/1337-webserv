#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <string>

using namespace std;
class Request {
private:
  std::string buffer;

public:
  Request(const std::string buf) {
    buffer = buf;
    std::cout << buffer << std::endl;
  };
  ~Request() {
    std::cout << "Deconstract" << std::endl;

  };
};

void RequestLine(std::string buf);

#endif