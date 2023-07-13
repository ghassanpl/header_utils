#pragma once

#include "bits.h"

#include <span>
#include <bit>
#include <algorithm>
#include "ranges.h"

namespace ghassanpl
{
	/// \addtogroup Bits
	/// @{

	/// A view over an integral value allowing for iteration and modification of its individual bits
	template <bit_integral INTEGER_TYPE>
	struct bit_view
	{
		// TODO: Should this have extent, like span<> ?
		// TODO: Should this store a bitcount and bitstart, so we can have views over non-integral bit counts?
		
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
				return bit_reference<INTEGER>{ this->integer_at_bit(bit_number), this->real_bit_at_bit(bit_number) };
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

			[[nodiscard]] constexpr INTEGER& integer_at_bit(size_t bit) const { return ::ghassanpl::at(integers, bit / integer_type_bit_count); }
			[[nodiscard]] static constexpr size_t real_bit_at_bit(size_t bit) { return bit % integer_type_bit_count; }

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
		using range_value = std::remove_pointer_t<typename std::iterator_traits<std::ranges::iterator_t<decltype(range)>>::pointer>;
		return bit_reference<range_value>{bit_view<range_value>{range}.integer_at_bit(bit_num), bit_num % bit_count<range_value>};
	}

	template <size_t BIT_NUM, std::ranges::contiguous_range RANGE_TYPE>
	[[nodiscard]] auto make_bit_reference(RANGE_TYPE&& range)
	{
		using range_value = std::remove_pointer_t<typename std::iterator_traits<std::ranges::iterator_t<decltype(range)>>::pointer>;
		return bit_reference<range_value, BIT_NUM% bit_count<range_value>>{bit_view<range_value>{range}.integer_at_bit(BIT_NUM)};
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

	/// @}
}

template <ghassanpl::bit_integral VALUE_TYPE>
struct std::hash<ghassanpl::bit_view<VALUE_TYPE>>
{
	[[nodiscard]] size_t operator()(ghassanpl::bit_view<VALUE_TYPE> view) const noexcept
	{
		return hash_range(view.integers());
	}
};