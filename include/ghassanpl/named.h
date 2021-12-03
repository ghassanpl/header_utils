/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <algorithm>
#include <iostream>

#if !defined(__cpp_nontype_template_args) || __cpp_nontype_template_args < 201911L
#error "This library requires non-type template parameters with class type support"
#endif

namespace ghassanpl
{

	namespace detail
	{
		template<size_t N>
		struct FixedString
		{
			constexpr FixedString(char const (&s)[N]) { std::copy_n(s, N, this->elems); }
			constexpr std::strong_ordering operator<=>(FixedString const&) const = default;
			char elems[N];
		};
	}

	template <typename T, detail::FixedString PARAMETER>
	struct named
	{
		using base_type = T;
		static constexpr detail::FixedString name = PARAMETER;

		T Value{};

		template <typename... ARGS>
		constexpr explicit named(ARGS&&... args) noexcept(std::is_nothrow_constructible_v<T, ARGS...>) : Value(std::forward<ARGS>(args)...) {}

		constexpr named() noexcept(std::is_nothrow_default_constructible_v<T>) = default;

		constexpr T* operator->() noexcept { return &Value; }
		constexpr T const* operator->() const noexcept { return &Value; }

		constexpr T& get() noexcept { return Value; }
		constexpr T const& get() const noexcept { return Value; }

		template <typename U>
		constexpr U as() noexcept { return static_cast<U>(Value); }

		template <typename U, typename = std::enable_if_t<std::is_convertible_v<T, U>>>
		constexpr explicit operator U() const noexcept(noexcept((U)Value)) { return (U)Value; }

		constexpr explicit operator bool() const noexcept { return Value; }
		constexpr auto operator <=>(named const&) const noexcept = default;

		constexpr auto drop() noexcept(std::is_nothrow_move_constructible_v<T>) { return std::move(Value); }
	};

	template <typename T, detail::FixedString PARAMETER>
	inline std::ostream& operator<<(std::ostream& strm, named<T, PARAMETER> const& val) { return strm << val.Value; }

}