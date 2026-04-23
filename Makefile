NAME = webserv
CPP = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Wshadow -Wno-shadow
SRC = $(shell find src -type f -name "*.cpp")
OBJS = $(SRC:.cpp=.o)
HDR = $(shell find src -type f -name "*.cpp")
all: $(NAME)

$(NAME) : $(OBJS)
	$(CPP) $(CXXFLAGS) $(OBJS) -o $(NAME)

%.o:%.cpp $(HDR)
	$(CPP) $(CXXFLAGS) $< -c -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re : fclean all
