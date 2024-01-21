#pragma once
#include "../min-cpp-version/cpp03.h"

#if __cplusplus < 201103L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201103L)
#error "This file requires compiler and library support for the ISO C++ 2011 standard."
#endif
