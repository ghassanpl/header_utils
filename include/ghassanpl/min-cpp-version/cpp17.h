#pragma once
#include "../min-cpp-version/cpp14.h"

#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
#error "This file requires compiler and library support for the ISO C++ 2017 standard."
#endif

#if !defined(__cpp_concepts)
#define GHPL_REQUIRES(...)
#define GHPL_TYPENAME(...) typename
#define GHPL_CONCEPT(name) constexpr inline bool name
#else
#define GHPL_REQUIRES(...) requires __VA_ARGS__
#define GHPL_TYPENAME(...) __VA_ARGS__
#define GHPL_CONCEPT(name) concept name
#endif
