#pragma once
#include "../min-cpp-version/cpp17.h"

#if __cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L)
#error "This file requires compiler and library support for the ISO C++ 2020 standard."
#endif

namespace ghassanpl
{
	template <class>
	concept always_false = false;

	template <class T, class... TYPES>
	concept is_any_of_v = (std::is_same_v<T, TYPES> || ...);
	
	template <class T, template <typename...> typename PRED>
	concept meets = PRED<T>::value;
}

#if __cplusplus > 202002L
#define GHPL_CONSTEXPR23 constexpr
#define GHPL_CONSTEVAL23 consteval
#else
#define GHPL_CONSTEXPR23 inline
#define GHPL_CONSTEVAL23 inline
#endif