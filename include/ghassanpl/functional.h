/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"
#include <functional>
#include <optional>
#include <type_traits>
#include <variant>
#if defined(__cpp_lib_bit_cast)
#include <bit>
#endif

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

	template <typename T>
	std::optional<T> to_optional(T* value) noexcept { return value ? *value : std::nullopt; }

	template <typename T>
	std::optional<T> move_to_optional(T* value) noexcept { return value ? std::move(*value) : std::nullopt; }

	///
	template <typename T> [[nodiscard]] constexpr auto flattened(std::optional<std::optional<T>>&& value) { return value ? flatten(std::move(value).value()) : std::nullopt; }
	template <typename T> [[nodiscard]] constexpr auto flattened(std::optional<T>&& value) { return value; }
	template <typename T> [[nodiscard]] constexpr auto flattened(std::optional<std::optional<T>> const& value) { return value ? flatten(value.value()) : std::nullopt; }
	template <typename T> [[nodiscard]] constexpr auto flattened(std::optional<T> const& value) { return value; }

	///
	template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

	///
	#define GHPL_OVERLOAD(...) [&](auto &&... args) -> decltype(auto) { \
		return __VA_ARGS__(std::forward<decltype(args)>(args)...); \
	}

	/// TODO: Boost::phoenix::arg_names is a better version of this

	/// Appropriate for predicate-taking functions like std::all_of
	namespace pred
	{
		                      [[nodiscard]] constexpr auto always_true() { return [](auto&& other) { return true; }; }

		template <typename T> [[nodiscard]] constexpr auto equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other == val; }; }
		template <typename T> [[nodiscard]] constexpr auto not_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other != val; }; }
		template <typename T> [[nodiscard]] constexpr auto less_than(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other < val; }; }
		template <typename T> [[nodiscard]] constexpr auto less_than_or_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other <= val; }; }
		template <typename T> [[nodiscard]] constexpr auto greater_than(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other > val; }; }
		template <typename T> [[nodiscard]] constexpr auto greater_than_or_equal_to(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other >= val; }; }
		
		template <typename T> [[nodiscard]] constexpr auto eq(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other == val; }; }
		template <typename T> [[nodiscard]] constexpr auto ne(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other != val; }; }
		template <typename T> [[nodiscard]] constexpr auto lt(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other < val; }; }
		template <typename T> [[nodiscard]] constexpr auto le(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other <= val; }; }
		template <typename T> [[nodiscard]] constexpr auto gt(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other > val; }; }
		template <typename T> [[nodiscard]] constexpr auto ge(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return other >= val; }; }
		
		                      [[nodiscard]] constexpr auto is_null() noexcept { return [](auto&& other) { return other == nullptr; }; }
		                      [[nodiscard]] constexpr auto is_not_null() noexcept { return [](auto&& other) { return other != nullptr; }; }
		                      [[nodiscard]] constexpr auto is_empty() noexcept { return [](auto&& other) { return std::empty(other); }; }
		                      [[nodiscard]] constexpr auto is_not_empty() noexcept { return [](auto&& other) { return !std::empty(other); }; }
		                      [[nodiscard]] constexpr auto is_true() noexcept { return [](auto&& other) { return !!other; }; }
		                      [[nodiscard]] constexpr auto is_false() noexcept { return [](auto&& other) { return !other; }; }
		template <typename T> [[nodiscard]] constexpr auto is_in(T&& val) { return [val = std::forward<T>(val)](auto&& other) { return std::find(std::begin(val), std::end(val), other) != std::end(val); }; }

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


		/// TODO: Need to differentiate between { !func } and [](val) -> !val
		/*
		template <typename FUNC>
		[[nodiscard]] constexpr auto negated(FUNC&& func)
		{
			return [func = std::forward<FUNC>(func)](auto&&... args) { return !func(std::forward<decltype(args)>(args)...); };
		}
		*/
	}

	/// Appropriate for functions like std::for_each that don't have semantics by themselves
	namespace op
	{
		template <typename T> [[nodiscard]] constexpr auto push_back_to(T& to) noexcept { return [&to](auto&& val) { to.push_back(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto emplace_back_to(T& to) noexcept { return [&to](auto&& val) { to.emplace_back(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto push_front_to(T& to) noexcept { return [&to](auto&& val) { to.push_front(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto emplace_front_to(T& to) noexcept { return [&to](auto&& val) { to.emplace_front(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto append_to(T& to) noexcept { return [&to](auto&& val) { to.append(std::forward<decltype(val)>(val)); }; }
		template <typename T> [[nodiscard]] constexpr auto insert_to(T& to) noexcept { return [&to](auto&& val) { to.insert(std::forward<decltype(val)>(val)); }; }
		
		template <typename T, GHPL_TYPENAME(std::output_iterator<T>) IT>
		[[nodiscard]] constexpr auto output_to(IT&& to) { return [to = std::forward<IT>(to)](auto&& val) { *to++ = std::forward<decltype(val)>(val); }; }

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

	/// Appropriate for not-in-place transformation functions 
	namespace xf
	{
		                      [[nodiscard]] constexpr auto identity() noexcept { return [](auto&& val) { return std::forward<decltype(val)>(val); }; }
		                                    constexpr auto identity_l = [] (auto&& val) noexcept { return std::forward<decltype(val)>(val); };

		template <typename T> [[nodiscard]] constexpr auto cast_to() noexcept { return [](auto&& val) { return (T)std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto dynamic_cast_to() noexcept { return [](auto&& val) { return dynamic_cast<T>(std::forward<decltype(val)>(val)); }; }
#if defined(__cpp_lib_bit_cast)
		template <typename T> [[nodiscard]] constexpr auto bit_cast_to() noexcept { return [](auto&& val) { return std::bit_cast<T>(std::forward<decltype(val)>(val)); }; }
#endif
		template <typename T> [[nodiscard]] constexpr auto constructed_as() noexcept { return [](auto&& val) { return T{ std::forward<decltype(val)>(val) }; }; }
		                      [[nodiscard]] constexpr auto called() noexcept { return [](auto&& val) { return std::forward<decltype(val)>(val)(); }; }

		template <typename T> [[nodiscard]] constexpr auto added_to(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) + other; }; }
		template <typename T> [[nodiscard]] constexpr auto subtracted_from(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return other - std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto decremented_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) - other; }; }
		template <typename T> [[nodiscard]] constexpr auto divided_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) / other; }; }
		template <typename T> [[nodiscard]] constexpr auto overed_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return other / std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto modulo_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) % other; }; }
		template <typename T> [[nodiscard]] constexpr auto multiplied_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) * other; }; }
		                      [[nodiscard]] constexpr auto complemented() noexcept { return [](auto&& val) { return -std::forward<decltype(val)>(val); }; }
		
		                      [[nodiscard]] constexpr auto negated() noexcept { return [](auto&& val) { return !std::forward<decltype(val)>(val); }; }
		
		                      [[nodiscard]] constexpr auto bit_inverted() noexcept { return [](auto&& val) { return ~std::forward<decltype(val)>(val); }; }
		template <typename T> [[nodiscard]] constexpr auto bit_anded_with(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) & other; }; }
		template <typename T> [[nodiscard]] constexpr auto bit_ored_with(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) | other; }; }
		template <typename T> [[nodiscard]] constexpr auto bit_xored_with(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) ^ other; }; }
		template <typename T> [[nodiscard]] constexpr auto shifted_left_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) << other; }; }
		template <typename T> [[nodiscard]] constexpr auto shifted_right_by(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) >> other; }; }
		
		template <typename T> [[nodiscard]] constexpr auto field(T&& field_ptr) noexcept { 
			return [field_ptr = std::forward<T>(field_ptr)](auto&& val) { 
				using arg_type = remove_cvref_t<decltype(val)>;
				if constexpr (std::is_pointer_v<arg_type>)
					return (std::forward<decltype(val)>(val)->*field_ptr);
				else
					return (std::forward<decltype(val)>(val).*field_ptr);
			}; 
		}
		
