/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <ranges>
#include <stdexcept>
#include <algorithm>
#include "span.h"

namespace ghassanpl
{
	/// \defgroup Ranges Ranges
	/// Ranges and such

	/// \ingroup Ranges
	///@{

	using std::ranges::random_access_range;
	using std::ranges::contiguous_range;
	using std::random_access_iterator;

	/// Helper template to get the value type of a type that decays to a `range`
	/// \ingroup Ranges
	template <typename RANGE>
	using range_value = std::ranges::range_value_t<std::decay_t<RANGE>>;

	/// Helper template to get the iterator type of a type that decays to a `range`
	template <typename RANGE>
	using range_iterator = std::ranges::iterator_t<std::decay_t<RANGE>>;

	/// Concept to check if a function is a predicate on the range elements (i.e. returns boolable)
	template <typename RANGE, typename FUNC>
	concept range_predicate = requires (FUNC func, RANGE range) { { func(*std::ranges::begin(range)) } -> std::convertible_to<bool>; };

	/// Returns whether or not a given integer is a valid index to a random access `range`
	[[nodiscard]] constexpr bool valid_index(random_access_range auto& range, std::integral auto index)
	{
		return index >= 0 && index < std::ranges::size(range);
	}

	/// Returns `index` converted to a valid index into a range of `range_size` as if using modulo arithmetic
	/// \pre range_size > 0
	[[nodiscard]] constexpr auto modulo_index(size_t range_size, std::integral auto index)
	{
		const auto cpp_modulo_sucks = (index % static_cast<decltype(index)>(range_size));
		return (index < 0) ? (range_size + cpp_modulo_sucks) : cpp_modulo_sucks;
	}

	/// Returns an valid index into `range`, created from `index` as if `range` is circular
	/// \pre range must not be empty
	[[nodiscard]] constexpr auto modulo_index(random_access_range auto& range, std::integral auto index)
	{
		const auto range_size = std::ranges::size(range);
		return modulo_index(range_size, index);
	}

	/// Returns a reference to the value at `index` of `range`
	[[nodiscard]] constexpr decltype(auto) at(random_access_range auto& range, std::integral auto index)
	{
		if (!valid_index(range, index))
			throw std::invalid_argument("index");
		return *(std::ranges::begin(range) + index);
	}

	/// Returns a pointer to the value at `index` of `range` if index is valid, else returns null
	[[nodiscard]] constexpr auto at_ptr(random_access_range auto& range, std::integral auto index)
		-> decltype(std::to_address(std::ranges::begin(range) + index))
	{
		if (!valid_index(range, index))
			return nullptr;
		return std::to_address(std::ranges::begin(range) + index);
	}

	/// Returns a reference to the value at \ref modulo_index of `range`
	/// \pre range must not be empty
	[[nodiscard]] constexpr decltype(auto) modulo_at(random_access_range auto& range, std::integral auto index)
	{
		return at(range, modulo_index(range, index)); /// TODO: make sure this is inlined properly 'cause we're quering size a couple of times, for example
	}

	/// Find a value in `range` and returns an index to it
	template <random_access_range RANGE, typename T>
	[[nodiscard]] constexpr auto index_of(RANGE const& range, T&& value) -> std::iter_difference_t<range_iterator<RANGE>>
	requires requires (T value, RANGE range) { { *std::ranges::begin(range) == value } -> std::convertible_to<bool>; }
	{
		const auto it = std::ranges::find(range, std::forward<T>(value));
		if (it == std::ranges::end(range))
			return -1;
		return std::ranges::distance(std::ranges::begin(range), it);
	}

	/// Return the element at `index` or `default_value` if not a valid index
	template <random_access_range RANGE, typename T = range_value<RANGE>>
	[[nodiscard]] constexpr auto at_or_default(RANGE&& range, std::integral auto index, T&& default_value)
	{
		if (!valid_index(range, index)) [[unlikely]] return std::forward<T>(default_value);
		return at(range, index);
	}

