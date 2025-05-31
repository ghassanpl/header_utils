/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <version>
#include <cassert>

#if !defined(__cpp_lib_format)
#error "This library requires std::format"
#endif
#include "source_location.h"

/// \defgroup Assuming Assuming
/// An in-my-humble-opinion better version of the `assert` concept.
/// 
/// Pros:
/// * The macro names "Assuming*" suggest that in the following code we are **assuming** the predicate given to the macro.
/// * The predicate and its parameters are always evaluated exactly once.
/// * A custom, user-provided "assumption failure" function is called on assumption failure.
/// * You can provide additional descriptions and arguments to the assumption failure function, for better debugging.
/// * Includes variants like `AssumingEqual(a, b, ...)` that assumes arguments are equal; each such variant makes sure to evaluate and stringify the arguments, and give a helpful message, such as: "Assumption Failed: Assuming that a will equal b."
/// * In non-debug compiles, the library instructs the compiler to actually ASSUME its predicate, so it can optimize the code better; keep in mind that the assumption failing is undefined behavior in non-debug compiles.
/// * Better macros to control the behavior of this library.
/// 
/// Cons:
/// * Depends on `<format>` and `<source_location>` headers.
/// * Still macro-based (only for stringification of arguments, thankfully.)
/// * Using compiler-specific code for enforcing assumptions.
/// * Slightly non-idiomatic naming conventions as well as some tiny assumptions about the user code-base.

/// \ingroup Assuming
///@{

/// \hideinitializer
#ifndef ASSUMING_DEBUG
#ifdef NDEBUG
#define ASSUMING_DEBUG 0
#else
#define ASSUMING_DEBUG 1
#endif
#endif

/// \hideinitializer
#ifndef ASSUMING_INCLUDE_MAGIC_ENUM
#if __has_include(<magic_enum_format.hpp>)
#define ASSUMING_INCLUDE_MAGIC_ENUM 1
#else
#define ASSUMING_INCLUDE_MAGIC_ENUM 0
#endif
#endif

#include <format>
#if __has_include(<debugging>)
#include <debugging>
#define ASSUMING_BREAKPOINT std::breakpoint
#else
#ifndef __has_builtin         // Optional of course.
#define __has_builtin(x) 0  // Compatibility with non-clang compilers.
#endif
#if __has_builtin(__builtin_debugtrap)
#define ASSUMING_BREAKPOINT   __builtin_debugtrap
#elif (defined(__clang__) || defined(__GNUC__)) && !defined(_MSC_VER)
#define ASSUMING_BREAKPOINT   __builtin_trap
#elif defined(_MSC_VER)
#define ASSUMING_BREAKPOINT  __debugbreak
#endif
#endif

#if ASSUMING_DEBUG
#include <sstream>
#endif

#ifndef ASSUMING_USE_STACKTRACE
#if defined(__cpp_lib_stacktrace)
#define ASSUMING_USE_STACKTRACE 1
#include <stacktrace>
#else
#define ASSUMING_USE_STACKTRACE 0
#endif
#endif

#if ASSUMING_INCLUDE_MAGIC_ENUM
#include <magic_enum/magic_enum_format.hpp>
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
/// \private
#define GHPL_UNREACHABLE() std::unreachable()
#else
#if (defined(__clang__) || defined(__GNUC__)) && !defined(_MSC_VER)
#define GHPL_UNREACHABLE() (__builtin_unreachable())
#else
#define GHPL_UNREACHABLE() GHPL_ASSUME(false)
#endif
#endif

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(assume)
/// \private
#define GHPL_ASSUME(...) [[assume(__VA_ARGS__)]]; (::std::ignore = (__VA_ARGS__))
#endif
#endif
#ifndef GHPL_ASSUME
#if (defined(__clang__) || defined(__GNUC__)) && !defined(_MSC_VER)
#define GHPL_ASSUME(...) ((__VA_ARGS__) ? static_cast<void>(0) : GHPL_UNREACHABLE())
#else
#define GHPL_ASSUME(...) (__assume(__VA_ARGS__), (::std::ignore = (__VA_ARGS__)))
#endif
#endif

#if ASSUMING_DEBUG || defined(DOXYGEN)

#define ASSUMING_HANDLE_HANDLER_RESULT(result) do { \
	switch (result) \
	{ \
		case ::ghassanpl::AssumptionHandlerResult::Continue: break; \
		case ::ghassanpl::AssumptionHandlerResult::Break: ASSUMING_BREAKPOINT(); break; \
		case ::ghassanpl::AssumptionHandlerResult::Terminate: std::terminate(); break; \
	} \
	} while (false)

