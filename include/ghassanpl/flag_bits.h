/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "flag_bits_v.h"

namespace ghassanpl
{
	/// \addtogroup Flags
	/// Functions and types for manipulations of enum values (not representing actual bit values, just bit numbers) as bit numbers in other integers.
	/// As well as a generic \ref enum_flags type that represents a set of enum values.
	
	/// \addtogroup FlagBits Flag Bits
	/// \ingroup Flags
	/// @{

	namespace detail
	{
		template<typename T>
		concept bit_integral = std::is_integral_v<T> && !std::is_same_v<std::decay_t<T>, bool>;

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
		concept valid_flag_bits_arguments = detail::valid_integral<RESULT_TYPE> && (integral_or_enum<ENUM_TYPES> && ...);
	}

	/// Takes a list of enum values that represent bit numbers (**not actual bit values**)
	/// and returns a value with all the chosen bits set.
	/// 
	/// \par Example
	/// \code
	/// enum class Flags {
	///   Parity,
	///   Zero,
	///   Positive,
	/// };
	/// 
	/// flag_bits(Flags::Parity, Flags::Positive) -> 3
	/// 
	/// \endcode
	/// 
	/// \tparam RESULT_TYPE the type of the result value, `unsigned long long` by default
	template <typename RESULT_TYPE = unsigned long long, typename... ARGS>
	constexpr RESULT_TYPE flag_bits(ARGS... args) noexcept
	requires detail::valid_flag_bits_arguments<RESULT_TYPE, ARGS...>
	{
		return ((RESULT_TYPE{ 1 } << detail::to_underlying_type(args)) | ... | 0);
	}

	/// Checks if an integral value has the bit at number represented by `flag` set
	template <typename INTEGRAL, typename ENUM_TYPE>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ENUM_TYPE>
	constexpr bool is_flag_set(INTEGRAL const& bits, ENUM_TYPE flag) noexcept { return (bits & flag_bits<INTEGRAL>(flag)) != 0; }

	/// Checks if an integral value has **any** of the bits at numbers represented by `args` set
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr bool are_any_flags_set(INTEGRAL const& bits, ARGS... args) noexcept
	{
		return (bits & flag_bits(args...)) != 0;
	}

	/// Checks if an integral value has **all** of the bits at numbers represented by `args` set
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr bool are_all_flags_set(INTEGRAL const& bits, ARGS... args) noexcept
	{
		return (bits & flag_bits(args...)) == flag_bits(args...);
	}

	/// Sets the bits at numbers represented by `args` in `bits`
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void set_flags(INTEGRAL& bits, ARGS... args) noexcept { bits |= flag_bits<INTEGRAL>(args...); }
	
	/// Unsets the bits at numbers represented by `args` in `bits`
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void unset_flags(INTEGRAL& bits, ARGS... args) noexcept { bits &= ~ flag_bits<INTEGRAL>(args...); }
	
	/// Toggles the bits at numbers represented by `args` in `bits`
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void toggle_flags(INTEGRAL& bits, ARGS... args) noexcept { bits ^= flag_bits<INTEGRAL>(args...); }
	
	/// Sets the bits at numbers represented by `args` in `bits` to either 0 or 1, depending on `val`
	template <typename INTEGRAL, typename... ARGS>
	requires detail::valid_flag_bits_arguments<INTEGRAL, ARGS...>
	constexpr void set_flags_to(INTEGRAL& bits, bool val, ARGS... args) noexcept
	{
		if (val)
			bits |= flag_bits<INTEGRAL>(args...);
		else
			bits &= ~ flag_bits<INTEGRAL>(args...);
	}

	///@}
}