# most important rule
hado ghir i9tira7at bash tb9a lkhdma smooth w mandy3osh lw9t f tkharbi9
-> u can break rules if it feels right

# quick info
- main (production) branch is called **main**, send PRs to it
- **clangd** for formatting, **clangd-tidy** for linting (config files present)
- linux and macos are the targeted platforms
- Makefile compiles `*.cpp` found in `src/`

# notes
- use github issues whenever possible
- try not to block other's work
- your utils and code stays in ur own directory (core, http, cgi, config ...) uness needed it goes public in `src/utils.cpp`

# Git rules
- clean and meaningful commit messages/history
- use detailed PRs
- a PR must solve only one problem (or related group of problems)
- code only gets merged after everyone approves (with exceptions)
- if ur commit solves a github issue use #67 or what ever issue number to mark as done
- DO NOT CODE IN PROUCTION BRANCH

# code rules
- use **clang-tidy**
- don't push shit you don't understand
- use AI for boring tasks where u won't learn anything 
- **if necessary** use Doxygen style comments in function you know others will interact with (code li dyalk bo7dk debber rasek fih) don't over explain, only when needed and with needed params/funcs (assuming u use resonable names for funcs and vars)
  example:

```CPP
/**
  @brief Calculates the area of a rectangle.
  This function takes the dimensions of a rectangle and returns the 
  total surface area. It handles zero-value checks internally.
  @param width The horizontal length of the rectangle.
  @param height The vertical length of the rectangle.
  @return The calculated area as a double.
 */
double calculateArea(double width, double height) {
    return width * height;
}
```

# Logging
- each topic/category has it's own logger for ease of toggling on/off
- each logger has levels
- logger implementation and loggers can be found at `src/shared/logger.hpp`
- to declare= a new logger add the following: (replace Category with your category, e.g: http, cgi ...)
```CPP
#ifdef LOG_Category
typedef ActiveLogger CategoryLogger;
#else
typedef QuietLogger CategoryLogger;
#endif
```
- to use in code decalre a logger (preferably global scope) e.g:
```CPP
CoreLogger coreLogger("TCP", CoreLogger::WARN);
```
- you can use the logger in code, e.g:
```CPP
coreLogger.debug("msg");
coreLogger.info("msg");
coreLogger.warn("msg");
coreLogger.error("msg");
```

- to activate the logger make sure to build with `make CPPFLAGS+="-DLOG_Category"`
- you can activate multiple loggers `-DLOG_1 -DLOG_2` 
- you can change log level by chaiging it in logger constructor, using a high category will suppress lower ones; e.g: changing logger level to WARN will suppress info and debug logs (no need to comment them in code) 
  **DEBUG < INFO < WARN < ERROR**
