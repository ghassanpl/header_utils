#pragma once
#include "../min-cpp-version/cpp20.h"

#if __cplusplus < 202302 && (!defined(_MSVC_LANG) || _MSVC_LANG < 202302)
#error "This file requires compiler and library support for the ISO C++ 2023 standard."
#endif

