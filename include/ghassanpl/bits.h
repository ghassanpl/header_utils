/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <span>
#include <bit>
#include <algorithm>
#include "ranges.h"
#include "hashes.h"

#if !defined(__cpp_concepts)
#error "This library requires concepts"
#endif
#if !defined(__cpp_lib_ranges)
#error "This library requires ranges"
#endif

namespace ghassanpl
{
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
	inline constexpr B to_big_endian(B val) noexcept { if constexpr (std::endian::native == std::endian::big) return val; else return std::byteswap(val); }

	template <bit_integral B>
	inline constexpr B to_little_endian(B val) noexcept { if constexpr (std::endian::native == std::endian::little) return val; else return std::byteswap(val); }

	template <std::endian ENDIANNESS, bit_integral B>
	inline constexpr B to_endian(B val) noexcept { if constexpr (std::endian::native == ENDIANNESS) return val; else return std::byteswap(val); }

	template <bit_integral B>
	inline constexpr B to_endian(B val, std::endian endianness) noexcept { if (std::endian::native == endianness) return val; else return std::byteswap(val); }

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
			constexpr bit_num_base(size_t bit_num) noexcept : m_bit_mask(std::bit_cast<VALUE_TYPE>(unsigned_value_type(unsigned_value_type(1) << bit_num % bit_count<VALUE_TYPE>))) {}
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

		constexpr bit_reference(VALUE_TYPE& ref, size_t bit_num) requires (!is_static)
			: base_type(bit_num)
			, m_value_ref(ref)
		{
			if (bit_num >= value_bit_count)
				throw std::invalid_argument("bit_num can't be greater than or equal to the number of bits in the value type");
		}

		constexpr bit_reference(VALUE_TYPE& ref, bit_num_t<BIT_NUM> = {}) noexcept requires (is_static)
			: m_value_ref(ref)
		{
			static_assert(BIT_NUM < value_bit_count, "BIT_NUM template argument can't be greater than or equal to the number of bits in the value type");
		}

		constexpr bit_reference(bit_view<VALUE_TYPE> ref, size_t bit_num) requires (!is_static)
			: bit_reference(ref.integer_at_bit(bit_num), bit_num % value_bit_count)
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

	/// TODO: Should this have extent, like span<> ?
	/// TODO: Should this store a bitcount and bitstart, so we can have views over non-integral bit counts?
	template <bit_integral INTEGER_TYPE>
	struct bit_view
	{
		using integer_type = INTEGER_TYPE;
		using element_type = bit_reference<integer_type>;
		static constexpr size_t integer_type_bit_count = bit_count<integer_type>;

		static constexpr bool is_const = std::is_const_v<integer_type>;

		bit_view() noexcept = default;
		bit_view(std::span<integer_type> span) noexcept : m_integers(span) {}

		bit_view(bit_view const&) noexcept = default;
		bit_view(bit_view&&) noexcept = default;
		bit_view& operator=(bit_view const&) noexcept = default;
		bit_view& operator=(bit_view&&) noexcept = default;

		template <typename INTEGER>
		struct base_iterator
		{
			using value_type = bit_reference<INTEGER>;
			using difference_type = ptrdiff_t;

			[[nodiscard]] constexpr value_type operator*() const
			{
				return bit_view(integers).at(bit_number);
			}
			template <typename U>
			[[nodiscard]] constexpr difference_type operator-(base_iterator<U> other) const
			{
				return bit_number - other.bit_number;
			}
			constexpr base_iterator& operator++()
			{
				if (bit_number < bit_view(integers).size())
					++bit_number;
				return *this;
			}
			constexpr base_iterator operator++(int)
			{
				auto ret = *this;
				++(*this);
				return ret;
			}
			constexpr base_iterator& operator--()
			{
				if (bit_number > 0)
					--bit_number;
				return *this;
			}
			constexpr base_iterator operator--(int)
			{
				auto ret = *this;
				--(*this);
				return ret;
			}
			constexpr base_iterator& operator+=(difference_type n)
			{
				bit_number = std::clamp(bit_number + n, 0, bit_view(integers).size());
				return *this;
			}
			constexpr base_iterator& operator-=(difference_type n)
			{
				bit_number = std::clamp(bit_number + n, 0, bit_view(integers).size());
				return *this;
			}

