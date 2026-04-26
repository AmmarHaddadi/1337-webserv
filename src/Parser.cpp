#include "../header/Parser.hpp"
#include <cstddef>

void Parser(std::string buf) {


	std::cout << buf << std::endl;
	std::cout << "==========================" << std::endl;
    
	size_t i = 0;
	size_t t = 0;
    
	while (t < 3) {
        
        size_t j = buf.find("\r\n", i);
        std::cout << "i : " << i << "<=> j : " << j << std::endl;

		std::string line = buf.substr(i, j);
		std::cout << "LINE: ===> " << line << std::endl;
		i = j + 1;
        t++;
	}
}