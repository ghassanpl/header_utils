/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include "cpp20.h"

namespace ghassanpl
{

	template <typename T, T SENTINEL = T{}>
	struct sentinel_optional
	{
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

		inline constexpr void assert_value() const { if (this->m_value == SENTINEL) throw std::bad_optional_access(); }

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

		constexpr explicit operator bool() const noexcept { return this->m_value != SENTINEL; }
		[[nodiscard]] constexpr bool has_value() const noexcept { return this->m_value != SENTINEL; }

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

		[[nodiscard]] constexpr const T& raw_value() const & {
			return this->m_value;
		}
		[[nodiscard]] constexpr T& raw_value() & {
			return this->m_value;
		}
		[[nodiscard]] constexpr T&& raw_value() && {
			return std::move(this->m_value);
		}
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

		[[nodiscard]] constexpr bool operator==(const sentinel_optional& other) const { return this->m_value == other.m_value; }
		[[nodiscard]] constexpr bool operator!=(const sentinel_optional& other) const { return this->m_value != other.m_value; }
		[[nodiscard]] constexpr bool operator>=(const sentinel_optional& other) const { return this->m_value >= other.m_value; }
		[[nodiscard]] constexpr bool operator>(const sentinel_optional& other) const { return this->m_value > other.m_value; }
		[[nodiscard]] constexpr bool operator<=(const sentinel_optional& other) const { return this->m_value <= other.m_value; }
		[[nodiscard]] constexpr bool operator<(const sentinel_optional& other) const { return this->m_value < other.m_value; }

	private:

		T m_value = SENTINEL;
	};

	template <class T>
	constexpr bool operator==(const sentinel_optional<T>& left, std::nullopt_t) noexcept { return !left.has_value(); }

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
}
