/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

//#include <utility>
#include <tuple>
#include <variant>
#include <optional>
#include <any>

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif

/// Concepts
namespace ghassanpl
{
	template <class T, class... TYPES>
	constexpr inline bool is_any_of_v = std::disjunction_v<std::is_same<T, TYPES>...>;
}

/// Manipulators
/// All of these are pretty much stolen from Daisy Hollman (https://twitter.com/the_whole_daisy), her twitter is fantastic
namespace ghassanpl
{
	template<size_t I, typename... Ts>
	using nth_type_of = std::tuple_element_t<I, std::tuple<Ts...>>;
	template<typename... Ts>
	using first_type_of = std::tuple_element_t<0, std::tuple<Ts...>>;
	template<typename... Ts>
	using last_type_of = std::tuple_element_t<(sizeof...(Ts) - 1), std::tuple<Ts...>>;

	template <size_t I, typename T, typename... Ts>
	auto nth_value_of(T&& t, Ts&&... args)
	{
		if constexpr (I == 0)
			return std::forward<T>(t);
		else
		{
			using return_type = typename nth_type_of<I, T, Ts...>::type;
			return std::forward<return_type>(nth_value_of<I - 1>((std::forward<Ts>(args))...));
		}
	}

	template <typename... Ts>
	auto first_value_of(Ts&&... args)
	{
		using return_type = typename first_type_of<Ts...>::type;
		return std::forward<return_type>(nth_value_of<0>((std::forward<Ts>(args))...));
	}

	template <typename... Ts>
	auto last_value_of(Ts&&... args)
	{
		using return_type = typename last_type_of<Ts...>::type;
		return std::forward<return_type>(nth_value_of<sizeof...(Ts) - 1>((std::forward<Ts>(args))...));
	}



	/// Function: for_each
	/// TODO: Description
	template <typename... ARGS, typename FUNC>
	constexpr void for_each(FUNC&& f);

	/// Function: for_each_pair
	/// TODO: Description
	template <typename FUNC, typename... ARGS>
	requires ((sizeof...(ARGS) % 2) == 0)
	constexpr void for_each_pair(FUNC&& f, ARGS&&... args);

	/// Function: for_each_in_tuple
	template<typename... ARGS, typename FUNC>
	constexpr void for_each_in_tuple(std::tuple<ARGS...> const& t, FUNC&& f);

	/// Function: transform_tuple
	template<typename... ARGS, typename FUNC>
	constexpr auto transform_tuple(std::tuple<ARGS...> const& t, FUNC&& f);

	/// Function: enumerate_pack
	/// Calls `f` on every argument `args...`.
	/// `f` must take one or two arguments: either a single argument for each element of the pack, or a `size_t` index of the argument, and then the argument.
	template <typename FUNC, typename... ARGS>
	constexpr void enumerate_pack(FUNC&& f, ARGS&&... args);

	/// Function: apply_to_nth
	/// Calls `f` on the `N`th argument in the `args` pack.
	template <size_t N, typename FUNC, typename... ARGS>
	requires (N < sizeof...(ARGS))
	constexpr auto apply_to_nth(FUNC&& f, ARGS&&... args);

	/// Function: apply_to_slice
	/// Calls `f` with a slice of the `args` pack. The slice will start at `Begin`, end at `End`, and increment by `Stride` arguments.
	template <size_t Begin, size_t End, size_t Stride, double&..., typename FUNC, typename... ARGS>
	constexpr auto apply_to_slice(FUNC&& f, ARGS&&... args);

	/// Function: apply_to_slice
	/// Calls `f` with a slice of the `args` pack. The slice will start at `Begin`, end at `End`, and increment by 1 argument.
	template <size_t Begin, size_t End, double&..., typename FUNC, typename... ARGS>
	constexpr auto apply_to_slice(FUNC&& f, ARGS&&... args);

	/// Function: apply_to_slice
	/// Calls `f` with a slice of the `args` pack. The slice will start at `Begin`, and increment by 1 argument.
	template <size_t Begin, double&..., typename FUNC, typename... ARGS>
	constexpr auto apply_to_slice(FUNC&& f, ARGS&&... args);


	/// Shamelessly stolen from hsutter's cppfront
	template< typename C, typename X >
	auto is(X const&) -> bool {
		return false;
	}