	/// Find a value in `range` and returns an index to it
	template <random_access_range RANGE, typename FUNC>
	requires range_predicate<RANGE, FUNC>
	[[nodiscard]] constexpr auto find_index(RANGE const& range, FUNC&& func) -> std::iter_difference_t<range_iterator<RANGE>>
	{
		const auto it = std::ranges::find_if(range, std::forward<FUNC>(func));
		if (it == std::ranges::end(range))
			return -1;
		return std::ranges::distance(std::ranges::begin(range), it);
	}
	
	/// Find a value in `range` and returns a pointer to it, or null if none found
	template <std::ranges::range RANGE, typename FUNC>
	requires range_predicate<RANGE, FUNC>
	[[nodiscard]] constexpr auto find_ptr(RANGE& range, FUNC&& func)
	{
		const auto it = std::ranges::find_if(range, std::forward<FUNC>(func));
		return it == std::ranges::end(range) ? nullptr : std::to_address(it);
	}

	template <random_access_range RANGE, typename FUNC, typename DEF_TYPE = range_value<RANGE>>
	requires range_predicate<RANGE, FUNC>
	[[nodiscard]] auto find_if_or_default(RANGE& range, FUNC&& func, DEF_TYPE&& default_value = DEF_TYPE{})
	{
		auto it = std::ranges::find_if(range, std::forward<FUNC>(func));
		if (it == std::ranges::end(range))
			return range_value<RANGE>{std::forward<DEF_TYPE>(default_value)};
		return *it;
	}

	/// Turns an `iterator` to `range` to an index
	[[nodiscard]] constexpr auto to_index(random_access_iterator auto iterator, random_access_range auto const& range)
	{
		return std::ranges::distance(std::ranges::begin(range), iterator);
	}

	/// Returns whether or not `pointer` is a valid pointer to an element in the contiguous `range`
	template <contiguous_range RANGE>
	[[nodiscard]] constexpr bool valid_address(RANGE&& range, range_value<RANGE>* pointer)
	{
		if (std::ranges::empty(range)) return false;
		return pointer >= std::to_address(std::ranges::begin(range)) && pointer < std::to_address(std::ranges::end(range));
	}

	/// Span stuff
	
	template <typename T, size_t N = std::dynamic_extent>
	[[nodiscard]] constexpr std::pair<span<T>, span<T>> split_at(span<T, N> spn, size_t index)
	{
		if (index >= spn.size())
			return { spn, span<T>{} };
		return { spn.subspan(0, index), spn.subspan(index) };
	}

	template <typename T, size_t N = std::dynamic_extent>
	[[nodiscard]] constexpr std::array<span<T>, 3> split_at(span<T, N> spn, size_t index, size_t size)
	{
		if (index >= spn.size() || index + size >= spn.size())
			return { spn, span<T>{}, span<T>{} };
		return { spn.subspan(0, index), spn.subspan(index, size), spn.subspan(index + size) };
	}
	
	template <typename T, size_t N = std::dynamic_extent>
	[[nodiscard]] constexpr std::pair<T*, T*> as_range(span<T, N> spn)
	{
		return { spn.data(), spn.data() + spn.size() };
	}

	/// Casts a spn of objects of type FROM to a spn of objects of type TO
	/// Does not perform any checks, specifically, no alignment or size checks are performed.
	template <typename TO, typename FROM, size_t N = std::dynamic_extent>
	[[nodiscard]] auto span_cast(span<FROM, N> bytes) noexcept
	{
		return span<TO>{ reinterpret_cast<TO*>(bytes.data()), (bytes.size() * sizeof(FROM)) / sizeof(TO) };
	}

	template <typename T1, size_t N1, typename T2, size_t N2>
	[[nodiscard]] constexpr bool are_adjacent(span<T1, N1> s1, span<T2, N2> s2)
	{
		return reinterpret_cast<char const*>(s1.data() + s1.size()) == reinterpret_cast<char const*>(s2.data())
			|| reinterpret_cast<char const*>(s2.data() + s2.size()) == reinterpret_cast<char const*>(s1.data());
	}

	template <typename T1, size_t N1, typename T2, size_t N2>
	[[nodiscard]] constexpr bool are_overlapping(span<T1, N1> s1, span<T2, N2> s2)
	{
		return valid_address(s1, s2.data()) || valid_address(s2, s1.data());
	}

