NAME = webserv
CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -Wshadow -Wno-shadow
SRC = $(shell find src -type f -name "*.cpp")
OBJS = $(SRC:.cpp=.o)
HDR = $(shell find src -type f -name "*.hpp")
all: $(NAME)

$(NAME) : $(OBJS)
	$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)

%.o:%.cpp $(HDR)
	$(CPP) $(CPPFLAGS) $< -c -o $@

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re : fclean all

V ?= 0
ifeq ($(V),1)
  Q :=
  REDIR :=
else
  Q := @
  REDIR := > /dev/null 2>&1
endif

# checks code is formatted well else throw error
format:
	$(Q)echo "Checking formatting..."
	$(Q)echo $(SRC) $(HDR) | xargs clang-format --dry-run --Werror
	$(Q)echo "Formatting is clean"

lint:
	$(Q)echo "Linting...\n"
	$(Q)if clang-tidy $(SRC) -header-filter='src/.*' -quiet -use-color=1 -- $(CPPFLAGS) > lint_output.log 2>&1; then \
		echo "Linting Good"; rm -f lint_output.log; \
	else \
		echo "Linting Bad"; cat lint_output.log; rm -f lint_output.log; exit 1; \
	fi

# Check everything (compile , format & lint)
check: format lint

.PHONY: all clean fclean re format lint check
