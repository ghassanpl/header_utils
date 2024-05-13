#pragma once
#include "../min-cpp-version/cpp17.h"

#if __cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L)
#error "This file requires compiler and library support for the ISO C++ 2020 standard."
#endif

namespace ghassanpl
{
	template <class>
	inline constexpr bool always_false = false;
}
