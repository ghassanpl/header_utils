/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <bit>
#include "min-cpp-version/cpp20.h"

namespace ghassanpl
{
	/// \defgroup soptional Sentinel Optional
	/// Contains `sentinel_optional`
	/// @{

	// TODO: 
	// ```c++
	// struct sentinel_struct { 
	//		static bool operator()(T const& v) const { return v == sentinel_value; }   // check func
	//		static void operator()(T& v) const { v = sentinel_value; }                 // reset func
	// };
	// template <typename T, typename SENTINEL_STRUCT>
	// struct sentinel_optional;
	// ```
	// for types that cannot be template arguments.

	namespace detail
	{
		struct better_equal_to
		{
			template <class T, class U>
			[[nodiscard]] constexpr auto operator()(T&& t, U&& u) const
			{
				/// This is a hack that ensures NAN can be a valid sentinel value
				if constexpr (std::is_floating_point_v<std::remove_cvref_t<T>> && std::is_floating_point_v<std::remove_cvref_t<U>>)
					return std::bit_cast<uint64_t>(double(t)) == std::bit_cast<uint64_t>(double(u));
				else
					return std::forward<T>(t) == std::forward<U>(u);
			}

			using is_transparent = int;
		};
	}

	/// An equivalent to `std::optional<T>` except that instead of keeping an additional `bool` to indicate value presence, it uses one of the
	/// potential values of `T` as a "sentinel" value to indicate `nullopt`. This makes the value the same size/alignment as `T` while still keeping the `optional` API.
	/// Mostly useful for enums or special types, somewhat useful for floating-points, sometimes useful for pointers.
	/// 
	/// \note This TECHNICALLY could work with C++17, but its only practically useful with C++20 since `SENTINEL` can be of more types, especially floats.
	/// \tparam T the value you want to hold
	/// \tparam SENTINEL a value that will represent the nullopt; needs to be assignable to T, and comparable with T using the EQ functor type
	/// \tparam EQ a functor type whose `operator()` will tell us if `value == SENTINEL`; exists because some types, like floats, cannot be straight-compared with NAN as a sentinel value
	template <typename T, auto SENTINEL = T{}, typename EQ = detail::better_equal_to>
	struct sentinel_optional
	{
		using sentinel_type = decltype(SENTINEL);

		static_assert(EQ{}(SENTINEL, SENTINEL), "Sentinel value must equal itself under the equal operation given in the template parameter EQ");

		constexpr sentinel_optional() noexcept {}
		constexpr sentinel_optional(std::nullopt_t) noexcept {}

		template <class... TYPES, std::enable_if_t<std::is_constructible_v<T, TYPES...>, int> = 0>
		constexpr explicit sentinel_optional(std::in_place_t, TYPES&&... args) noexcept(std::is_nothrow_constructible_v<T, TYPES...>)
			: m_value(std::forward<TYPES>(args)...)
		{
		}

		template <class U = T, std::enable_if_t<
			!std::is_same_v<remove_cvref_t<U>, sentinel_optional> &&
			!std::is_same_v<remove_cvref_t<U>, std::in_place_t> &&
			!std::is_same_v<std::remove_cv_t<U>, bool> &&
			std::is_constructible_v<T, U>
			, int> = 0>
		constexpr sentinel_optional(U&& right) noexcept(std::is_nothrow_constructible_v<T, U>)
			: m_value(std::forward<U>(right))
		{
		}

		template <class U, std::enable_if_t<std::is_constructible_v<T, const U&>, int> = 0>
		constexpr explicit sentinel_optional(const sentinel_optional<U>& right) noexcept(std::is_nothrow_constructible_v<T, const U&>)
		{
			if (right)
				this->m_value = *right;
		}

		template <class U, std::enable_if_t<std::is_constructible_v<T, U>, int> = 0>
		constexpr explicit sentinel_optional(sentinel_optional<U>&& right) noexcept(std::is_nothrow_constructible_v<T, U>)
		{
			if (right)
				this->m_value = std::move(*right);
		}

		constexpr sentinel_optional& operator=(std::nullopt_t) noexcept {
			reset();
			return *this;
		}

