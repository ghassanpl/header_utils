# header_utils

A collection of header-only C++20 "libraries" that implement small but useful utilities.

**This is in-progress, live-at-head code that most likely has a lot of bugs, was not thoroughly tested, and requires full C++20 support**

**Pull requests welcome!**

### Note on portability

This code uses a lot of C++ features from all the standards, including C++23 (though that standard is mostly optional for most parts). Full C++20 support is expected for it to compile. Unfortunately this means that the only compiler than can properly compile everything at the moment is MSVC. Clang still has not fully implemented `from_chars`, for example. I have tried to make sure that everything compiles on at least MSVC and Clang, but a few features may not be available on non-MSVC compilers.

## [Documentation](https://ghassanpl.github.io/header_utils/topics.html)
