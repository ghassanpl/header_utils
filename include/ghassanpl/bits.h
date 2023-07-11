/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>
#include <cstdint>
#include <bit>
#include <stdexcept>

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif
#if !defined(__cpp_lib_ranges)
#error "This library requires ranges"
#endif

namespace ghassanpl
{
	
#if __cpp_lib_byteswap < 202110L
	template <class>
	inline constexpr bool always_false = false;

	[[nodiscard]] constexpr unsigned short byteswap_ushort(const unsigned short val) noexcept {
		return static_cast<unsigned short>((val << 8) | (val >> 8));
	}

	[[nodiscard]] constexpr unsigned long byteswap_ulong(const unsigned long val) noexcept {
		return (val << 24) | ((val << 8) & 0x00FF'0000) | ((val >> 8) & 0x0000'FF00) | (val >> 24);
	}

	[[nodiscard]] constexpr unsigned long long byteswap_uint64(const unsigned long long val) noexcept {
		return (val << 56) | ((val << 40) & 0x00FF'0000'0000'0000) | ((val << 24) & 0x0000'FF00'0000'0000)
			| ((val << 8) & 0x0000'00FF'0000'0000) | ((val >> 8) & 0x0000'0000'FF00'0000)
			| ((val >> 24) & 0x0000'0000'00FF'0000) | ((val >> 40) & 0x0000'0000'0000'FF00) | (val >> 56);
	}

	template <class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
	[[nodiscard]] constexpr T byteswap(const T val) noexcept
	{
		if constexpr (sizeof(T) == 1)
			return val;
		else if constexpr (sizeof(T) == 2)
			return static_cast<T>(byteswap_ushort(static_cast<unsigned short>(val)));
		else if constexpr (sizeof(T) == 4)
			return static_cast<T>(byteswap_ulong(static_cast<unsigned long>(val)));
		else if constexpr (sizeof(T) == 8)
			return static_cast<T>(byteswap_uint64(static_cast<unsigned long long>(val)));
		else
			static_assert(always_false<T>, "unsupported integer size");
	}
#else
	using std::byteswap;
#endif

	/// \addtogroup Bits
	/// Types and functions for manipulating bits in integral values
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
	static constexpr inline auto bit_count = sizeof(T) * CHAR_BIT;

	/// A value of type `uint64_t` with all bits set
	/// \ingroup Bits
	constexpr inline uint64_t all_bits = ~uint64_t{ 0 };

	///@}

	template <size_t BEGIN, size_t END>
	constexpr inline uint64_t bit_mask_v = (all_bits >> (64 - END)) << BEGIN;

	template <bit_integral FOR>
	constexpr inline uint64_t bit_mask_for_v = (all_bits >> (64 - bit_count<FOR>));

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

	template <size_t N>
	using uintN_t = typename uintN_t_t<N>::value;

	template <size_t N>
	struct intN_t_t
	{
		using value = std::make_signed_t<uintN_t<N>>;
	};

	template <size_t N>
	using intN_t = typename intN_t_t<N>::value;

	template <bool SIGNED, size_t N>
	struct sintN_t_t;
	template <size_t N> struct sintN_t_t<false, N> { using value = uintN_t<N>; };
	template <size_t N> struct sintN_t_t<true, N> { using value = intN_t<N>; };

	template <bool SIGNED, size_t N>
	using sintN_t = typename sintN_t_t<SIGNED, N>::value;


	/*

	inline constexpr uint64_t bit_mask(size_t begin, size_t end)
	{
		return (all_bits >> (64 - end)) << begin;
	}

	template <bit_integral T, typename NUMERIC_TYPE = uint64_t>
	inline NUMERIC_TYPE get_bits(T value, size_t begin, size_t end)
	{
		return (GetBits<T, NUMERIC_TYPE>(value) & bit_mask(begin, end)) >> begin;
	}

	template <typename T, typename NUMERIC_TYPE = uint64_t>
	requires std::is_trivially_copyable_v<T> && (sizeof(T) <= sizeof(uint64_t))
	inline NUMERIC_TYPE get_bits(T value, size_t begin, size_t end)
	{
		return (GetBits<T, NUMERIC_TYPE>(value) & bit_mask(begin, end)) >> begin;
	}

	template <size_t BEGIN, size_t END, typename T, typename NUMERIC_TYPE = uint64_t>
	requires std::is_trivially_copyable_v<T> && (sizeof(T) <= sizeof(uint64_t))
	inline NUMERIC_TYPE get_bits(T value)
	{
		return (GetBits<T, NUMERIC_TYPE>(value) & bit_mask_v<BEGIN, END>) >> BEGIN;
	}
	*/

	template <bit_integral T>
	inline constexpr auto most_significant_half(T v) noexcept
	{
		constexpr auto bit_count = ghassanpl::bit_count<T>;
		constexpr auto bit_count_half = bit_count / 2;
		return sintN_t<std::is_signed_v<T>, bit_count_half>{ (v >> bit_count_half) & bit_mask_v<0, bit_count_half> };
	}