		template <class U = T, std::enable_if_t<
			!std::is_same_v<sentinel_optional, remove_cvref_t<U>> && 
			std::is_constructible_v<T, U> && 
			std::is_assignable_v<T&, U>,
			int> = 0>
		constexpr sentinel_optional& operator=(U&& right) noexcept(std::is_nothrow_assignable_v<T&, U> && std::is_nothrow_constructible_v<T, U>)
		{
			this->m_value = std::forward<U>(right);
			return *this;
		}

		template <class U, std::enable_if_t<
			!(
				std::is_same_v<T, U> || 
				std::is_assignable_v<T&, sentinel_optional<U>&> ||
				std::is_assignable_v<T&, const sentinel_optional<U>&> ||
				std::is_assignable_v<T&, const sentinel_optional<U>> ||
				std::is_assignable_v<T&, sentinel_optional<U>>
			) &&
			std::is_constructible_v<T, const U&> && 
			std::is_assignable_v<T&, const U&>,
			int> = 0>
		constexpr sentinel_optional& operator=(const sentinel_optional<U>& right) noexcept(std::is_nothrow_assignable_v<T&, const U&> && std::is_nothrow_constructible_v<T, const U&>) 
		{
			if (right)
				this->m_value = *right;
			else
				reset();

			return *this;
		}

		template <class U, std::enable_if_t<
			!(
				std::is_same_v<T, U> ||
				std::is_assignable_v<T&, sentinel_optional<U>&> ||
				std::is_assignable_v<T&, const sentinel_optional<U>&> ||
				std::is_assignable_v<T&, const sentinel_optional<U>> ||
				std::is_assignable_v<T&, sentinel_optional<U>>
				) &&
			std::is_constructible_v<T, U> &&
			std::is_assignable_v<T&, U>,
			int> = 0>
		constexpr sentinel_optional& operator=(sentinel_optional<U>&& right) noexcept(std::is_nothrow_assignable_v<T&, U>&& std::is_nothrow_constructible_v<T, U>)
		{
			if (right)
				this->m_value = std::move(*right);
			else
				reset();

			return *this;
		}

		template <class... TYPES>
		constexpr T& emplace(TYPES&&... args) noexcept(std::is_nothrow_constructible_v<T, TYPES...>) 
		{
			reset();
			return this->m_value = T(std::forward<TYPES>(args)...);
		}

		inline constexpr void assert_value() const { if (!has_value()) throw std::bad_optional_access(); }

		[[nodiscard]] constexpr const T* operator->() const
		{
			this->assert_value();
			return std::addressof(this->m_value);
		}
		[[nodiscard]] constexpr T* operator->()
		{
			this->assert_value();
			return std::addressof(this->m_value);
		}

		[[nodiscard]] constexpr T& operator*() &
		{
			this->assert_value();
			return this->m_value;
		}

		[[nodiscard]] constexpr const T& operator*() const&
		{
			this->assert_value();
			return this->m_value;
		}

		[[nodiscard]] constexpr T&& operator*() &&
		{
			this->assert_value();
			return std::move(this->m_value);
		}

		[[nodiscard]] constexpr const T&& operator*() const&&
		{
			this->assert_value();
			return std::move(this->m_value);
		}

		[[nodiscard]] constexpr bool has_value() const noexcept { return !EQ{}(this->m_value, SENTINEL); }
		constexpr explicit operator bool() const noexcept { return has_value(); }

		[[nodiscard]] constexpr const T& value() const& {
			this->assert_value();
			return this->m_value;
		}
		[[nodiscard]] constexpr T& value()& {
			this->assert_value();
			return this->m_value;
		}
		[[nodiscard]] constexpr T&& value()&& {
			this->assert_value();
			return std::move(this->m_value);
		}
		[[nodiscard]] constexpr const T&& value() const&& {
			this->assert_value();
			return std::move(this->m_value);
		}

		/// Returns the value without asserting `has_value()`
		[[nodiscard]] constexpr const T& raw_value() const & {
			return this->m_value;
		}
		/// \copydoc raw_value
		[[nodiscard]] constexpr T& raw_value() & {
			return this->m_value;
		}
		/// \copydoc raw_value
		[[nodiscard]] constexpr T&& raw_value() && {
			return std::move(this->m_value);
		}
		/// \copydoc raw_value
		[[nodiscard]] constexpr const T&& raw_value() const && {
			return std::move(this->m_value);
		}

