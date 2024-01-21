#pragma once
#include "../min-cpp-version/cpp14.h"

#if __cplusplus < 201703L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201703L)
#error "This file requires compiler and library support for the ISO C++ 2017 standard."
#endif

