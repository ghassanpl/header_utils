/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <version>

#if !defined(__cpp_lib_format)
#error "This library requires std::format"
#endif
#if !defined(__cpp_lib_source_location)
#error "This library requires std::source_location"
#endif

#ifndef ASSUMING_DEBUG
#ifdef NDEBUG
#define ASSUMING_DEBUG 0
#else
#define ASSUMING_DEBUG 1
#endif
#endif

#ifndef ASSUMING_INCLUDE_MAGIC_ENUM
#if __has_include(<magic_enum.hpp>)
#define ASSUMING_INCLUDE_MAGIC_ENUM 1
#else
#define ASSUMING_INCLUDE_MAGIC_ENUM 0
#endif
#endif

#include <format>
#include <source_location>

#if ASSUMING_INCLUDE_MAGIC_ENUM
#include <magic_enum.hpp>
#endif

#if !ASSUMING_DEBUG
#ifdef _MSC_VER
#define ASSUMING_ASSUME(cond) (__assume(cond), (::std::ignore = (cond)))
#elif defined(__GNUC__)
#define ASSUMING_ASSUME(cond) ((cond) ? static_cast<void>(0) : __builtin_unreachable())
#else
#define ASSUMING_ASSUME(cond) static_cast<void>((cond) ? 0 : 0)
#endif
#endif

#if ASSUMING_DEBUG

#define Assuming(exp, ...) { if (auto&& _assuming_exp_v = (exp); !_assuming_exp_v) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will evalute to true", { { #exp, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_exp_v)) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingNull(exp, ...) { if (auto _assuming_exp_v = (const void*)std::to_address(exp); _assuming_exp_v != nullptr) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will be null", { { #exp, std::format("{}", _assuming_exp_v) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingNotNull(exp, ...) { if (auto _assuming_exp_v = (const void*)std::to_address(exp); _assuming_exp_v == nullptr) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will not be null", { { #exp, std::format("{}", _assuming_exp_v) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingBinOp(a, b, op, text, ...) { auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!(_assuming_a_v op _assuming_b_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#a " will " text " " #b, { \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingEqual(a, b, ...) AssumingBinOp(a, b, ==, "be equal to", __VA_ARGS__)
#define AssumingNotEqual(a, b, ...) AssumingBinOp(a, b, !=, "not be equal to", __VA_ARGS__)
#define AssumingGreater(a, b, ...) AssumingBinOp(a, b, >, "be greater than", __VA_ARGS__)
#define AssumingLess(a, b, ...) AssumingBinOp(a, b, <, "be less than", __VA_ARGS__)
#define AssumingGreaterEqual(a, b, ...) AssumingBinOp(a, b, >=, "be greater or equal to", __VA_ARGS__)
#define AssumingLessEqual(a, b, ...) AssumingBinOp(a, b, <=, "be less or equal to", __VA_ARGS__)

#define AssumingZero(a, ...) AssumingBinOp(a, 0, ==, "be equal to", __VA_ARGS__)

