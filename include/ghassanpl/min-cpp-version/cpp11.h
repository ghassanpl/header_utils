#pragma once
#include "../min-cpp-version/cpp03.h"

#if __cplusplus < 201103L && (!defined(_MSVC_LANG) || _MSVC_LANG < 201103L)
#error "This file requires compiler and library support for the ISO C++ 2011 standard."
#endif

namespace ghassanpl
{
	template <class T>
	struct type_identity { using type = T; };

	template <class T>
	using type_identity_t = typename type_identity<T>::type;
}