	template< typename C, typename X >
	requires std::is_same_v<C, X>
	auto is(X const&) -> bool {
		return true;
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<C, X> && !std::is_same_v<C, X>)
	auto is(X const&) -> bool {
		return true;
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<X, C> && !std::is_same_v<C, X>)
	auto is(X const& x) -> bool {
		return dynamic_cast<C const*>(&x) != nullptr;
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<X, C> && !std::is_same_v<C, X>)
	auto is(X const* x) -> bool {
		return dynamic_cast<C const&>(x) != nullptr;
	}

	template< typename C, typename X >
	requires (requires (X x) { *x; X(); }&& std::is_same_v<C, void>)
	auto is(X const& x) -> bool
	{
		return x == X();
	}

	template< typename C >
	auto as(...) -> auto {
		return nullptr;
	}

	template< typename C, typename X >
	requires std::is_same_v<C, X>
	auto as(X const& x) -> auto&& {
		return x;
	}

	template< typename C, typename X >
	auto as(X const& x) -> auto
	requires (!std::is_same_v<C, X>&& requires { C{ x }; })
	{
		return C{ x };
	}

	template< typename C, typename X >
	requires std::is_base_of_v<C, X>
	auto as(X&& x) -> C&& {
		return std::forward<X>(x);
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<X, C> && !std::is_same_v<C, X>)
	auto as(X& x) -> C& {
		return dynamic_cast<C&>(x);
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<X, C> && !std::is_same_v<C, X>)
	auto as(X const& x) -> C const& {
		return dynamic_cast<C const&>(x);
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<X, C> && !std::is_same_v<C, X>)
	auto as(X* x) -> C* {
		return dynamic_cast<C*>(x);
	}

	template< typename C, typename X >
	requires (std::is_base_of_v<X, C> && !std::is_same_v<C, X>)
	auto as(X const* x) -> C const* {
		return dynamic_cast<C const*>(x);
	}


	//-------------------------------------------------------------------------------------------------------------
	//  std::variant is and as
	//
	template<typename... Ts>
	constexpr auto operator_is(std::variant<Ts...> const& x) {
		return x.index();
	}

	template<size_t I, typename... Ts>
	constexpr auto operator_as(std::variant<Ts...> const& x) -> auto&& 
	{
		if constexpr (I < std::variant_size_v<std::variant<Ts...>>)
			return std::get<I>(x);
		else
			return nullptr;
	}


	#define CPP2_TYPEOF(x)  std::remove_cvref_t<decltype(x)>

	template<typename T, typename... Ts>
	auto is(std::variant<Ts...> const& x)
	{
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<0>(x)), T >) if (x.index() == 0) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<1>(x)), T >) if (x.index() == 1) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<2>(x)), T >) if (x.index() == 2) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<3>(x)), T >) if (x.index() == 3) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<4>(x)), T >) if (x.index() == 4) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<5>(x)), T >) if (x.index() == 5) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<6>(x)), T >) if (x.index() == 6) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<7>(x)), T >) if (x.index() == 7) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<8>(x)), T >) if (x.index() == 8) return true;
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<9>(x)), T >) if (x.index() == 9) return true;
		if constexpr (std::is_same_v< T, void >)
		{
			if (x.valueless_by_exception()) return true;
			//  Need to guard this with is_any otherwise the get_if is illegal
			if constexpr (is_any_of_v<std::monostate, Ts...>) return std::get_if<std::monostate>(&x) != nullptr;
		}
		return false;
	}

	template<typename T, typename... Ts>
	auto as(std::variant<Ts...> const& x)
	{
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<0>(x)), T >) if (x.index() == 0) return operator_as<0>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<1>(x)), T >) if (x.index() == 1) return operator_as<1>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<2>(x)), T >) if (x.index() == 2) return operator_as<2>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<3>(x)), T >) if (x.index() == 3) return operator_as<3>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<4>(x)), T >) if (x.index() == 4) return operator_as<4>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<5>(x)), T >) if (x.index() == 5) return operator_as<5>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<6>(x)), T >) if (x.index() == 6) return operator_as<6>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<7>(x)), T >) if (x.index() == 7) return operator_as<7>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<8>(x)), T >) if (x.index() == 8) return operator_as<8>(x);
		if constexpr (std::is_same_v< CPP2_TYPEOF(operator_as<9>(x)), T >) if (x.index() == 9) return operator_as<9>(x);
		throw std::bad_variant_access();
	}


	//-------------------------------------------------------------------------------------------------------------
	//  std::any is and as
	//
	template<typename T, typename X>
	requires (std::is_same_v<X, std::any> && !std::is_same_v<T, std::any> && !std::is_same_v<T, void>)
	constexpr auto is(X const& x) -> bool
	{
		return x.type() == typeid(T);
	}

	template<typename T, typename X>
		requires (std::is_same_v<X, std::any> && std::is_same_v<T, void>)
	constexpr auto is(X const& x) -> bool
	{
		return !x.has_value();
	}

	template<typename T, typename X>
	requires (!std::is_reference_v<T> && std::is_same_v<X, std::any> && !std::is_same_v<T, std::any>)
	constexpr auto as(X const& x) -> T
	{
		return std::any_cast<T>(x);
	}


	//-------------------------------------------------------------------------------------------------------------
	//  std::optional is and as
	//
	template<typename T, typename X>
	requires std::is_same_v<X, std::optional<T>>
	constexpr auto is(X const& x) -> bool
	{
		return x.has_value();
	}

	template<typename T, typename U>
	requires std::is_same_v<T, void>
	constexpr auto is(std::optional<U> const& x) -> bool
	{
		return !x.has_value();
	}

	template<typename T, typename X>
	requires std::is_same_v<X, std::optional<T>>
	constexpr auto as(X const& x) -> auto&&
	{
		return x.value();
	}

}

