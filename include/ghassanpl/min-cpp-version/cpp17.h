#pragma once
#include "../min-cpp-version/cpp14.h"
#include <type_traits>

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

namespace ghassanpl
{
#if defined(__cpp_lib_remove_cvref) && __cpp_lib_remove_cvref >= 201711L
    using std::remove_cvref;
    using std::remove_cvref_t;
#else
    template <class T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template <class T>
    struct remove_cvref { using type = remove_cvref_t<T>; };
#endif
}