#define AssumingEmpty(exp, ...) { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); !empty(_assuming_exp_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will be empty", { { "size of " #exp, std::format("{}", size(_assuming_exp_v)) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingNotEmpty(exp, ...) { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); empty(_assuming_exp_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will not be empty", { { "size of " #exp, std::format("{}", size(_assuming_exp_v)) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingNullOrEmpty(exp, ...) { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); !::ghassanpl::detail::IsNullOrEmpty(_assuming_exp_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will be null or empty", { { #exp, _assuming_exp_v ? std::format("'{}'", _assuming_exp_v) : "(null)" } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingNotNullOrEmpty(exp, ...) { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); ::ghassanpl::detail::IsNullOrEmpty(_assuming_exp_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#exp " will not be null or empty", { { #exp, _assuming_exp_v ? std::format("'{}'", _assuming_exp_v) : "(null)" } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingValidIndex(_index, _container, ...) { using std::size; auto&& _assuming_index = (_index); auto&& _assuming_container = (_container); const auto _assuming_container_size = size(_assuming_container); \
	if (!(_assuming_index >= 0 && size_t(_assuming_index) < _assuming_container_size)) [[unlikely]] { \
		::ghassanpl::ReportAssumptionFailure(#_index " will be a valid index to " #_container, { \
			{ #_index, std::format("{}", _assuming_index) }, \
			{  "size of " #_container, std::format("{}", _assuming_container_size) }, \
		}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } }

#define AssumingValidIterator(_iterator, _container, ...) { using std::end; auto&& _assuming_iterator = (_iterator); auto&& _assuming_container = (_container); const auto _assuming_end = end(_assuming_container); \
	if (_assuming_iterator == _assuming_end) [[unlikely]] { \
		::ghassanpl::ReportAssumptionFailure(#_iterator " will be a valid iterator to " #_container, {}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } }

#define AssumingBetween(v, a, b, ...) { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!(_assuming_v_v >= _assuming_a_v && _assuming_v_v < _assuming_b_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#v " will be between " #a " and " #b, { \
		{ #v, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_v_v)) }, \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }

#define AssumingBetweenInclusive(v, a, b, ...) { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!(_assuming_v_v >= _assuming_a_v && _assuming_v_v <= _assuming_b_v)) [[unlikely]] \
	::ghassanpl::ReportAssumptionFailure(#v " will be between " #a " and " #b " (inclusive)", { \
		{ #v, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_v_v)) }, \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); }
#else

#define Assuming(exp, ...) ASSUMING_ASSUME(!!(exp))
#define AssumingNull(exp, ...) ASSUMING_ASSUME(!((exp) != nullptr))
#define AssumingNotNull(exp, ...) ASSUMING_ASSUME(!((exp) == nullptr))
#define AssumingBinOp(a, b, op, text, ...) ASSUMING_ASSUME(((a) op (b)))
#define AssumingEqual(a, b, ...) AssumingBinOp(a, b, ==, "be equal to", __VA_ARGS__)
#define AssumingZero(a, ...) AssumingBinOp(a, 0, ==, "be equal to", __VA_ARGS__)
#define AssumingNotEqual(a, b, ...) AssumingBinOp(a, b, !=, "not be equal to", __VA_ARGS__)
#define AssumingGreater(a, b, ...) AssumingBinOp(a, b, >, "be greater than", __VA_ARGS__)
#define AssumingLess(a, b, ...) AssumingBinOp(a, b, <, "be less than", __VA_ARGS__)
#define AssumingGreaterEqual(a, b, ...) AssumingBinOp(a, b, >=, "be greater or equal to", __VA_ARGS__)
#define AssumingLessEqual(a, b, ...) AssumingBinOp(a, b, <=, "be less or equal to", __VA_ARGS__)
#define AssumingEmpty(exp, ...) { using std::empty; ASSUMING_ASSUME(empty(exp)); }
#define AssumingNotEmpty(exp, ...) { using std::empty; using std::size; ASSUMING_ASSUME(!empty(exp)); }
#define AssumingNullOrEmpty(exp, ...) { using std::empty; using std::size; ASSUMING_ASSUME(::ghassanpl::detail::IsNullOrEmpty(exp));  }
#define AssumingNotNullOrEmpty(exp, ...) { using std::empty; using std::size; ASSUMING_ASSUME(!::ghassanpl::detail::IsNullOrEmpty(exp)); }
#define AssumingValidIndex(_index, _container, ...) { using std::size; auto&& _assuming_index = (_index); ASSUMING_ASSUME(((_assuming_index) >= 0 && size_t(_assuming_index) < size(_container))); }
#define AssumingValidIterator(_iterator, _container, ...) { using std::end; auto&& _assuming_iterator = (_iterator); auto&& _assuming_container = (_container); const auto _assuming_end = end(_assuming_container); ASSUMING_ASSUME(!(_assuming_iterator == _assuming_end)); }
#define AssumingBetween(v, a, b, ...) { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); ASSUMING_ASSUME(_assuming_v_v >= _assuming_a_v && _assuming_v_v < _assuming_b_v); }
#define AssumingBetweenInclusive(v, a, b, ...) { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); ASSUMING_ASSUME(_assuming_v_v >= _assuming_a_v && _assuming_v_v <= _assuming_b_v); }

#endif

namespace ghassanpl
{
	namespace detail
	{
		inline bool IsNullOrEmpty(const char* str) { return str == nullptr || str[0] == 0; }
		template <typename T>
		inline bool IsNullOrEmpty(T&& str) { using std::empty; return empty(str); }

#if ASSUMING_DEBUG
		/*
		template <typename T>
		concept formattable = requires { typename std::formatter<T>; };
		*/

		template <typename T>
		concept formattable = requires { { std::formatter<T>{} }; };

		template <typename T>
		concept streamable = requires (T val, std::stringstream & ss) { { ss << val }; };

		template <typename T>
		decltype(auto) GetFormattable(T&& val)
		{
			using simple_type = std::remove_cvref_t<T>;
#if ASSUMING_INCLUDE_MAGIC_ENUM
			if constexpr (std::is_enum_v<simple_type>)
				return magic_enum::enum_name(val);
			else
#endif
			if constexpr (std::is_constructible_v<std::string_view, simple_type>)
				return std::forward<T>(val);
			else if constexpr (std::is_pointer_v<simple_type>)
				return (const void*)std::to_address(std::forward<T>(val));
			else if constexpr (streamable<simple_type> && !formattable<simple_type>)
			{
				std::stringstream ss;
				ss << std::forward<T>(val);
				return std::move(ss).str();
			}
			else if constexpr (!formattable<simple_type> && !streamable<simple_type>)
				return std::format("<{}>", typeid(simple_type).name());
			else
				return std::forward<T>(val);
		}

		inline std::string AdditionalDataToString() { return {}; }
		template <typename T, typename... ARGS>
		inline std::string AdditionalDataToString(T&& fmt, ARGS&&... args) {
			return std::vformat(std::forward<T>(fmt), std::make_format_args(GetFormattable(std::forward<ARGS>(args))...));
			//return std::format(std::forward<T>(fmt), std::forward<ARGS>(args)...);
		}

#endif
	}

	void ReportAssumptionFailure(std::string_view expectation, std::initializer_list<std::pair<std::string_view, std::string>> values, std::string data, std::source_location loc = std::source_location::current());
}