#define ASSUMING_REPORT(...) ASSUMING_HANDLE_HANDLER_RESULT(::ghassanpl::ReportAssumptionFailure(__VA_ARGS__))

/// The basic Assuming macro. Assumes the term is true. Expression result must be convertible to bool.
#define Assuming(exp, ...) do { if (auto&& _assuming_exp_v = (exp); !_assuming_exp_v) [[unlikely]] \
	ASSUMING_REPORT(#exp " will evalute to true", { { #exp, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_exp_v)) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the point in code is not reachable.
#define AssumingNotReachable(...) do { ASSUMING_REPORT("execution will never reach this point", {}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); GHPL_UNREACHABLE(); } while (false)

/// Assumes the point in code is not reached via a recursive function call.
#define AssumingNotRecursive(...) \
	static int _assuming_recursion_counter##__LINE__ = 0; \
	if (_assuming_recursion_counter##__LINE__ != 0)  \
		ASSUMING_REPORT("enclosing block will not be entered recursively", {}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); \
	const ::ghassanpl::detail::RecursionScopeMarker _assuming_scope_marker##__LINE__( _assuming_recursion_counter##__LINE__ )

/// Assumes the point in code executes in exactly one thread, the same thread over the lifetime of the program.
#define AssumingSingleThread(...) do { \
		static std::thread::id _assuming_thread_id = std::this_thread::get_id(); \
		auto _assuming_current_thread_id = std::this_thread::get_id(); \
		if (_assuming_thread_id != _assuming_current_thread_id)\
			ASSUMING_REPORT("this code will be executed in one thread only", { {"required_thread_id", std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_thread_id))}, {"thread_id", std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_current_thread_id))} }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); \
	} while (false)

/// Assumes the point in code executes on the specified thread
#define AssumingOnThread(thread_to_check, ...) do { \
		auto _assuming_thread_id = (thread_to_check); \
		auto _assuming_current_thread_id = std::this_thread::get_id(); \
		if (_assuming_thread_id != _assuming_current_thread_id)\
			ASSUMING_REPORT("this code will be executed in one thread only", { {"required_thread_id", std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_thread_id))}, { "thread_id", std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_current_thread_id))} }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); \
	} while (false)

/// Assumes the point in code DOES NOT execute on the specified thread
#define AssumingNotOnThread(thread_to_check, ...) do { \
		auto _assuming_thread_id = (thread_to_check); \
		auto _assuming_current_thread_id = std::this_thread::get_id(); \
		if (_assuming_thread_id == _assuming_current_thread_id)\
			ASSUMING_REPORT("this code will not be executed in specific thread", { {"forbidden_thread_id", std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_thread_id))}, { "thread_id", std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_current_thread_id))} }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); \
	} while (false)