			[[nodiscard]] constexpr base_iterator operator+(difference_type n) const;
			friend constexpr base_iterator operator+(difference_type, base_iterator);

			[[nodiscard]] constexpr base_iterator operator-(difference_type) const;
			[[nodiscard]] constexpr value_type operator[](difference_type) const;

			template <typename U>
			[[nodiscard]] constexpr bool operator==(base_iterator<U> const& other) const noexcept
			{
				return integers.data() == other.integers.data() && integers.size() == other.integers.size() && bit_number == other.bit_number;
			}

			template <typename U>
			[[nodiscard]] constexpr std::strong_ordering operator<=>(base_iterator<U> const& other) const noexcept
			{
				return std::tie(integers.data(), integers.size(), bit_number) <=> std::tie(other.integers.data(), other.integers.size(), other.bit_number);
			}

			base_iterator() noexcept = default;

			base_iterator(base_iterator const&) noexcept = default;
			base_iterator(base_iterator&&) noexcept = default;
			base_iterator& operator=(base_iterator const&) noexcept = default;
			base_iterator& operator=(base_iterator&&) noexcept = default;

		private:

			base_iterator(std::span<INTEGER> span, size_t num) noexcept : integers(span), bit_number(num) {}
			friend struct bit_view;

			std::span<INTEGER> integers;
			size_t bit_number{};
		};

		using iterator = base_iterator<INTEGER_TYPE>;
		using const_iterator = base_iterator<INTEGER_TYPE const>;

		static_assert(std::random_access_iterator<iterator>);

		[[nodiscard]] constexpr const_iterator begin() const { return const_iterator{ m_integers, 0 }; }
		[[nodiscard]] constexpr const_iterator end() const { return const_iterator{ m_integers, size() }; }
		[[nodiscard]] constexpr iterator begin() { return iterator{ m_integers, 0 }; }
		[[nodiscard]] constexpr iterator end() { return iterator{ m_integers, size() }; }
		[[nodiscard]] constexpr const_iterator cbegin();
		[[nodiscard]] constexpr const_iterator cend();

		[[nodiscard]] constexpr element_type at(size_t index) requires (!is_const)
		{
			return bit_reference<integer_type>{ integer_at_bit(index), real_bit_at_bit(index) };
		}

		[[nodiscard]] constexpr element_type at(size_t index) const
		{
			return bit_reference<integer_type const>{ integer_at_bit(index), real_bit_at_bit(index) };
		}

		[[nodiscard]] constexpr element_type operator[](size_t index) requires (!is_const)
		{
			return bit_reference<integer_type>{ integer_at_bit_unsafe(index), real_bit_at_bit(index) };
		}

		[[nodiscard]] constexpr element_type operator[](size_t index) const
		{
			return bit_reference<integer_type const>{ integer_at_bit_unsafe(index), real_bit_at_bit(index) };
		}

		constexpr void set(size_t index) requires (!is_const);
		constexpr void clear(size_t index) requires (!is_const);
		[[nodiscard]] constexpr bool is_set(size_t index) const;

		constexpr void set_all() requires (!is_const);
		constexpr void clear_all() requires (!is_const);
		constexpr void toggle_all() requires (!is_const);

		[[nodiscard]] constexpr bool are_all_set() const;
		
		template <typename U>
		[[nodiscard]] constexpr bool are_all_set(bit_view<U> const& bit_set) const;

		[[nodiscard]] constexpr bool are_any_set() const;

		template <typename U>
		[[nodiscard]] constexpr bool are_any_set(bit_view<U> const& bit_set) const;

		[[nodiscard]] constexpr inline size_t size() const noexcept { return size_t(m_integers.size() * integer_type_bit_count); }

