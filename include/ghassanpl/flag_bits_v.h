/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "bits.h"

#if !__has_cpp_attribute(nodiscard)
#error "This library requires [[nodiscard]]"
#endif

namespace ghassanpl
{
	template<typename T>
	concept integral_or_enum = bit_integral<T> || std::is_enum_v<T>;

	namespace detail
	{
		template <typename T> 
		requires integral_or_enum<T>
		constexpr auto to_underlying_type(T t) noexcept
		{
			if constexpr (std::is_enum_v<T>)
				return static_cast<std::underlying_type_t<T>>(t);
			else
				return t;
		}

		template <typename INT_TYPE, auto BIT_NUM>
		concept allowed_bit_num = BIT_NUM >= 0 && BIT_NUM < CHAR_BIT * sizeof(INT_TYPE);

		template <typename RESULT_TYPE, auto... VALUES>
		concept valid_flag_bits_v_arguments =
			 bit_integral<RESULT_TYPE> &&
			(integral_or_enum<decltype(VALUES)> && ...) &&
			(detail::allowed_bit_num<RESULT_TYPE, static_cast<decltype(detail::to_underlying_type(VALUES))>(VALUES)> && ...);
	}

	template <typename RESULT_TYPE, auto... VALUES> 
	requires detail::valid_flag_bits_v_arguments<RESULT_TYPE, VALUES...>
	constexpr inline RESULT_TYPE flag_bits_v = ((RESULT_TYPE(1) << (detail::to_underlying_type(VALUES))) | ... | 0);

	template <auto INT_VALUE, auto VALUE>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUE>
	constexpr inline auto is_flag_set_v = (INT_VALUE & flag_bits_v<decltype(INT_VALUE), VALUE>) != 0;

	template <auto INT_VALUE, auto... VALUES>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUES...>
	constexpr inline auto are_any_flags_set_v = (INT_VALUE & flag_bits_v<decltype(INT_VALUE), VALUES...>) != decltype(INT_VALUE){};//((is_flag_set_v<INT_VALUE, VALUES>) || ...);

	template <auto INT_VALUE, auto... VALUES>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUES...>
	constexpr inline auto are_all_flags_set_v = (INT_VALUE & flag_bits_v<decltype(INT_VALUE), VALUES...>) == flag_bits_v<decltype(INT_VALUE), VALUES...>;//((is_flag_set_v<INT_VALUE, VALUES>) && ...);

	template <auto INT_VALUE, auto... VALUES>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUES...>
	constexpr inline auto set_flag_v = INT_VALUE | (flag_bits_v<decltype(INT_VALUE), VALUES...>);

	template <auto INT_VALUE, auto... VALUES>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUES...>
	constexpr inline auto unset_flag_v = INT_VALUE & ~(flag_bits_v<decltype(INT_VALUE), VALUES...>);

	template <auto INT_VALUE, auto... VALUES>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUES...>
	constexpr inline auto toggle_flag_v = INT_VALUE ^ (flag_bits_v<decltype(INT_VALUE), VALUES...>);

	template <auto INT_VALUE, bool TO, auto... VALUES>
	requires detail::valid_flag_bits_v_arguments<decltype(INT_VALUE), VALUES...>
	constexpr inline auto set_flag_to_v = TO ?
		(INT_VALUE | (flag_bits_v<decltype(INT_VALUE), VALUES...>)) :
		(INT_VALUE & ~(flag_bits_v<decltype(INT_VALUE), VALUES...>));
}