namespace ghassanpl
{

	namespace detail
	{
		struct pass_identity
		{
			template <typename T>
			requires (!std::is_same_v<T, pass_identity>)
			T operator* (T&& obj) const { return std::forward<T>(obj); }

			pass_identity operator* (pass_identity&& p) const { return {}; }
		};

		template <typename T>
		requires (!std::is_same_v<T, pass_identity>)
		T operator * (T&& obj, pass_identity&& p) { return std::forward<T>(obj); }

		constexpr size_t not_given = std::numeric_limits<size_t>::max();
	}

	template <typename FUNC, typename... ARGS>
	requires ((sizeof...(ARGS) % 2) == 0)
	constexpr void for_each_pair(FUNC&& f, ARGS&&... args)
	{
		if constexpr (sizeof...(args) > 1)
		{
			[&]<typename T1, typename T2, typename... NEW_ARGS>(T1 && t1, T2 && t2, NEW_ARGS&&... args) {
				f(std::forward<T1>(t1), std::forward<T2>(t2));
				if constexpr (sizeof...(args) > 1)
					for_each_pair(f, std::forward<NEW_ARGS>(args)...);
			}(std::forward<ARGS>(args)...);
		}
	}

	/*
	template <typename... ARGS, typename FUNC>
	constexpr void for_each(FUNC&& f)
	{
		[&] <size_t... Idxs>(std::index_sequence<Idxs...>) {
			([&] <typename T> (size_t i, std::type_identity<T>) {
				if constexpr (std::is_invocable_v<FUNC, size_t, std::type_identity<T>>)
					f(i, std::type_identity<T>{});
				else if constexpr (std::is_invocable_v<FUNC, std::type_identity<T>, size_t>)
					f(std::type_identity<T>{}, i);
				else
					f(std::type_identity<T>{});
			}(Idxs, std::type_identity<ARGS>{}), ...);
		}(std::make_index_sequence<sizeof...(ARGS)>{});
	}
	*/

	namespace detail
	{
		/*
		template<typename T, typename FUNC, size_t... Is>
		void do_for_each_in_tuple(T&& t, FUNC f, std::index_sequence<Is...>)
		{
			if constexpr (std::is_invocable_v<FUNC, size_t, std::type_identity<T>>)
				(f(Is, std::get<Is>(t)), ...);
			else if constexpr (std::is_invocable_v<FUNC, std::type_identity<T>, size_t>)
				(f(std::get<Is>(t), Is), ...);
			else
				(f(std::get<Is>(t)), ...);
		}

		template<typename ARGS, size_t Is, typename FUNC>
		void do_for_each(FUNC f)
		{
			if constexpr (std::is_invocable_v<FUNC, size_t, std::type_identity<ARGS>>)
				(f(std::integral_constant<size_t, Is>, std::type_identity<ARGS>{}), ...);
			else if constexpr (std::is_invocable_v<FUNC, std::type_identity<ARGS>, size_t>)
				(f(std::type_identity<ARGS>{}, std::integral_constant<size_t, Is>), ...);
			else
				(f(std::type_identity<ARGS>{}), ...);
		}

		template<typename ARGS, size_t Is, typename FUNC>
		void do_for_each(FUNC f)
		{
			if constexpr (std::is_invocable_v<FUNC, size_t, std::type_identity<ARGS>>)
				(f(std::integral_constant<size_t, Is>, std::type_identity<ARGS>{}), ...);
			else if constexpr (std::is_invocable_v<FUNC, std::type_identity<ARGS>, size_t>)
				(f(std::type_identity<ARGS>{}, std::integral_constant<size_t, Is>), ...);
			else
				(f(std::type_identity<ARGS>{}), ...);
		}
		*/

