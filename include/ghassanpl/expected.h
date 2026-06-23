/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <system_error>
#include <optional>
#include "min-cpp-version/cpp17.h"
#define GHPL_LATEST_MSVC_VERSION_WITH_EXPECTED_BUGS 1942
#if __has_include(<expected>) && (!defined(_MSC_VER) || _MSC_VER > GHPL_LATEST_MSVC_VERSION_WITH_EXPECTED_BUGS) && defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
#include <expected>
namespace ghassanpl
{
	using std::expected;
	using std::unexpected;
}
#elif __has_include(<tl/expected.hpp>)
#include <tl/expected.hpp>
namespace ghassanpl
{
	using tl::expected;
	using tl::unexpected;
}
#elif defined(GHPL_EXPECTED_HEADER)
#include GHPL_EXPECTED_HEADER
#else
#error "No expected implementation found"
#endif

namespace ghassanpl
{
	namespace detail
	{
		template <typename T>
		constexpr bool is_expected_f(type_identity<T>) { return false; }
		template <typename E, typename V>
		constexpr bool is_expected_f(type_identity<expected<V, E>> e) { return true; }

		template <typename T>
		constexpr auto expected_error_type(type_identity<T>) { return type_identity<void>{}; }
		template <typename E, typename V>
		constexpr auto expected_error_type(type_identity<expected<V, E>> e) { return type_identity<E>{}; }

		template <typename T>
		using error_type_t = typename decltype(expected_error_type(type_identity<T>{}))::type;
		static_assert(std::is_same_v<error_type_t<expected<int, bool>>, bool>);
	}

	template <typename T>
	constexpr bool is_expected = detail::is_expected_f(type_identity<remove_cvref_t<T>>{});
	template <typename E, typename T>
	constexpr bool is_expected_with_error = detail::is_expected_f(type_identity<T>{}) && std::is_same_v<detail::error_type_t<T>, E>;
	static_assert(is_expected<expected<bool, int>>);
	static_assert(is_expected_with_error<int, expected<bool, int>>);

	/// Calls the given function with `args` and an `std::error_code` as the last argument
	/// \returns an `expected` with the result of the function call if the `std::error_code` is `0`, otherwise an `unexpected` with the error code
	template <typename FUNC, typename... ARGS>
	auto call_with_expected_ec(FUNC&& func, ARGS&&... args) noexcept(noexcept(func(std::forward<ARGS>(args)...)))
		-> expected<std::invoke_result_t<FUNC, ARGS&&..., std::add_lvalue_reference_t<std::error_code>>, std::error_code>
	{
		std::error_code ec{};
		if constexpr (std::is_void_v<std::invoke_result_t<FUNC, ARGS&&..., std::add_lvalue_reference_t<std::error_code>>>)
		{
			func(std::forward<ARGS>(args)..., ec);
			if (ec)
				return unexpected(ec);
			return {};
		}
		else
		{
			auto result = func(std::forward<ARGS>(args)..., ec);
			if (ec)
				return unexpected(ec);
			return result;
		}
	}

	template <typename T>
	struct undroppable final
	{
		template <typename... ARGS, typename = std::enable_if_t<std::is_constructible_v<T, ARGS...>>>
		undroppable(ARGS&&... args) noexcept(std::is_nothrow_constructible_v<T, ARGS...>)
			: m_value(std::forward<ARGS>(args)...)
		{
		}
		
		undroppable(undroppable const& other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		undroppable& operator=(undroppable const& other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		undroppable(undroppable&& other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		undroppable& operator=(undroppable&& other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		T handle() noexcept
		{
			return std::exchange(m_value, {}).value();
		}

		bool was_handled() const noexcept { return !m_value.has_value(); }

		T const& value() const noexcept { return m_value.value(); }
		T& value() noexcept { return m_value.value(); }

		~undroppable() noexcept(false)
		{
			if (m_value)
				throw std::move(m_value.value());
		}

	private:

		std::optional<T> m_value;
	};

	template <typename T, typename E, typename U>
	expected<T, E> return_if(expected<U, E>& exp, T&& value)
	{
		if (exp)
			return expected<T, E>{ std::forward<T>(value) };
		else
			return unexpected(std::move(exp).error());
	}

	template <typename T, typename E, typename FUNC>
	auto with_error(expected<T, E> const& exp, FUNC&& func)
	{
		using result_type = std::invoke_result_t<FUNC, decltype(exp.error())>;
		if constexpr (std::is_void_v<result_type>)
		{
			if (!exp)
				func(exp.error());
		}
		else
		{
			return !exp ? func(exp.error()) : result_type{};
		}
	}

	template <typename T, typename E, typename FUNC>
	auto with_error(expected<T, E>&& exp, FUNC&& func)
	{
		using result_type = std::invoke_result_t<FUNC, decltype(exp.error())>;
		if constexpr (std::is_void_v<result_type>)
		{
			if (!exp)
				func(std::move(exp).error());
		}
		else
		{
			return !exp ? func(std::move(exp).error()) : result_type{};
		}
	}

#define TOKEN_PASTE(x, y) x ## y
#define TOKEN_PASTE2(x, y) TOKEN_PASTE(x, y)
#define OR_RETURN_VAL TOKEN_PASTE2(_return_val_, __LINE__)
#define or_return(exp) if (decltype(auto) OR_RETURN_VAL = (exp); !OR_RETURN_VAL.has_value()) return ::ghassanpl::unexpected(std::move(OR_RETURN_VAL).error())
#define set_or_return(var, exp) do { if (decltype(auto) OR_RETURN_VAL = (exp); !OR_RETURN_VAL.has_value()) return ::ghassanpl::unexpected(std::move(OR_RETURN_VAL).error()); else var = std::move(OR_RETURN_VAL).value(); } while (0)
#define let_or_return(var, exp) \
	decltype(auto) OR_RETURN_VAL = (exp); \
	if (!OR_RETURN_VAL)\
		return ::ghassanpl::unexpected(std::move(OR_RETURN_VAL).error()); \
	auto var = std::forward<decltype(OR_RETURN_VAL)>(OR_RETURN_VAL).value()
#define do_or_return(exp, func) if (decltype(auto) OR_RETURN_VAL = (exp); !OR_RETURN_VAL.has_value()) return ::ghassanpl::unexpected(std::move(OR_RETURN_VAL).error()); else (func)(std::move(OR_RETURN_VAL).value());
#define or_break(exp) if (decltype(auto) OR_RETURN_VAL = (exp); !OR_RETURN_VAL.has_value()) break
}