		/*
		template <class _Ty2>
		[[nodiscard]] constexpr remove_cv_t<T> value_or(_Ty2&& _Right) const& {
			static_assert(is_convertible_v<const T&, remove_cv_t<T>>,
				"The const overload of optional<T>::value_or requires const T& to be convertible to remove_cv_t<T> "
				"(N4950 [optional.observe]/15 as modified by LWG-3424).");
			static_assert(is_convertible_v<_Ty2, T>,
				"optional<T>::value_or(U) requires U to be convertible to T (N4950 [optional.observe]/15).");

			if (this->has_value()) {
				return static_cast<const T&>(this->m_value);
			}

			return static_cast<remove_cv_t<T>>(std::forward<_Ty2>(_Right));
		}
		template <class _Ty2>
		[[nodiscard]] constexpr remove_cv_t<T> value_or(_Ty2&& _Right)&& {
			static_assert(is_convertible_v<T, remove_cv_t<T>>,
				"The rvalue overload of optional<T>::value_or requires T to be convertible to remove_cv_t<T> "
				"(N4950 [optional.observe]/17 as modified by LWG-3424).");
			static_assert(is_convertible_v<_Ty2, T>,
				"optional<T>::value_or(U) requires U to be convertible to T (N4950 [optional.observe]/17).");

			if (this->has_value()) {
				return static_cast<T&&>(this->m_value);
			}

			return static_cast<remove_cv_t<T>>(std::forward<_Ty2>(_Right));
		}
		*/

		constexpr void reset() noexcept { this->m_value = SENTINEL; }

		[[nodiscard]] friend constexpr bool operator==(const sentinel_optional& lhs, const sentinel_optional& rhs)
		{
			return lhs.has_value() != rhs.has_value() ? false : (lhs.has_value() == false ? true : lhs.m_value == rhs.m_value);
		}
		[[nodiscard]] friend constexpr bool operator!=(const sentinel_optional& lhs, const sentinel_optional& rhs)
		{
			return lhs.has_value() != rhs.has_value() ? true : (lhs.has_value() == false ? false : lhs.m_value != rhs.m_value);
		}
		[[nodiscard]] friend constexpr bool operator<(const sentinel_optional& lhs, const sentinel_optional& rhs)
		{
			return !rhs ? false : (!lhs ? true : *lhs < *rhs);
		}
		[[nodiscard]] friend constexpr bool operator<=(const sentinel_optional & lhs, const sentinel_optional & rhs)
		{
			return !lhs ? true : (!rhs ? false : *lhs <= *rhs);
		}
		[[nodiscard]] friend constexpr bool operator>(const sentinel_optional& lhs, const sentinel_optional & rhs)
		{
			return !lhs ? false : (!rhs ? true : *lhs > *rhs);
		}
		[[nodiscard]] friend constexpr bool operator>=(const sentinel_optional& lhs, const sentinel_optional& rhs)
		{
			return !rhs ? true : (!lhs ? false : *lhs >= *rhs);
		}
		[[nodiscard]] friend constexpr auto operator<=>(const sentinel_optional& lhs, const sentinel_optional& rhs)
		{
			return lhs && rhs ? *lhs <=> *rhs : lhs.has_value() <=> rhs.has_value();
		}

		[[nodiscard]] friend constexpr bool operator==(const sentinel_optional& opt, std::nullopt_t) { return !opt; }
		[[nodiscard]] friend constexpr auto operator<=>(const sentinel_optional& opt, std::nullopt_t) { return opt.has_value() <=> false; }

	private:

		T m_value = SENTINEL;
	};

	template <class T, std::enable_if_t<std::is_constructible_v<std::decay_t<T>, T>, int> = 0>
	[[nodiscard]] constexpr sentinel_optional<std::decay_t<T>> make_sentinel_optional(T&& value) noexcept(noexcept(sentinel_optional<std::decay_t<T>>{std::forward<T>(value)})) 
	{
		return sentinel_optional<std::decay_t<T>>{std::forward<T>(value)};
	}
	template <class T, class... TYPES, std::enable_if_t<std::is_constructible_v<T, TYPES...>, int> = 0>
	[[nodiscard]] constexpr sentinel_optional<T> make_optional(TYPES&&... args) noexcept(noexcept(sentinel_optional<T>{std::in_place, std::forward<TYPES>(args)...}))
	{
		return sentinel_optional<T>{std::in_place, std::forward<TYPES>(args)...};
	}

	/// @}
}
