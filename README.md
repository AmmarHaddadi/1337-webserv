# most important rule
hado ghir i9tira7at bash tb9a lkhdma smooth w mandy3osh lw9t f tkharbi9
-> u can break rules if it feels right
# quick info
- production branch is called **production**, send PRs to it
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