		template <size_t INDEX, typename VALUE, typename FUNC>
		constexpr void call(VALUE&& value, FUNC&& func)
		{
			if constexpr (std::is_invocable_v<FUNC, std::integral_constant<size_t, INDEX>, VALUE>)
				func(std::integral_constant<size_t, INDEX>{}, std::forward<VALUE>(value));
			else if constexpr (std::is_invocable_v<FUNC, VALUE, std::integral_constant<size_t, INDEX>>)
				func(std::forward<VALUE>(value), std::integral_constant<size_t, INDEX>{});
			else if constexpr (std::is_invocable_v<FUNC, VALUE, size_t>)
				func(std::forward<VALUE>(value), INDEX);
			else if constexpr (std::is_invocable_v<FUNC, size_t, VALUE>)
				func(INDEX, std::forward<VALUE>(value));
			else if constexpr (std::is_invocable_v<FUNC, VALUE>)
				func(std::forward<VALUE>(value));
			else
				func(INDEX);
		}

		template <size_t INDEX, typename VALUE, typename FUNC>
		constexpr auto call_r(VALUE&& value, FUNC&& func)
		{
			if constexpr (std::is_invocable_v<FUNC, std::integral_constant<size_t, INDEX>, VALUE>)
				return func(std::integral_constant<size_t, INDEX>{}, std::forward<VALUE>(value));
			else if constexpr (std::is_invocable_v<FUNC, VALUE, std::integral_constant<size_t, INDEX>>)
				return func(std::forward<VALUE>(value), std::integral_constant<size_t, INDEX>{});
			else if constexpr (std::is_invocable_v<FUNC, VALUE, size_t>)
				return func(std::forward<VALUE>(value), INDEX);
			else if constexpr (std::is_invocable_v<FUNC, size_t, VALUE>)
				return func(INDEX, std::forward<VALUE>(value));
			else if constexpr (std::is_invocable_v<FUNC, VALUE>)
				return func(std::forward<VALUE>(value));
			else
				return func(INDEX);
		}

		template<typename T, typename FUNC, size_t... Is>
		constexpr void do_for_each_in_tuple(T&& t, FUNC f, std::index_sequence<Is...>)
		{
			(call<Is>(std::get<Is>(t), f), ...);
		}

		template<typename T, typename FUNC, size_t... Is>
		constexpr auto do_transform_tuple(T&& t, FUNC f, std::index_sequence<Is...>)
		{
			return std::make_tuple(call_r<Is>(std::get<Is>(t), f)...);
		}

		template<typename... ARGS, typename FUNC, size_t... Is>
		constexpr void do_for_each(FUNC f, std::index_sequence<Is...>)
		{
			(call<Is>(std::type_identity<ARGS>{}, f), ...);
		}
	}

	template <typename... ARGS, typename FUNC>
	constexpr void for_each(FUNC&& f)
	{
		detail::do_for_each<ARGS...>(f, std::make_index_sequence<sizeof...(ARGS)>());
	}

	template<typename... ARGS, typename FUNC>
	constexpr void for_each_in_tuple(std::tuple<ARGS...> const& t, FUNC&& f)
	{
		detail::do_for_each_in_tuple(t, f, std::make_index_sequence<sizeof...(ARGS)>());
	}

	template<typename... ARGS, typename FUNC>
	constexpr auto transform_tuple(std::tuple<ARGS...> const& t, FUNC&& f)
	{
		return detail::do_transform_tuple(t, f, std::make_index_sequence<sizeof...(ARGS)>());
	}