	template <typename T, size_t N1, typename U, size_t N2>
	requires std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>
	[[nodiscard]] constexpr bool ends_with(span<T, N1> s1, span<U, N2> s2)
	{
		if (s1.size() < s2.size()) return false;
		return std::ranges::equal(s1.subspan(s1.size() - s2.size()), s2);
	}

	template <typename T, size_t N1, typename U, size_t N2>
	requires std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>
	[[nodiscard]] constexpr bool starts_with(span<T, N1> s1, span<U, N2> s2)
	{
		if (s1.size() < s2.size()) return false;
		return std::ranges::equal(s1.subspan(0, s2.size()), s2);
	}

	/// Arrays
	
	template <typename T, std::size_t... Ns>
	[[nodiscard]] constexpr auto join(std::array<T, Ns>... arrays)
	{
		std::array<T, (Ns + ...)> result;
		std::size_t index = 0;
		((std::copy_n(std::make_move_iterator(arrays.begin()), Ns, result.begin() + index), index += Ns), ...);
		return result;
	}

	template<typename T, size_t LL, typename... ARGS>
	requires (std::same_as<std::remove_cvref_t<ARGS>, T> && ...)
	[[nodiscard]] constexpr auto join(std::array<T, LL> rhs, ARGS&&... args)
	{
		std::array<T, LL + sizeof...(ARGS)> ar;
		auto current = std::move(rhs.begin(), rhs.end(), ar.begin());
		((*current++ = args), ...);
		return ar;
	}

#ifndef __cpp_lib_ranges_contains

	struct __contains_fn
	{
		template <std::input_iterator I, std::sentinel_for<I> S, class T, class Proj = std::identity>
		requires std::indirect_binary_predicate<std::ranges::equal_to, std::projected<I, Proj>, const T*>
		constexpr bool operator()(I first, S last, const T& value, Proj proj = {}) const
		{
			return std::ranges::find(std::move(first), last, value, proj) != last;
		}

