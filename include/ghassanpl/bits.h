/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <bit>
#include <stdexcept>

#include "cpp23.h"

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif

namespace ghassanpl
{
	/// \addtogroup Bits
	/// Types and functions for retrieving and manipulating bits in integral values
	/// @{

	/// Whether or not a type is integral (but not a bool).
	/// \ingroup Bits
	/// \par Rationale
	/// bool being integral is basically a remnant of the old days. Its size is implementation defined, and giving it any values except true (1) or false (0) is pretty much undefined behavior.
	/// Therefore the rest of this code uses bit_integral to detect values for which manipulating bits is well defined (that actually represent and are meant to model integers).
	template<typename T>
	concept bit_integral = std::is_integral_v<T> && !std::is_same_v<std::decay_t<T>, bool>;

	/// Equal to the number of bits in the type
	/// \ingroup Bits
	template <typename T>
	constexpr inline auto bit_count = sizeof(T) * CHAR_BIT;

	/// A value of type `uint64_t` with all bits set
	constexpr inline uint64_t all_bits = ~uint64_t{ 0 };

	/// Value with bits between BEGIN and END (exclusive) set
	template <size_t BEGIN, size_t END>
	constexpr inline uint64_t bit_mask_v = (all_bits >> (64 - END)) << BEGIN;

	/// Value with all bits available for the FOR type set (e.g. first 8 bits for uint8_t will be set, etc.)
	template <bit_integral FOR>
	constexpr inline uint64_t bit_mask_for_v = (all_bits >> (64 - bit_count<FOR>));

	template <typename TO, typename FROM>
	concept bit_castable = std::is_trivially_copyable_v<TO> && std::is_trivially_copyable_v<FROM> && sizeof(TO) == sizeof(FROM);

	namespace detail
	{
		template <size_t N>
		struct uintN_t_t;
		template <> struct uintN_t_t<8> { using value = uint8_t; };
		template <> struct uintN_t_t<16> { using value = uint16_t; };
		template <> struct uintN_t_t<32> { using value = uint32_t; };
		template <> struct uintN_t_t<64> { using value = uint64_t; };
		template <size_t N>
		requires (N < 8)
		struct uintN_t_t<N>
		{
			using value = uint8_t;
		};
	}

	/// An unsigned integer type for the given PoT bit size. \hideinitializer
	template <size_t N>
	using uintN_t = typename detail::uintN_t_t<N>::value;

	namespace detail
	{
		template <size_t N>
		struct intN_t_t
		{
			using value = std::make_signed_t<uintN_t<N>>;
		};
	}

	/// An integer type for the given PoT bit size. \hideinitializer
	template <size_t N>
	using intN_t = typename detail::intN_t_t<N>::value;

	namespace detail
	{
		template <bool SIGNED, size_t N>
		struct sintN_t_t;
		template <size_t N> struct sintN_t_t<false, N> { using value = uintN_t<N>; };
		template <size_t N> struct sintN_t_t<true, N> { using value = intN_t<N>; };
	}

	/// A signed integer type for the given PoT bit size. \hideinitializer
	template <bool SIGNED, size_t N>
	using sintN_t = typename detail::sintN_t_t<SIGNED, N>::value;

	/// Returns an integer with the N/2 most significant bits of the given N-bit integer
	template <bit_integral T>
	[[nodiscard]] constexpr auto most_significant_half(T v) noexcept
	{
		constexpr auto bit_count = ghassanpl::bit_count<T>;
		constexpr auto bit_count_half = bit_count / 2;
		return static_cast<sintN_t<std::is_signed_v<T>, bit_count_half>>((v >> bit_count_half) & bit_mask_v<0, bit_count_half>);
	}

	/// Returns an integer with the N/2 least significant bits of the given N-bit integer
	template <bit_integral T>
	[[nodiscard]] constexpr auto least_significant_half(T v) noexcept
	{
		constexpr auto bit_count = ghassanpl::bit_count<T>;
		constexpr auto bit_count_half = bit_count / 2;
		return static_cast<sintN_t<std::is_signed_v<T>, bit_count_half>>(v & bit_mask_v<0, bit_count_half>);
	}

	template <bit_integral T>
	[[nodiscard]] constexpr auto split_bits(T v) noexcept
	{
		return std::make_pair(most_significant_half(v), least_significant_half(v));
	}

	/// \name Endianness
	/// @{

	/// Returns `val` in its big-endian representation
	template <bit_integral B>
	[[nodiscard]] constexpr B to_big_endian(B val) noexcept { if constexpr (std::endian::native == std::endian::big) return val; else return byteswap(val); }

	/// Returns `val` in its big-endian representation
	template <bit_integral B>
	[[nodiscard]] constexpr B to_little_endian(B val) noexcept { if constexpr (std::endian::native == std::endian::little) return val; else return byteswap(val); }

	/// Returns `val` in its `ENDIANNESS` representation
	template <std::endian ENDIANNESS, bit_integral B>
	[[nodiscard]] constexpr B to_endian(B val) noexcept { if constexpr (std::endian::native == ENDIANNESS) return val; else return byteswap(val); }

