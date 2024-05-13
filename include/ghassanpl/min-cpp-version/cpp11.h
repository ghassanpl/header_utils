#pragma once
#include "../min-cpp-version/cpp03.h"

#if __cplusplus < 201103L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201103L)
#error "This file requires compiler and library support for the ISO C++ 2011 standard."
#endif

namespace ghassanpl
{
#if defined(__cpp_lib_type_identity) && __cpp_lib_type_identity >= 201806L
	using std::type_identity;
	using std::type_identity_t;
#else
	template <class T>
	struct type_identity { using type = T; };

	template <class T>
	using type_identity_t = typename type_identity<T>::type;
#endif

	template <typename T>
	constexpr T default_value() noexcept { return T{}; }
	template <>
	constexpr void default_value<void>() noexcept { }
}