		template <std::ranges::input_range R, class T, class Proj = std::identity>
		requires std::indirect_binary_predicate<std::ranges::equal_to, std::projected<std::ranges::iterator_t<R>, Proj>, const T*>
		constexpr bool operator()(R&& r, const T& value, Proj proj = {}) const
		{
			return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(value), proj);
		}
	};

	/// contains(range, el)
	constexpr inline __contains_fn contains{};


	struct __contains_subrange_fn
	{
		template <std::forward_iterator I1, std::sentinel_for<I1> S1, std::forward_iterator I2, std::sentinel_for<I2> S2, class Pred = std::ranges::equal_to, class Proj1 = std::identity, class Proj2 = std::identity>
		requires std::indirectly_comparable<I1, I2, Pred, Proj1, Proj2>
		constexpr bool operator()(I1 first1, S1 last1, I2 first2, S2 last2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		{
			return (first2 == last2) || !std::ranges::search(first1, last1, first2, last2, pred, proj1, proj2).empty();
		}

		template <std::ranges::forward_range R1, std::ranges::forward_range R2, class Pred = std::ranges::equal_to, class Proj1 = std::identity, class Proj2 = std::identity>
			requires std::indirectly_comparable<std::ranges::iterator_t<R1>, std::ranges::iterator_t<R2>, Pred, Proj1, Proj2>
		constexpr bool operator()(R1&& r1, R2&& r2, Pred pred = {}, Proj1 proj1 = {}, Proj2 proj2 = {}) const
		{
			return (*this)(std::ranges::begin(r1), std::ranges::end(r1), std::ranges::begin(r2), std::ranges::end(r2), std::move(pred), std::move(proj1), std::move(proj2));
		}
	};


	/// contains_subrange(range, subrange)
	constexpr inline __contains_subrange_fn contains_subrange{};

#endif

#ifndef __cpp_lib_ranges_to_container

	namespace detail
	{
		struct from_range_t { explicit from_range_t() = default; };
		constexpr inline from_range_t from_range;

		template <class RANGE, class CONTAINER>
		concept _Ref_converts = std::convertible_to<std::ranges::range_reference_t<RANGE>, std::ranges::range_value_t<CONTAINER>>;

		template <class RANGE, class CONTAINER, class... TYPES>
		concept _Converts_direct_constructible = _Ref_converts<RANGE, CONTAINER>
			&& std::constructible_from<CONTAINER, RANGE, TYPES...>;

		template <class RANGE, class CONTAINER, class... TYPES>
		concept _Converts_tag_constructible = _Ref_converts<RANGE, CONTAINER>
			&& std::constructible_from<CONTAINER, const from_range_t&, RANGE, TYPES...>;

		template <class RANGE, class CONTAINER, class... TYPES>
		concept _Converts_and_common_constructible = _Ref_converts<RANGE, CONTAINER> && std::ranges::common_range<RANGE> //
			&& std::input_iterator<std::ranges::iterator_t<RANGE>> //
			&& std::constructible_from<CONTAINER, std::ranges::iterator_t<RANGE>, std::ranges::iterator_t<RANGE>, TYPES...>;

		template <class CONTAINER, class REFERENCE>
		concept can_push_back = requires (CONTAINER& container, REFERENCE ref) {
			container.push_back(ref);
		};

		template <class CONTAINER, class REFERENCE>
		concept can_insert_at_end = requires (CONTAINER & container, REFERENCE ref) {
			container.insert(container.end(), ref);
		};

		// clang-format off
		template <class RANGE, class CONTAINER, class... TYPES>
		concept _Converts_constructible_insertable = _Ref_converts<RANGE, CONTAINER>
			&& std::constructible_from<CONTAINER, TYPES...>
			&& (can_push_back<CONTAINER, std::ranges::range_reference_t<RANGE>>
				|| can_insert_at_end<CONTAINER, std::ranges::range_reference_t<RANGE>>);
		// clang-format on

		template <class REFERENCE, class CONTAINER>
		[[nodiscard]] constexpr auto container_inserter(CONTAINER& _Cont)
		{
			if constexpr (can_push_back<CONTAINER, REFERENCE>) {
				return std::back_insert_iterator{ _Cont };
			}
			else {
				return std::insert_iterator{ _Cont, _Cont.end() };
			}
		}

		template <class RANGE, class CONTAINER>
		concept sized_and_reservable =
			std::ranges::sized_range<RANGE> &&
			std::ranges::sized_range<CONTAINER> &&
			requires(CONTAINER & container, const std::ranges::range_size_t<CONTAINER> count) {
			container.reserve(count);
			{ container.capacity() } -> std::same_as<std::ranges::range_size_t<CONTAINER>>;
			{ container.max_size() } -> std::same_as<std::ranges::range_size_t<CONTAINER>>;
		};

		template <std::ranges::input_range RANGE>
		struct phony_input_iterator
		{
			using iterator_category = std::input_iterator_tag;
			using value_type = std::ranges::range_value_t<RANGE>;
			using difference_type = ptrdiff_t;
			using pointer = std::add_pointer_t<std::ranges::range_reference_t<RANGE>>;
			using reference = std::ranges::range_reference_t<RANGE>;

			reference operator*() const;
			pointer operator->() const;

			phony_input_iterator& operator++();
			phony_input_iterator operator++(int);

			bool operator==(const phony_input_iterator&) const;
		};

		template <template <class...> class CONTAINER, class RANGE, class... ARGS>
		auto to_helper()
		{
			if constexpr (requires (RANGE range, ARGS... args) { CONTAINER(range, args...); })
				return static_cast<decltype(CONTAINER(std::declval<RANGE>(), std::declval<ARGS>()...))*>(nullptr);
			else if constexpr (requires (RANGE range, ARGS... args) { CONTAINER(from_range, range, args...); })
				return static_cast<decltype(CONTAINER(from_range, std::declval<RANGE>(), std::declval<ARGS>()...))*>(nullptr);
			else if constexpr (requires (phony_input_iterator<RANGE> it, ARGS... args) { CONTAINER(it, it, args...);})
				return static_cast<decltype(CONTAINER(std::declval<phony_input_iterator<RANGE>>(), std::declval<phony_input_iterator<RANGE>>(), std::declval<ARGS>()...))*>(nullptr);
		}

	}

	/// to<container>();
	template <class CONTAINER, std::ranges::input_range RANGE, class... TYPES>
	requires (!std::ranges::view<CONTAINER>)
	[[nodiscard]] constexpr CONTAINER to(RANGE&& range, TYPES&&... args)
	{
		using namespace detail;
		if constexpr (_Converts_direct_constructible<RANGE, CONTAINER, TYPES...>)
			return CONTAINER(std::forward<RANGE>(range), std::forward<TYPES>(args)...);
		else if constexpr (_Converts_tag_constructible<RANGE, CONTAINER, TYPES...>)
			return CONTAINER(from_range, std::forward<RANGE>(range), std::forward<TYPES>(args)...);
		else if constexpr (_Converts_and_common_constructible<RANGE, CONTAINER, TYPES...>)
			return CONTAINER(std::ranges::begin(range), std::ranges::end(range), std::forward<TYPES>(args)...);
		else if constexpr (_Converts_constructible_insertable<RANGE, CONTAINER, TYPES...>)
		{
			CONTAINER container(std::forward<TYPES>(args)...);
			if constexpr (sized_and_reservable<RANGE, CONTAINER>) {
				container.reserve(std::ranges::size(range));
			}
			std::ranges::copy(range, detail::container_inserter<std::ranges::range_reference_t<RANGE>>(container));
			return container;
		}
		else if constexpr (std::ranges::input_range<std::ranges::range_reference_t<RANGE>>)
		{
			const auto transform = [](auto&& _Elem) {
				return ghassanpl::to<std::ranges::range_value_t<CONTAINER>>(std::forward<decltype(_Elem)>(_Elem));
			};
			return ghassanpl::to<CONTAINER>(std::ranges::views::transform(range, transform), std::forward<TYPES>(args)...);
		}
		else
			static_assert(always_false<CONTAINER>, "the program is ill-formed per N4910 [range.utility.conv.to]/1.3");
	}

	/// to<container>();
	template <template <class...> class CONTAINER, std::ranges::input_range RANGE, class... TYPES, class _Deduced = std::remove_pointer_t<decltype(detail::to_helper<CONTAINER, RANGE, TYPES...>())>>
	[[nodiscard]] constexpr _Deduced to(RANGE&& _Range, TYPES&&... ARGS)
	{
		return to<_Deduced>(std::forward<RANGE>(_Range), std::forward<TYPES>(ARGS)...);
	}

	///@}
#endif

	/*
	template <class _Ty = void>
	struct plus_equals
	{
		[[nodiscard]] constexpr _Ty operator()(_Ty& left, const _Ty& right) const { return left += right; }
	};

	template <>
	struct plus_equals<void>
	{
		template <class _Ty1, class _Ty2>
		[[nodiscard]] constexpr auto operator()(_Ty1& _Left, _Ty2&& _Right) const noexcept(noexcept(static_cast<_Ty1&>(_Left) + static_cast<_Ty2&&>(_Right)))
			-> decltype(static_cast<_Ty1&>(_Left) + static_cast<_Ty2&&>(_Right))
		{
			return static_cast<_Ty1&>(_Left) + static_cast<_Ty2&&>(_Right);
		}
		using is_transparent = int;
	};

	template <std::ranges::range RANGE, std::movable FOLD_VALUE, typename FOLD_FUNC = std::plus<>>
	requires std::is_invocable_v<FOLD_FUNC, FOLD_VALUE&, std::ranges::range_reference_t<RANGE>>
	auto fold(RANGE&& r, FOLD_VALUE init, FOLD_FUNC&& op = {})
	{
		auto first = std::ranges::begin(r);
		auto const last = std::ranges::end(r);
		for (; first != last; ++first)
			init = std::invoke(op, std::move(init), *first);
		return init;
	}

	template <std::ranges::range RANGE, typename FOLD_VALUE, typename FOLD_FUNC = plus_equals<>>
	decltype(auto) fold_while(RANGE&& r, FOLD_VALUE&& init, FOLD_FUNC&& op = {})
	requires std::is_invocable_r_v<bool, FOLD_FUNC, FOLD_VALUE&, std::ranges::range_reference_t<RANGE>>
	{
		auto first = std::ranges::begin(r);
		auto const last = std::ranges::end(r);
		for (; first != last; ++first)
		{
			if (!std::invoke(op, std::ref(init), *first))
				break;
		}
		return init;
	}
	*/
}