	/// Returns `val` in its `endianness` representation
	template <bit_integral B>
	[[nodiscard]] constexpr B to_endian(B val, std::endian endianness) noexcept { if (std::endian::native == endianness) return val; else return byteswap(val); }

	/// @}

	/// Used to specify that a \c ghassanpl::bit_reference references a bit number given at runtime
	constexpr inline size_t dynamic_bit_number = size_t(-1);

	namespace detail
	{
		template <size_t NUM>
		using bit_num_t = std::integral_constant<size_t, NUM>;

		template <size_t NUM>
		constexpr inline auto bit_num = bit_num_t<NUM>{};

		template <bit_integral VALUE_TYPE, size_t BIT_NUM>
		struct bit_num_base
		{
			using unsigned_value_type = std::make_unsigned_t<VALUE_TYPE>;
		protected:
			static constexpr VALUE_TYPE m_bit_mask = std::bit_cast<VALUE_TYPE>(unsigned_value_type(unsigned_value_type(1) << BIT_NUM % bit_count<VALUE_TYPE>));
		};

		template <bit_integral VALUE_TYPE>
		struct bit_num_base<VALUE_TYPE, dynamic_bit_number>
		{
			using unsigned_value_type = std::make_unsigned_t<VALUE_TYPE>;
		protected:
			constexpr bit_num_base(size_t bitnum) noexcept : m_bit_mask(std::bit_cast<VALUE_TYPE>(unsigned_value_type(unsigned_value_type(1) << bitnum % bit_count<VALUE_TYPE>))) {}
			VALUE_TYPE m_bit_mask;
		};
	}

	/// Models a reference to a specific bit in a variable. Can be a statically-defined bit number (BIT_NUM != dynamic_bit_number)
	template <bit_integral VALUE_TYPE, size_t BIT_NUM = dynamic_bit_number>
	struct bit_reference : public detail::bit_num_base<VALUE_TYPE, BIT_NUM>
	{
		using base_type = detail::bit_num_base<VALUE_TYPE, BIT_NUM>;
		static constexpr size_t value_bit_count = bit_count<VALUE_TYPE>;

		static constexpr bool is_static = BIT_NUM != dynamic_bit_number;
		static constexpr bool is_const = std::is_const_v<VALUE_TYPE>;

		constexpr bit_reference(VALUE_TYPE& ref, size_t bitnum) requires (!is_static)
			: base_type(bitnum)
			, m_value_ref(ref)
		{
			if (bitnum >= value_bit_count)
				throw std::invalid_argument("bit_num can't be greater than or equal to the number of bits in the value type");
		}

		constexpr bit_reference(VALUE_TYPE& ref, detail::bit_num_t<BIT_NUM> = {}) noexcept requires is_static
			: m_value_ref(ref)
		{
			static_assert(BIT_NUM < value_bit_count, "BIT_NUM template argument can't be greater than or equal to the number of bits in the value type");
		}

		constexpr bit_reference& operator=(bool val) noexcept requires (!is_const)
		{
			if (val)
				m_value_ref |= this->m_bit_mask;
			else
				m_value_ref &= ~this->m_bit_mask;
			return *this;
		}

		[[nodiscard]] explicit constexpr operator bool() const noexcept
		{
			return (m_value_ref & this->m_bit_mask) != 0;
		}

		template <bit_integral OTHER_VALUE_TYPE, size_t OTHER_BIT_NUM>
		[[nodiscard]] constexpr bool operator==(bit_reference<OTHER_VALUE_TYPE, OTHER_BIT_NUM> const& other) const noexcept
		{
			return this->operator bool() == other.operator bool();
		}

		template <bit_integral OTHER_VALUE_TYPE, size_t OTHER_BIT_NUM>
		[[nodiscard]] constexpr auto operator<=>(bit_reference<OTHER_VALUE_TYPE, OTHER_BIT_NUM> const& other) const noexcept
		{
			return this->operator bool() <=> other.operator bool();
		}

		/// Return the value of the referenced variable
		[[nodiscard]] constexpr auto& integer_value() const noexcept { return m_value_ref; }

		/// Returns the bit number of the referenced bit
		[[nodiscard]] constexpr size_t bit_number() const noexcept
		{
			if constexpr (is_static)
				return BIT_NUM; 
			else
				return std::countr_zero(std::bit_cast<std::make_unsigned_t<VALUE_TYPE>>(this->m_bit_mask)); 
		}

	private:

		VALUE_TYPE& m_value_ref;
	};

	/// \cond ignore
	template <bit_integral VALUE_TYPE>
	bit_reference(VALUE_TYPE&, size_t) -> bit_reference<VALUE_TYPE>;
	template <bit_integral VALUE_TYPE, size_t BIT_NUM>
	bit_reference(VALUE_TYPE&, detail::bit_num_t<BIT_NUM>) -> bit_reference<VALUE_TYPE, BIT_NUM>;
	/// \endcond

	///@}
}