/// Assumes the term is not (convertible to) a null pointer.
#define AssumingNull(exp, ...) do { if (auto _assuming_exp_v = (const void*)std::to_address(exp); _assuming_exp_v != nullptr) [[unlikely]] \
	ASSUMING_REPORT(#exp " will be null", { { #exp, std::format("{}", _assuming_exp_v) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the term is (convertible to) a null pointer.
#define AssumingNotNull(exp, ...) do { if (auto _assuming_exp_v = (const void*)std::to_address(exp); _assuming_exp_v == nullptr) [[unlikely]] \
	ASSUMING_REPORT(#exp " will not be null", { { #exp, std::format("{}", _assuming_exp_v) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the two expressions `a` and `b` are true with regards to the relation `op` (describen in `text`). This is an internal macro used by others.
#define AssumingBinOp(a, b, op, text, ...) do { auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!(_assuming_a_v op _assuming_b_v)) [[unlikely]] \
	ASSUMING_REPORT(#a " will " text " " #b, { \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the two expressions evaluate equal.
#define AssumingEqual(a, b, ...) AssumingBinOp(a, b, ==, "be equal to", __VA_ARGS__)
/// Assumes the two expressions do not evaluate equal.
#define AssumingNotEqual(a, b, ...) AssumingBinOp(a, b, !=, "not be equal to", __VA_ARGS__)
/// Assumes the first term is greater than the second.
#define AssumingGreater(a, b, ...) AssumingBinOp(a, b, >, "be greater than", __VA_ARGS__)
/// Assumes the first term is less than the second.
#define AssumingLess(a, b, ...) AssumingBinOp(a, b, <, "be less than", __VA_ARGS__)
/// Assumes the first term is greater than or equal to the second.
#define AssumingGreaterEqual(a, b, ...) AssumingBinOp(a, b, >=, "be greater or equal to", __VA_ARGS__)
/// Assumes the first term is less than or equal to the second.
#define AssumingLessEqual(a, b, ...) AssumingBinOp(a, b, <=, "be less or equal to", __VA_ARGS__)

/// Assumes the first term contains the bits in the second term.
#define AssumingContainsBits(a, b, ...) do { auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!((_assuming_a_v & _assuming_b_v) == _assuming_b_v)) [[unlikely]] \
	ASSUMING_REPORT(#a " will contain flags " #b, { \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the term evaluates to 0.
#define AssumingZero(a, ...) AssumingBinOp(a, 0, ==, "be equal to", __VA_ARGS__)

/// Assumes the term evaluates to an empty container (tested via `empty(container)`)
#define AssumingEmpty(exp, ...) do { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); !empty(_assuming_exp_v)) [[unlikely]] \
	ASSUMING_REPORT(#exp " will be empty", { { "size of " #exp, std::format("{}", size(_assuming_exp_v)) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the term evaluates to an non-empty container (tested via `empty(container)`)
#define AssumingNotEmpty(exp, ...) do { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); empty(_assuming_exp_v)) [[unlikely]] \
	ASSUMING_REPORT(#exp " will not be empty", { { "size of " #exp, std::format("{}", size(_assuming_exp_v)) } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the term evaluates to either a null value or an empty string
#define AssumingNullOrEmpty(exp, ...) do { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); !::ghassanpl::detail::IsNullOrEmpty(_assuming_exp_v)) [[unlikely]] \
	ASSUMING_REPORT(#exp " will be null or empty", { { #exp, _assuming_exp_v ? std::format("'{}'", _assuming_exp_v) : "(null)" } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the term does not evaluate to neither a null value nor an empty string
#define AssumingNotNullOrEmpty(exp, ...) do { using std::empty; using std::size; if (auto&& _assuming_exp_v = (exp); ::ghassanpl::detail::IsNullOrEmpty(_assuming_exp_v)) [[unlikely]] \
	ASSUMING_REPORT(#exp " will not be null or empty", { { #exp, _assuming_exp_v ? std::format("'{}'", _assuming_exp_v) : "(null)" } }, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the `_index` term evaluates to a valid index to the `_container` term. This is checked via `size(_container)`
#define AssumingValidIndex(_index, _container, ...) do { using std::size; auto&& _assuming_index = (_index); auto&& _assuming_container = (_container); const auto _assuming_container_size = size(_assuming_container); \
	if (!(_assuming_index >= 0 && size_t(_assuming_index) < _assuming_container_size)) [[unlikely]] { \
		ASSUMING_REPORT(#_index " will be a valid index to " #_container, { \
			{ #_index, std::format("{}", _assuming_index) }, \
			{  "size of " #_container, std::format("{}", _assuming_container_size) }, \
		}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } } while (false)

/// Assumes the `_index` term evaluates to a valid iterator to the `_container` term. This is checked via `end(_container)`
#define AssumingValidIterator(_iterator, _container, ...) do { using std::end; auto&& _assuming_iterator = (_iterator); auto&& _assuming_container = (_container); const auto _assuming_end = end(_assuming_container); \
	if (_assuming_iterator == _assuming_end) [[unlikely]] { \
		ASSUMING_REPORT(#_iterator " will be a valid iterator to " #_container, {}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } } while (false)

/// Assumes the `v` term evaluates to a value between `a` and `b` exclusive.
#define AssumingBetween(v, a, b, ...) do { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!(_assuming_v_v >= _assuming_a_v && _assuming_v_v < _assuming_b_v)) [[unlikely]] \
	ASSUMING_REPORT(#v " will be between " #a " and " #b, { \
		{ #v, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_v_v)) }, \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)

/// Assumes the `v` term evaluates to a value between `a` and `b` inclusive.
#define AssumingBetweenInclusive(v, a, b, ...) do { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); if (!(_assuming_v_v >= _assuming_a_v && _assuming_v_v <= _assuming_b_v)) [[unlikely]] \
	ASSUMING_REPORT(#v " will be between " #a " and " #b " (inclusive)", { \
		{ #v, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_v_v)) }, \
		{ #a, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_a_v)) }, \
		{ #b, std::format("{}", ::ghassanpl::detail::GetFormattable(_assuming_b_v)) } \
	}, ::ghassanpl::detail::AdditionalDataToString(__VA_ARGS__)); } while (false)
#else

#define Assuming(exp, ...) GHPL_ASSUME(!!(exp))
#define AssumingNotReachable(...) GHPL_ASSUME(false)

// NOTE: TODO: The three macros below have a significant codegen overhead, should we even try their assumptions? I don't think the compiler can infer any useful information from them...
#define AssumingNotRecursive(...) \
	static int _assuming_recursion_counter##__LINE__ = 0; \
	GHPL_ASSUME(_assuming_recursion_counter##__LINE__ == 0); \
	const ::ghassanpl::detail::RecursionScopeMarker _assuming_scope_marker##__LINE__( _assuming_recursion_counter##__LINE__ )
#define AssumingSingleThread(...) do { \
		static std::thread::id _assuming_thread_id = std::this_thread::get_id(); \
		auto _assuming_current_thread_id = std::this_thread::get_id(); \
		GHPL_ASSUME(_assuming_thread_id == _assuming_current_thread_id); \
	} while (false)
#define AssumingOnThread(thread_to_check, ...) { \
		auto _assuming_thread_id = (thread_to_check); \
		auto _assuming_current_thread_id = std::this_thread::get_id(); \
		GHPL_ASSUME(_assuming_thread_id == _assuming_current_thread_id); \
	} while (false)


#define AssumingNull(exp, ...) GHPL_ASSUME(!((const void*)std::to_address(exp) != nullptr))
#define AssumingNotNull(exp, ...) GHPL_ASSUME(!((const void*)std::to_address(exp) == nullptr))
#define AssumingBinOp(a, b, op, text, ...) GHPL_ASSUME(((a) op (b)))
#define AssumingEqual(a, b, ...) AssumingBinOp(a, b, ==, "be equal to", __VA_ARGS__)
#define AssumingZero(a, ...) AssumingBinOp(a, 0, ==, "be equal to", __VA_ARGS__)
#define AssumingNotEqual(a, b, ...) AssumingBinOp(a, b, !=, "not be equal to", __VA_ARGS__)
#define AssumingGreater(a, b, ...) AssumingBinOp(a, b, >, "be greater than", __VA_ARGS__)
#define AssumingLess(a, b, ...) AssumingBinOp(a, b, <, "be less than", __VA_ARGS__)
#define AssumingGreaterEqual(a, b, ...) AssumingBinOp(a, b, >=, "be greater or equal to", __VA_ARGS__)
#define AssumingLessEqual(a, b, ...) AssumingBinOp(a, b, <=, "be less or equal to", __VA_ARGS__)
#define AssumingEmpty(exp, ...) do { using std::empty; GHPL_ASSUME(empty(exp)); } while (false)
#define AssumingNotEmpty(exp, ...) do { using std::empty; using std::size; GHPL_ASSUME(!empty(exp)); } while (false)
#define AssumingNullOrEmpty(exp, ...) do { using std::empty; using std::size; GHPL_ASSUME(::ghassanpl::detail::IsNullOrEmpty(exp));  } while (false)
#define AssumingNotNullOrEmpty(exp, ...) do { using std::empty; using std::size; GHPL_ASSUME(!::ghassanpl::detail::IsNullOrEmpty(exp)); } while (false)
#define AssumingValidIndex(_index, _container, ...) do { using std::size; auto&& _assuming_index = (_index); GHPL_ASSUME(((_assuming_index) >= 0 && size_t(_assuming_index) < size(_container))); } while (false)
#define AssumingValidIterator(_iterator, _container, ...) do { using std::end; auto&& _assuming_iterator = (_iterator); auto&& _assuming_container = (_container); const auto _assuming_end = end(_assuming_container); GHPL_ASSUME(!(_assuming_iterator == _assuming_end)); } while (false)
#define AssumingBetween(v, a, b, ...) do { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); GHPL_ASSUME(_assuming_v_v >= _assuming_a_v && _assuming_v_v < _assuming_b_v); } while (false)
#define AssumingBetweenInclusive(v, a, b, ...) do { auto&& _assuming_v_v = (v); auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); GHPL_ASSUME(_assuming_v_v >= _assuming_a_v && _assuming_v_v <= _assuming_b_v); } while (false)

#define AssumingContainsBits(a, b, ...) do { auto&& _assuming_a_v = (a); auto&& _assuming_b_v = (b); GHPL_ASSUME(!((_assuming_a_v & _assuming_b_v) == _assuming_b_v)); } while (false)


#endif

///@}

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
		concept formattable = requires { { std::formatter<T>{} }; }; /// TODO: Use std::formattable

		template <typename T>
		concept streamable = requires (T val, std::stringstream & ss) { { ss << val }; };

		template <typename T>
		decltype(auto) GetFormattable(T&& val)
		{
			using simple_type = std::remove_cvref_t<T>;
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
		std::string AdditionalDataToString(T&& fmt, ARGS&&... args)
		{
			auto formattable = std::tuple{ GetFormattable(std::forward<ARGS>(args))... };
			auto format_args = std::apply([](auto&... args) { return std::make_format_args(args...); }, formattable);
			return std::vformat(std::forward<T>(fmt), std::move(format_args));
		}
		
		/// Shamelessly stolen from UE :)
		struct RecursionScopeMarker
		{
			explicit RecursionScopeMarker(int& counter) : mCounter(counter) { ++mCounter; }
			~RecursionScopeMarker() { --mCounter; }
			int& mCounter;
		};


#endif
	}

	enum class AssumptionHandlerResult
	{
		Break,    ///< Break into debugger after reporting the assumption failure
		Terminate, ///< Terminate the program after reporting the assumption failure
		Continue, ///< Continue execution after reporting the assumption failure
	};

	inline auto DefaultReportAssumptionFailure(std::string_view expectation, std::initializer_list<std::pair<std::string_view, std::string>> values, std::string data, source_location loc
#if ASSUMING_USE_STACKTRACE
		, std::stacktrace stacktrace
#endif //  ASSUMING_USE_STACKTRACE
	) -> AssumptionHandlerResult
	{
		throw std::make_tuple(
			std::move(expectation),
			std::move(values),
			std::move(data),
			std::move(loc)
#if ASSUMING_USE_STACKTRACE
			, std::move(stacktrace)
#endif //  ASSUMING_USE_STACKTRACE
		);
	}

	/// This function must be provided by your own code, as it is called by an assumption macro with a failing assumption.
	/// \ingroup Assuming
	/// \param expectation An explanation of which assumption failed
	/// \param values The values of the expressions the assumption macro checked
	/// \param data Any additional arguments you gave to the macro, std-formatted.
	/// \param loc
	inline auto (*AssumptionFailureHandler)(std::string_view expectation, std::initializer_list<std::pair<std::string_view, std::string>> values, std::string data, source_location loc
#if ASSUMING_USE_STACKTRACE
		, std::stacktrace stacktrace
#endif //  ASSUMING_USE_STACKTRACE
	) -> AssumptionHandlerResult = nullptr;

	/// TODO: AssumptionFailureHandler should return bool, and based on that we should brek into debugger or not.

#if defined(ASSUMING_REPORT_NORETURN) && ASSUMING_REPORT_NORETURN
	[[noreturn]]
#endif
	inline auto ReportAssumptionFailure(std::string_view expectation, std::initializer_list<std::pair<std::string_view, std::string>> values, std::string data, source_location loc
#if __INTELLISENSE__
		= {}
#else
		= source_location::current()
#endif
#if ASSUMING_USE_STACKTRACE
		, std::stacktrace stacktrace = std::stacktrace::current()
#endif //  ASSUMING_USE_STACKTRACE
	) -> AssumptionHandlerResult
	{
		if (AssumptionFailureHandler)
			return AssumptionFailureHandler(std::move(expectation), std::move(values), std::move(data), std::move(loc)
#if ASSUMING_USE_STACKTRACE
				, std::move(stacktrace)
#endif //  ASSUMING_USE_STACKTRACE
		);
		else
		{
			return DefaultReportAssumptionFailure(std::move(expectation), std::move(values), std::move(data), std::move(loc)
#if ASSUMING_USE_STACKTRACE
				, std::move(stacktrace)
#endif //  ASSUMING_USE_STACKTRACE
			);
		}
	}
}

/// \namespace ghassanpl
/// Primary namespace for everything in this library.

/// \def ASSUMING_DEBUG
/// You can define this macro project-wide to either 0 or 1 to disable or enable checking of assumptions. If this macro is not defined, its value is based on
/// the NDEBUG macro.

/// \def ASSUMING_INCLUDE_MAGIC_ENUM
/// You can define this macro project-wide to either 1 or 0 to include `<magic_enum.hpp>` or not, if you want prettier printing of enum names.
/// If you don't define it, it will check for `<magic_enum.hpp>` and include it if it exists.