	template <typename FUNC, typename... ARGS>
	constexpr void enumerate_pack(FUNC&& f, ARGS&&... args)
	{
		/*
		[&] <size_t... Idxs>(std::index_sequence<Idxs...>) {
			([&] <typename T> (size_t i, T && arg) {
				if constexpr (std::is_invocable_v<FUNC, T>)
					f(std::forward<T>(arg));
				else if constexpr (std::is_invocable_v<FUNC, size_t, T>)
					f(i, std::forward<T>(arg));
				else
					static_assert(!std::is_void_v<T>, "Cannot invoke callback with this type");
			}(Idxs, std::forward<ARGS>(args)), ...);
		}(std::make_index_sequence<sizeof...(ARGS)>{});
		*/

		[]<size_t... INDICES, typename FUNC2, typename TUPLE>(FUNC2&& func, std::index_sequence<INDICES...>, TUPLE&& tuple) {
			(detail::call<INDICES>(std::get<INDICES>(tuple), func), ...);
		}(std::forward<FUNC>(f), std::make_index_sequence<sizeof...(ARGS)>{}, std::forward_as_tuple(std::forward<ARGS>(args)...));
		

		//detail::do_enumerate_pack(std::forward<FUNC>(func), std::make_index_sequence<sizeof...(ARGS)>{}, std::forward<ARGS>(args)...);
	}

	template <size_t I, typename FUNC, typename... ARGS>
	requires (I < sizeof...(ARGS))
	constexpr auto apply_to_nth(FUNC&& f, ARGS&&... args)
	{
		return [&] <size_t... Idxs>(std::index_sequence<Idxs...>) {
			return ([&] {
				static_assert(std::is_invocable_v<FUNC, ARGS>, "Cannot invoke callback with this type");
				if constexpr (Idxs == I)
					return f(std::forward<ARGS>(args));
				else
					return detail::pass_identity{};
			}() * ...);
		}(std::make_index_sequence<sizeof...(args)>{});
	}

	namespace detail
	{
		template <size_t, size_t = detail::not_given, size_t = 1, class = std::integral_constant<size_t, 0>>
		struct pack_slicer;

		template <size_t Begin, size_t End, size_t Stride, size_t I>
		struct pack_slicer<Begin, End, Stride, std::integral_constant<size_t, I>>
		{
			static constexpr bool include = I >= Begin && I < End && ((I - Begin) % Stride == 0);

			using next = pack_slicer<Begin, End, Stride, std::integral_constant<size_t, I + 1>>;

			template <typename T, typename... ARGS>
			static constexpr auto make_slice_function(T&& t, ARGS&&... ts)
			{
				return [slicer = next::make_slice_function(std::forward<ARGS>(ts)...), nt = std::forward<T>(t)]<typename CALLBACK>(CALLBACK && cb) mutable {
					if constexpr (include)
						return slicer([t = std::forward<T>(nt), &cb]<typename... NEW_ARGS>(NEW_ARGS&&... sl) mutable { return cb(std::forward<T>(t), std::forward<NEW_ARGS>(sl)...); });
					else
						return slicer([&cb]<typename... NEW_ARGS>(NEW_ARGS&&... sl) mutable { return cb(std::forward<NEW_ARGS>(sl)...); });
				};
			}

			static constexpr auto make_slice_function() { return [](auto&& cb) mutable { return cb(); }; }
		};
	}

	template <size_t Begin, size_t End, size_t Stride, double&..., typename FUNC, typename... ARGS>
	constexpr auto apply_to_slice(FUNC&& callback, ARGS&&... args)
	{
		return ::ghassanpl::detail::pack_slicer<Begin, End, Stride>::make_slice_function(std::forward<ARGS>(args)...)(std::forward<FUNC>(callback));
	}

	template <size_t Begin, size_t End, double&..., typename FUNC, typename... ARGS>
	constexpr auto apply_to_slice(FUNC&& callback, ARGS&&... args)
	{
		return ::ghassanpl::detail::pack_slicer<Begin, End>::make_slice_function(std::forward<ARGS>(args)...)(std::forward<FUNC>(callback));
	}

	template <size_t Begin, double&..., typename FUNC, typename... ARGS>
	constexpr auto apply_to_slice(FUNC&& callback, ARGS&&... args)
	{
		return ::ghassanpl::detail::pack_slicer<Begin>::make_slice_function(std::forward<ARGS>(args)...)(std::forward<FUNC>(callback));
	}

	/// Fun stuff I found no use case for (yet):
	/// - Compile-time sorting parameter packs: https://twitter.com/The_Whole_Daisy/status/1387798831912439809/photo/1
}