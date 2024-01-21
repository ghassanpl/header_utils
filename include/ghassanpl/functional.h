/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <functional>
#include <optional>
#include <type_traits>

namespace ghassanpl
{
	/// Returns a function that calls `func` when invoked, but only the first time
	/// \ingroup Functional
	template <typename... ARGS>
	[[nodiscard]] auto make_single_time_function(std::function<void(ARGS...)> func)
	{
		return[func = std::move(func)](auto&&... args) mutable {
			if (func) std::exchange(func, {})(std::forward<decltype(args)>(args)...);
			};
	}

	/// Returns a function that calls `func` when invoked, but only the first time
	/// \ingroup Functional
	template <typename FUNC>
	[[nodiscard]] auto make_single_time_function(FUNC&& func)
	{
		return make_single_time_function(std::function{ std::forward<FUNC>(func) });
	}

	///
	template <typename T, typename FUNC>
	[[nodiscard]] auto transform(std::optional<T> const& value, FUNC&& func) -> decltype(std::optional{ func(value.value()) })
	{
		return value ? std::optional{ func(value.value()) } : std::nullopt;
	}

	namespace pred
	{
		template <typename T> constexpr auto equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other == val; }; }
		template <typename T> constexpr auto not_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other != val; }; }
		template <typename T> constexpr auto less_than(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other < val; }; }
		template <typename T> constexpr auto less_than_or_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other <= val; }; }
		template <typename T> constexpr auto greater_than(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other > val; }; }
		template <typename T> constexpr auto greater_than_or_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other >= val; }; }
		constexpr auto is_null() { return [](auto&& other) { return other == nullptr; }; }
		constexpr auto is_not_null() { return [](auto&& other) { return other != nullptr; }; }
		constexpr auto is_empty() { return [](auto&& other) { return std::empty(other); }; }
		constexpr auto is_not_empty() { return [](auto&& other) { return !std::empty(other); }; }
		constexpr auto is_true() { return [](auto&& other) { return !!other; }; }
		constexpr auto is_false() { return [](auto&& other) { return !other; }; }
		template <typename T> constexpr auto is_in(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return std::find(std::begin(val), std::end(val), other) != std::end(val); }; }
	}

	namespace op
	{
		template <typename T>
		constexpr auto push_back_to(T& to) { return [&to](auto&& val) { to.push_back(std::forward<decltype(val)>(val)); }; }
		template <typename T>
		constexpr auto emplace_back_to(T& to) { return[&to](auto&& val) { to.emplace_back(std::forward<decltype(val)>(val)); }; }
		template <typename T>
		constexpr auto push_front_to(T& to) { return [&to](auto&& val) { to.push_front(std::forward<decltype(val)>(val)); }; }
		template <typename T>
		constexpr auto emplace_front_to(T& to) { return[&to](auto&& val) { to.emplace_front(std::forward<decltype(val)>(val)); }; }
		template <typename T>
		constexpr auto insert_to(T& to) { return [&to](auto&& val) { to.insert(std::forward<decltype(val)>(val)); }; }
		template <typename T, GHPL_TYPENAME(std::output_iterator<T>) IT>
		constexpr auto output_to(IT&& to) { return [to = std::forward<IT>(to)](auto&& val) { *to++ = std::forward<decltype(val)>(val); }; }

		template <typename T>
		constexpr auto append_to(T& to) { return [&to](auto&& val) { to.append(std::forward<decltype(val)>(val)); }; }
	}

	namespace xf
	{
		template <typename T>
		constexpr auto cast_to() { return [](auto&& val) { return (T)std::forward<decltype(val)>(val); } }
		template <typename T>
		constexpr auto dynamic_cast_to() { return [](auto&& val) { return dynamic_cast<T>(std::forward<decltype(val)>(val)); } }
#if defined(__cpp_lib_bit_cast)
		template <typename T>
		constexpr auto bit_cast_to() { return [](auto&& val) { return std::bit_cast<T>(std::forward<decltype(val)>(val)); } }
#endif

		template <typename T>
		constexpr auto constructed_as() {
			return [](auto&& val) { return T{ std::forward<decltype(val)>(val) }; }
		}
		
		template <typename T>
		constexpr auto called() { return [](auto&& val) { return val(); }; }
	}

	/// Returns a new `T` transformed by `func`
	template <typename T, typename FUNC>
	constexpr T resulting(FUNC&& func)
	{
		T result{};
		func(result);
		return result;
	}

	namespace detail
	{
		template <typename Ret, typename Arg, typename = std::enable_if_t<
			std::is_lvalue_reference_v<Arg> &&
			!std::is_const_v<std::remove_reference_t<Arg>>&&
			std::is_default_constructible_v<std::remove_reference_t<Arg>>>
		>
		constexpr auto detect_first_arg(std::function<Ret(Arg)>) {
			return type_identity<Arg>{};
		}
		constexpr auto detect_first_arg(...) {
			return type_identity<void>{};
		}
	}

	/// Returns a new object transformed by `func`. The first argument to `func` determines what the object's type is.
	template <typename FUNC, typename ARG_TYPE = typename decltype(detail::detect_first_arg(std::function{ std::declval<FUNC>() }))::type >
	GHPL_REQUIRES((!std::is_void_v<ARG_TYPE>&& requires { std::function{ std::declval<FUNC>() }; }))
		constexpr auto resulting(FUNC&& func)
	{
		/// Because std::function can deduce function signature, but that magic is not available to us mortals via the stdlib:
		/// https://en.cppreference.com/w/cpp/utility/functional/function/deduction_guides
		//using type = ;
		static_assert(!std::is_void_v<ARG_TYPE>, "function must take a single l-value reference argument");
		std::remove_reference_t<ARG_TYPE> result{};
		func(result);
		return result;
	}
} // namespace util