		[[nodiscard]] constexpr integer_type& integer_at_bit(size_t bit) { return ::ghassanpl::at(m_integers, bit / integer_type_bit_count); }
		[[nodiscard]] constexpr integer_type const& integer_at_bit(size_t bit) const { return ::ghassanpl::at(m_integers, bit / integer_type_bit_count); }
		
		[[nodiscard]] static constexpr size_t real_bit_at_bit(size_t bit) { return bit % integer_type_bit_count; }

		[[nodiscard]] constexpr auto integers() const noexcept { return m_integers; }

		/// [[nodiscard]] constexpr bit_view<integer_type> subview(size_t starting_at_bit, size_t bit_count) const;

		template <typename U>
		constexpr void copy_to(bit_view<U>& target) const requires (!std::is_const_v<U>);
		template <typename U>
		constexpr void and_with(bit_view<U>& target) const requires (!std::is_const_v<U>);
		template <typename U>
		constexpr void or_with(bit_view<U>& target) const requires (!std::is_const_v<U>);
		template <typename U>
		constexpr void xor_with(bit_view<U>& target) const requires (!std::is_const_v<U>);

		[[nodiscard]] constexpr size_t find_first_bit(bool that_is) const noexcept;
		[[nodiscard]] constexpr size_t find_last_bit(bool that_is) const noexcept;
		[[nodiscard]] constexpr size_t count_bits(bool that_are) const noexcept;

	private:

		[[nodiscard]] constexpr integer_type& integer_at_bit_unsafe(size_t bit) { return m_integers[bit / integer_type_bit_count]; }
		[[nodiscard]] constexpr integer_type const& integer_at_bit_unsafe(size_t bit) const { return m_integers[bit / integer_type_bit_count]; }

		std::span<integer_type> m_integers;
	};

	static_assert(std::ranges::random_access_range<bit_view<int>>);

	template <std::ranges::contiguous_range RANGE_TYPE>
	bit_view(RANGE_TYPE& T) -> bit_view<std::ranges::range_value_t<std::decay_t<decltype(T)>>>;
	template <std::ranges::contiguous_range RANGE_TYPE>
	bit_view(RANGE_TYPE const& T) -> bit_view<std::ranges::range_value_t<std::decay_t<decltype(T)>> const>;

	template <std::ranges::contiguous_range RANGE_TYPE>
	[[nodiscard]] auto make_bit_reference(RANGE_TYPE&& range, size_t bit_num)
	{
		using range_value = std::remove_pointer_t<std::iterator_traits<std::ranges::iterator_t<decltype(range)>>::pointer>;
		return bit_reference<range_value>{bit_view<range_value>{range}, bit_num};
	}

	template <size_t BIT_NUM, std::ranges::contiguous_range RANGE_TYPE>
	[[nodiscard]] auto make_bit_reference(RANGE_TYPE&& range)
	{
		using range_value = std::remove_pointer_t<std::iterator_traits<std::ranges::iterator_t<decltype(range)>>::pointer>;
		return bit_reference<range_value, BIT_NUM % bit_count<range_value>>{bit_view<range_value>{range}.integer_at_bit(BIT_NUM)};
	}

	template <bit_integral VALUE_TYPE>
	[[nodiscard]] auto make_bit_reference(VALUE_TYPE& value, size_t bit_num)
	{
		return bit_reference<VALUE_TYPE>{value, bit_num};
	}

	template <size_t BIT_NUM, bit_integral VALUE_TYPE>
	[[nodiscard]] auto make_bit_reference(VALUE_TYPE& value)
	{
		static_assert(BIT_NUM < bit_count<VALUE_TYPE>, "BIT_NUM template argument can't be greater than or equal to the number of bits in the value type");
		return bit_reference<VALUE_TYPE, BIT_NUM>{value};
	}
}

namespace std
{
	template <ghassanpl::bit_integral VALUE_TYPE>
	struct hash<ghassanpl::bit_view<VALUE_TYPE>>
	{
		[[nodiscard]] size_t operator()(ghassanpl::bit_view<VALUE_TYPE> view) const noexcept
		{
			return hash_range(view.integers());
		}
	};
}