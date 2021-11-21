/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "flag_bits_v.h"

namespace ghassanpl
{
	namespace detail
	{
		template<typename T>
		concept valid_integral = bit_integral<T> || requires (T other, int t) {
			{ other &= t } -> bit_integral;
			{ other ^= t } -> bit_integral;
			{ other |= t } -> bit_integral;
			{ other & t } -> bit_integral;
			{ other ^ t } -> bit_integral;
			{ other | t } -> bit_integral;
		};

		template <typename RESULT_TYPE, typename... ENUM_TYPES>
		concept valid_flag_bits_arguments = detail::valid_integral<RESULT_TYPE> && (detail::integral_or_enum<ENUM_TYPES> && ...);
	}

	template <typename RESULT_TYPE = unsigned long long, typename... ARGS>
	constexpr RESULT_TYPE flag_bits(ARGS... args) noexcept
	requires detail::valid_flag_bits_arguments<RESULT_TYPE, ARGS...>
	{
		return ((RESULT_TYPE{ 1 } << detail::to_underlying_type(args)) | ... | 0);
	}

	template <typename INTEGRAL, typename ENUM_TYPE>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ENUM_TYPE>
	constexpr bool is_flag_set(INTEGRAL const& bits, ENUM_TYPE flag) noexcept { return (bits & flag_bits<INTEGRAL>(flag)) != 0; }

	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr bool are_any_flags_set(INTEGRAL const& bits, ARGS... args) noexcept
	{
		return (bits & flag_bits(args...)) != 0;
	}

	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr bool are_all_flags_set(INTEGRAL const& bits, ARGS... args) noexcept
	{
		return (bits & flag_bits(args...)) == flag_bits(args...);
	}

	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void set_flags(INTEGRAL& bits, ARGS... args) noexcept { bits |= flag_bits<INTEGRAL>(args...); }
	
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void unset_flags(INTEGRAL& bits, ARGS... args) noexcept { bits &= ~ flag_bits<INTEGRAL>(args...); }
	
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void toggle_flags(INTEGRAL& bits, ARGS... args) noexcept { bits ^= flag_bits<INTEGRAL>(args...); }
	
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void set_flags_to(INTEGRAL& bits, bool val, ARGS... args) noexcept
	{
		if (val)
			bits |= flag_bits<INTEGRAL>(args...);
		else
			bits &= ~ flag_bits<INTEGRAL>(args...);
	}
}