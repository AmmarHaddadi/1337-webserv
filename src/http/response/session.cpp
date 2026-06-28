#include "response.hpp"
#include <fstream>
#include <sstream>

using namespace http;

std::string http::generateSessionID() {
    unsigned char buf[16];
    std::ifstream file("/dev/urandom", std::ios::binary);

    file.read(reinterpret_cast<char *>(buf), 16);
    std::stringstream ss;
    for (size_t i =0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buf[i]);
    }
    return ss.str();
}