#ifdef __cpp_impl_three_way_comparison
		template <typename T> [[nodiscard]] constexpr auto compared_with(T&& other) noexcept { return [other = std::forward<T>(other)](auto&& val) { return std::forward<decltype(val)>(val) <=> other; }; }
#endif

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
	template <typename FUNC>
	GHPL_REQUIRES((!std::is_void_v<typename decltype(detail::detect_first_arg(std::function{ std::declval<FUNC>() }))::type>&& requires { std::function{ std::declval<FUNC>() }; }))
	constexpr auto resulting(FUNC&& func)
	{
		/// Because std::function can deduce function signature, but that magic is not available to us mortals via the stdlib:
		/// https://en.cppreference.com/w/cpp/utility/functional/function/deduction_guides
		using arg_type = typename decltype(detail::detect_first_arg(std::function{ std::declval<FUNC>() }))::type;
		static_assert(!std::is_void_v<arg_type>, "function must take a single l-value reference argument");
		std::remove_reference_t<arg_type> result{};
		func(result);
		return result;
	}

	/// Variant operations
	
	template <typename T, typename... ARGS>
	[[nodiscard]] constexpr std::optional<T> optional_get(std::variant<ARGS...> const& var) noexcept
	{
		if (std::holds_alternative<T>(var))
			return std::get<T>(var);
		return std::nullopt;
	}

	/*
	template <typename T, typename E, typename... ARGS>
	[[nodiscard]] constexpr std::expected<T, E> get_or(std::variant<ARGS...> const& var, E&& error) noexcept
	{
		if (std::holds_alternative<T>(var))
			return std::get<T>(var);
		return std::unexpected(std::forward<E>(error));
	}
	*/

	
	namespace detail
	{
		template<class... T> struct mp_list {};

		template <typename T, typename... TT>
		constexpr bool mp_set_contains() { return (std::is_same_v<TT, T> || ...); }
		template <typename T, typename... TT>
		constexpr bool mp_set_contains(mp_list<TT...>) { return (std::is_same_v<TT, T> || ...); }
		
		template <typename...TT, typename T>
		constexpr auto operator&(mp_list<TT...>, mp_list<T>) ->
			std::conditional_t<(std::is_same_v<TT, T> || ...),
				mp_list<TT...>,
				mp_list<TT..., T>
			>;
		template <typename...TT, typename T>
		constexpr auto operator+(mp_list<TT...>, mp_list<T>) -> mp_list<TT..., T>;

		template <template <typename...> typename TMPL, typename... Ts>
		constexpr auto mp_apply(mp_list<Ts...>) -> TMPL<Ts...>;

		/*
		template<class L> struct mp_make_unique_impl;
		template<template<class...> class L, class... T>
		struct mp_make_unique_impl<L<T...>> {
			using type = decltype(mp_apply<L>((mp_list<>{} & ... & mp_list<T>{})));
		};
		template<class L> using mp_make_unique = typename mp_make_unique_impl<L>::type;
		*/
		template<template<class...> class L, class... T>
		constexpr auto mp_make_unique(L<T...>) -> decltype(mp_apply<L>((mp_list<>{} & ... & mp_list<T>{})));
	}

	/// Returns a new variant transformed by `func`
	template <typename FUNC, typename... Ts>
	constexpr auto transformed(std::variant<Ts...> const& var, FUNC&& func)
	{
		using result_type = decltype(detail::mp_make_unique(std::variant<
			decltype(func(std::declval<Ts>()))...
		>{}));
		return std::visit([&func](auto&& val) -> result_type { return result_type(func(std::forward<decltype(val)>(val))); }, var);
	}


	namespace detail
	{

		template <typename... Ts1, typename... Ts2>
		constexpr auto cat_lists(mp_list<Ts1...>, mp_list<Ts2...>) -> mp_list<Ts1..., Ts2...>;

		template <typename... Ts>
		constexpr auto flatten(mp_list<std::variant<Ts...>>) -> decltype(flatten(mp_list<Ts...>{}));

		template <typename T>
		constexpr auto flatten(mp_list<T>) -> mp_list<T>;

		template <typename T, typename T2, typename... Ts>
		constexpr auto flatten(mp_list<T, T2, Ts...>)
			-> decltype(cat_lists(cat_lists(flatten(mp_list<T>{}), flatten(mp_list<T2>{})), mp_list<Ts... >{}));

		template <typename... Ts>
		using flattened = decltype(flatten(mp_list<Ts...>{}));

		template <template <typename...> typename TMPL, typename T1, typename T2, typename... Ts>
		constexpr auto mp_apply_or_single(mp_list<T1, T2, Ts...>) -> TMPL<T1, T2, Ts...>;
		template <template <typename...> typename TMPL, typename T>
		constexpr auto mp_apply_or_single(mp_list<T>) -> T;

		//template <typename T, typename Ts...> using first_only = T;
		template <typename T, typename... Ts>
		using variant_flat_t = decltype(mp_apply_or_single<std::variant>(detail::mp_make_unique(flattened<T, Ts...>{})));
	}

	template <typename... TYPES>
	struct convertible_to_variant
	{
		std::variant<TYPES...> value;

		convertible_to_variant() noexcept = delete;
		convertible_to_variant(convertible_to_variant&&) noexcept = default;
		convertible_to_variant& operator=(convertible_to_variant&&) noexcept = default;

		convertible_to_variant(std::variant<TYPES...> val) noexcept : value(std::move(val)) {}
		convertible_to_variant& operator=(std::variant<TYPES...> val) noexcept { std::swap(value, val); return *this; }

		template <typename... OTHER_TYPES, 
			typename = std::enable_if_t<!std::is_same_v<decltype(value), std::variant<OTHER_TYPES...>>>,
			typename = std::enable_if_t<(detail::mp_set_contains<TYPES, OTHER_TYPES...>() && ...)>
		>
		operator std::variant<OTHER_TYPES...>() && noexcept
		{
			static_assert((detail::mp_set_contains<TYPES, OTHER_TYPES...>() && ...), "not every type in source variant is present in destination variant");
			
			return std::visit(overloaded{
				[](TYPES&& val) -> std::variant<OTHER_TYPES...> { 
					return std::variant<OTHER_TYPES...>(std::move(val)); 
				}...
			}, std::move(value));
		}

		operator std::variant<TYPES...>() && noexcept {
			return std::move(value);
		}
	};

	template <typename... TYPES>
	convertible_to_variant(std::variant<TYPES...>) -> convertible_to_variant<TYPES...>;
	template <typename T>
	convertible_to_variant(T) -> convertible_to_variant<remove_cvref_t<T>>;

#ifndef __clang__
	/// Returns a new variant transformed by `func`, flattened
	template <typename FUNC, typename... Ts>
	constexpr auto transformed_flattened(std::variant<Ts...> const& var, FUNC&& func)
	{
		using result_type = decltype(detail::mp_make_unique(
			detail::variant_flat_t<decltype(func(std::declval<Ts>()))...>{}
		));
		return std::visit([&func](auto&& val) -> result_type { return convertible_to_variant(func(std::forward<decltype(val)>(val))); }, var);
	}
#endif
}
