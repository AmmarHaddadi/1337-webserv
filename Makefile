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

# checks code is formatted well else throw error
format:
	@printf "\033[33mChecking formatting...\033[0m\n"
	@echo $(SRC) $(HDR) | xargs clang-format --dry-run --Werror
	@printf "\033[32mFormatting is clean!\033[0m\n"

# We use run-clang-tidy for speed.
lint:
	@printf "$(YELLOW)Linting...$(RESET) "
	@# Run clang-tidy directly on all source files.
	@# Everything after '--' are the compiler flags needed to parse the files.
	@if clang-tidy $(SRC) $(HDR) -header-filter='src/.*' -quiet -use-color=1 -- $(CPPFLAGS) > lint_output.log 2>&1; then \
		printf "$(GREEN)OK!$(RESET)\n"; \
		rm -f lint_output.log; \
	else \
		printf "$(RED)FAILED$(RESET)\n"; \
		cat lint_output.log; \
		rm -f lint_output.log; \
		exit 1; \
	fi

# Check everything (Style + Logic)
check: format lint

.PHONY: all clean fclean re format lint check
