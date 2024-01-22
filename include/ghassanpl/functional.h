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
	[[nodiscard]] constexpr auto make_single_time_function(std::function<void(ARGS...)> func)
	{
		return[func = std::move(func)](auto&&... args) mutable {
			if (func) std::exchange(func, {})(std::forward<decltype(args)>(args)...);
		};
	}

	/// Returns a function that calls `func` when invoked, but only the first time
	/// \ingroup Functional
	template <typename FUNC>
	[[nodiscard]] constexpr auto make_single_time_function(FUNC&& func)
	{
		return make_single_time_function(std::function{ std::forward<FUNC>(func) });
	}

	/// Pre C++23 transform/fmap
	template <typename T, typename FUNC>
	[[nodiscard]] constexpr auto transformed(std::optional<T> const& value, FUNC&& func) -> decltype(std::optional{ func(value.value()) })
	{
		return value ? std::optional{ func(value.value()) } : std::nullopt;
	}

	/// Pre C++23 transform/fmap
	template <typename T, typename FUNC>
	[[nodiscard]] constexpr auto transformed(std::optional<T>&& value, FUNC&& func) -> decltype(std::optional{ func(value.value()) })
	{
		return value ? std::optional{ func(std::move(value).value()) } : std::nullopt;
	}

	template <typename T> [[nodiscard]] constexpr auto flattened(std::optional<std::optional<T>>&& value) { return value ? flatten(std::move(value).value()) : std::nullopt; }
	template <typename T> [[nodiscard]] constexpr auto flattened(std::optional<T>&& value) { return value; }

	namespace pred
	{
		template <typename T> [[nodiscard]] constexpr auto equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other == val; }; }
		template <typename T> [[nodiscard]] constexpr auto not_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other != val; }; }
		template <typename T> [[nodiscard]] constexpr auto less_than(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other < val; }; }
		template <typename T> [[nodiscard]] constexpr auto less_than_or_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other <= val; }; }
		template <typename T> [[nodiscard]] constexpr auto greater_than(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other > val; }; }
		template <typename T> [[nodiscard]] constexpr auto greater_than_or_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other >= val; }; }
		                      [[nodiscard]] constexpr auto is_null() noexcept { return [](auto&& other) { return other == nullptr; }; }
		                      [[nodiscard]] constexpr auto is_not_null() noexcept { return [](auto&& other) { return other != nullptr; }; }
		                      [[nodiscard]] constexpr auto is_empty() noexcept { return [](auto&& other) { return std::empty(other); }; }
		                      [[nodiscard]] constexpr auto is_not_empty() noexcept { return [](auto&& other) { return !std::empty(other); }; }
		                      [[nodiscard]] constexpr auto is_true() noexcept { return [](auto&& other) { return !!other; }; }
		                      [[nodiscard]] constexpr auto is_false() noexcept { return [](auto&& other) { return !other; }; }
		template <typename T> [[nodiscard]] constexpr auto is_in(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return std::find(std::begin(val), std::end(val), other) != std::end(val); }; }

		template <typename FUNC> [[nodiscard]] constexpr auto negated(FUNC&& func) {
			return [func = std::forward<FUNC>(func)](auto&&... args) { return func(std::forward<decltype(args)>(args)...); };
		}

		template <typename... FUNCS>
		[[nodiscard]] constexpr auto when_any(FUNCS&&... funcs) {
			return [funcs = std::make_tuple(std::forward<FUNCS>(funcs)...)](auto&&... args) {
				return std::apply([&](auto&&... funcs) {
					return (funcs(args...) || ...);
				}, funcs);
			};
		}
		
		template <typename... FUNCS>
		[[nodiscard]] constexpr auto when_all(FUNCS&&... funcs) {
			return [funcs = std::make_tuple(std::forward<FUNCS>(funcs)...)](auto&&... args) {
				return std::apply([&](auto&&... funcs) {
					return (funcs(args...) && ...);
				}, funcs);
			};
		}

		template <typename... FUNCS>
		[[nodiscard]] constexpr auto when_none(FUNCS&&... funcs) {
			return [funcs = std::make_tuple(std::forward<FUNCS>(funcs)...)](auto&&... args) {
				return std::apply([&](auto&&... funcs) {
					return !(funcs(args...) || ...);
				}, funcs);
			};
		}
	}

	namespace op
	{
		template <typename T> [[nodiscard]] constexpr auto push_back_to(T& to) noexcept { return [&to](auto&& val) { to.push_back(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto emplace_back_to(T& to) noexcept { return [&to](auto&& val) { to.emplace_back(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto push_front_to(T& to) noexcept { return [&to](auto&& val) { to.push_front(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto emplace_front_to(T& to) noexcept { return [&to](auto&& val) { to.emplace_front(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto append_to(T& to) noexcept { return [&to](auto&& val) { to.append(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto insert_to(T& to) noexcept { return [&to](auto&& val) { to.insert(std::forward<decltype(val)>(val)); }; }
		
		template <typename T, GHPL_TYPENAME(std::output_iterator<T>) IT>
		[[nodiscard]] constexpr auto output_to(IT&& to) { return [to = std::forward<IT>(to)](auto&& val) { *to++ = std::forward<decltype(val)>(val); } }

		template <typename T> [[nodiscard]] constexpr auto assign_to(T& to) noexcept { return [&to](auto&& val) { to = std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto add_to(T& to) noexcept { return [&to](auto&& val) { to += std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto stream_to(T& to) noexcept { return [&to](auto&& val) { to << std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto stream_from(T& from) noexcept { return [&from](auto&& val) { from >> std::forward<decltype(val)>(val); }; }

		/// For funcs a, b, c, calls them as `return (a(args...), b(args...), c(args...))`
		/// \return return value of last call
		template <typename... FUNCS>
		[[nodiscard]] constexpr auto call_all(FUNCS&&... funcs) /// forwarding FUNCS can throw
		{
			return [funcs = std::make_tuple(std::forward<FUNCS>(funcs)...)](auto&&... args) {
				return std::apply([&](auto&&... funcs) { 
					return (funcs(args...), ...); 
				}, funcs);
			};
		}

		template <typename F>
		[[nodiscard]] constexpr auto call_composed(F&& f) /// forwarding F can throw
		{
			return std::forward<F>(f);
		}

		/// For funcs a, b, c, calls them as `return a(b(c(args...)))`
		template <typename F, typename ... Fs>
		[[nodiscard]] constexpr auto call_composed(F&& f, Fs&&... fs) /// forwarding F or Fs can throw
		{
			return [first = std::forward<F>(f), rest = call_composed(std::forward<Fs>(fs)...)](auto&&... args) {
				return first(rest(std::forward<decltype(args)>(args)...));
			};
		}

		template <typename F>
		[[nodiscard]] constexpr auto call_piped(F&& f) /// forwarding F can throw
		{
			return std::forward<F>(f);
		}

		/// For funcs a, b, c, calls them as `return c(b(a(args...)));` or `return a(args...) |> b |> c;`
		template <typename F, typename ... Fs>
		[[nodiscard]] constexpr auto call_piped(F&& f, Fs&&... fs) /// forwarding F or Fs can throw
		{
			return [first = std::forward<F>(f), rest = call_composed(std::forward<Fs>(fs)...)](auto&&... args) {
				return rest(first(std::forward<decltype(args)>(args)...));
			};
		}

		/// Acts as `if (predicate(args...)) op(args...);`
		template <typename THEN, typename IF>
		[[nodiscard]] constexpr auto call_when(THEN&& op, IF&& predicate)
		{
			return [predicate = std::forward<IF>(predicate), op = std::forward<THEN>(op)](auto&&... args) {
				if (predicate(args...))
					op(std::forward<decltype(args)>(args)...);
			};
		}
		
		/// Acts as `if (predicate(args...)) op(args...);`
		template <typename THEN, typename IF, typename ELSE_VAL>
		[[nodiscard]] constexpr auto call_when(THEN&& op, IF&& predicate, ELSE_VAL&& elseval)
		{
			return [predicate = std::forward<IF>(predicate), op = std::forward<THEN>(op), elseval = std::forward<ELSE_VAL>(elseval)](auto&&... args)
				-> std::common_type_t<
					decltype(op(std::forward<decltype(args)>(args)...)),
					decltype(std::forward<ELSE_VAL>(elseval))
				>
			{
				if (predicate(args...))
					return op(std::forward<decltype(args)>(args)...);
				return std::forward<ELSE_VAL>(elseval);
			};
		}

		/// Acts as `if (predicate(args...)) return op(args...); else return elseop(args...);`
		template <typename THEN, typename IF, typename ELSE>
		[[nodiscard]] constexpr auto call_when_else(THEN&& op, IF&& predicate, ELSE&& elseop)
		{
			return [predicate = std::forward<IF>(predicate), op = std::forward<THEN>(op), elseop = std::forward<ELSE>(elseop)](auto&&... args) 
				-> std::common_type_t<
					decltype(op(std::forward<decltype(args)>(args)...)),
					decltype(elseop(std::forward<decltype(args)>(args)...))
				> 
			{
				if (predicate(args...))
					return op(std::forward<decltype(args)>(args)...);
				else
					return elseop(std::forward<decltype(args)>(args)...);
			};
		}

	}

	namespace xf
	{
		template <typename T> [[nodiscard]] constexpr auto cast_to() noexcept { return [](auto&& val) { return (T)std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto dynamic_cast_to() noexcept { return [](auto&& val) { return dynamic_cast<T>(std::forward<decltype(val)>(val)); }; }
#if defined(__cpp_lib_bit_cast)
		template <typename T> [[nodiscard]] constexpr auto bit_cast_to() noexcept { return [](auto&& val) { return std::bit_cast<T>(std::forward<decltype(val)>(val)); }; }
#endif
		template <typename T> [[nodiscard]] constexpr auto constructed_as() noexcept { return [](auto&& val) { return T{ std::forward<decltype(val)>(val) }; }; }
		template <typename T> [[nodiscard]] constexpr auto called() noexcept { return [](auto&& val) { return val(); }; }
	}

	/// Macro that allows us to pass std:: functions to algorithms as if they were function pointers,
	/// because you can't take the address of an std:: function for some stupid reason
#define std_call(func) ([](auto&&... vals) { return std::func(std::forward<decltype(vals)>(vals)...); })

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
		static_assert(!std::is_void_v<ARG_TYPE>, "function must take a single l-value reference argument");
		std::remove_reference_t<ARG_TYPE> result{};
		func(result);
		return result;
	}
} // namespace util
