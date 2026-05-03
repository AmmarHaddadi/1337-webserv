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
	$(Q)printf "\033[33mChecking formatting...\033[0m\n"
	$(Q)echo $(SRC) $(HDR) | xargs clang-format --dry-run --Werror
	$(Q)printf "\033[32mFormatting is clean!\033[0m\n"

# Lint Check: Uses bear to generate the database, then runs tidy in parallel
# We use run-clang-tidy for speed.
lint:
	$(Q)printf "Linting...\n"
	$(Q)$(MAKE) fclean $(REDIR)
	$(Q)bear -- $(MAKE) all $(REDIR)
	$(Q)test -f compile_commands.json || (echo "ERROR: compile_commands.json not generated"; exit 1)
	$(Q)if run-clang-tidy -p . -header-filter='src/.*' -quiet -use-color=1 > lint_output.log 2>&1; then \
		echo "OK!"; rm -f lint_output.log; \
	else \
		echo "FAILED"; cat lint_output.log; rm -f lint_output.log; exit 1; \
	fi
	$(Q)rm -f compile_commands.json

# Check everything (Style + Logic)
check: format lint

.PHONY: all clean fclean re format lint check
