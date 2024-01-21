#pragma once
#include "../min-cpp-version/cpp11.h"

#if __cplusplus < 201402L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201402L)
#error "This file requires compiler and library support for the ISO C++ 2014 standard."
#endif