	template <bit_integral T>
	inline constexpr auto least_significant_half(T v) noexcept
	{
		constexpr auto bit_count = ghassanpl::bit_count<T>;
		constexpr auto bit_count_half = bit_count / 2;
		return sintN_t<std::is_signed_v<T>, bit_count_half>{ v & bit_mask_v<0, bit_count_half> };
	}

	/// endianness

	template <bit_integral B>
	inline constexpr B to_big_endian(B val) noexcept { if constexpr (std::endian::native == std::endian::big) return val; else return byteswap(val); }

	template <bit_integral B>
	inline constexpr B to_little_endian(B val) noexcept { if constexpr (std::endian::native == std::endian::little) return val; else return byteswap(val); }

	template <std::endian ENDIANNESS, bit_integral B>
	inline constexpr B to_endian(B val) noexcept { if constexpr (std::endian::native == ENDIANNESS) return val; else return byteswap(val); }

	template <bit_integral B>
	inline constexpr B to_endian(B val, std::endian endianness) noexcept { if (std::endian::native == endianness) return val; else return byteswap(val); }

	/// bit references and bit views

	template <size_t NUM>
	using bit_num_t = std::integral_constant<size_t, NUM>;

	template <size_t NUM>
	constexpr inline auto bit_num = bit_num_t<NUM>{};

	constexpr inline size_t dynamic_bit_number = size_t(-1);

	namespace detail
	{
		template <bit_integral VALUE_TYPE, size_t BIT_NUM>
		struct bit_num_base
		{
			using unsigned_value_type = std::make_unsigned_t<VALUE_TYPE>;
			static constexpr VALUE_TYPE m_bit_mask = std::bit_cast<VALUE_TYPE>(unsigned_value_type(unsigned_value_type(1) << BIT_NUM % bit_count<VALUE_TYPE>));
		};
		template <bit_integral VALUE_TYPE>
		struct bit_num_base<VALUE_TYPE, dynamic_bit_number>
		{
			using unsigned_value_type = std::make_unsigned_t<VALUE_TYPE>;
			constexpr bit_num_base(size_t bitnum) noexcept : m_bit_mask(std::bit_cast<VALUE_TYPE>(unsigned_value_type(unsigned_value_type(1) << bitnum % bit_count<VALUE_TYPE>))) {}
			VALUE_TYPE m_bit_mask;
		};
	}

	template <bit_integral ELEMENT_TYPE>
	struct bit_view;

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

		constexpr bit_reference(VALUE_TYPE& ref, bit_num_t<BIT_NUM> = {}) noexcept requires (is_static)
			: m_value_ref(ref)
		{
			static_assert(BIT_NUM < value_bit_count, "BIT_NUM template argument can't be greater than or equal to the number of bits in the value type");
		}

		constexpr bit_reference(bit_view<VALUE_TYPE> ref, size_t bitnum) requires (!is_static)
			: bit_reference(ref.integer_at_bit(bitnum), bitnum % value_bit_count)
		{
		}

		constexpr bit_reference& operator=(bool val) noexcept requires (!is_const)
		{
			if (val)
				m_value_ref |= this->m_bit_mask;
			else
				m_value_ref &= ~this->m_bit_mask;
			return *this;
		}

		/// TODO: Is it worth pulling in the <atomic> header just for this?
		/*
		constexpr bit_reference& atomic_set(bool val) noexcept
		{
			std::atomic_ref<VALUE_TYPE> ref(m_value_ref);
			if (val)
				ref |= this->m_bit_mask;
			else
				ref &= ~this->m_bit_mask;
			return *this;
		}
		*/

		[[nodiscard]] explicit constexpr operator bool() const noexcept
		{
			return (m_value_ref & this->m_bit_mask) != 0;
		}

		[[nodiscard]] constexpr bool operator==(bit_reference const& other) const noexcept
		{
			return (m_value_ref & this->m_bit_mask) == (other.m_value_ref & other.m_bit_mask);
		}

		[[nodiscard]] constexpr auto operator<=>(bit_reference const& other) const noexcept
		{
			return this->operator bool() <=> other.operator bool();
		}

		[[nodiscard]] constexpr size_t bit_number() const noexcept { return std::countr_zero(std::bit_cast<std::make_unsigned_t<VALUE_TYPE>>(this->m_bit_mask)); }
		[[nodiscard]] constexpr auto& integer_value() const noexcept { return m_value_ref; }

	private:

		VALUE_TYPE& m_value_ref;
	};

	template <bit_integral VALUE_TYPE>
	bit_reference(VALUE_TYPE&, size_t) -> bit_reference<VALUE_TYPE>;
	template <bit_integral VALUE_TYPE, size_t BIT_NUM>
	bit_reference(VALUE_TYPE&, bit_num_t<BIT_NUM>) -> bit_reference<VALUE_TYPE, BIT_NUM>;
}