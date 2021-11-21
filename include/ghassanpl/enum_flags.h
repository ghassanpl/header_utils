/// Copyright 2017-2020 Ghassan.pl
/// Usage of the works is permitted provided that this instrument is retained with
/// the works, so that any entity that uses the works is notified of this instrument.
/// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.
#pragma once

#include "flag_bits.h"
#include <bit>
#include <compare>
#include <iterator>

namespace ghassanpl
{
	template <detail::integral_or_enum ENUM, detail::valid_integral VALUE_TYPE = unsigned long long>
	struct enum_flags
	{
		using value_type = VALUE_TYPE;
		using enum_type = ENUM;
		using self_type = enum_flags;

		value_type bits = 0;

		constexpr enum_flags() noexcept = default;
		constexpr enum_flags(const enum_flags&) noexcept = default;
		constexpr enum_flags(enum_flags&&) noexcept = default;
		constexpr enum_flags& operator=(const enum_flags&) noexcept = default;
		constexpr enum_flags& operator=(enum_flags&&) noexcept = default;

		constexpr enum_flags(enum_type base_value) noexcept : bits(flag_bits<VALUE_TYPE>(base_value)) {}
		constexpr explicit enum_flags(value_type value) noexcept : bits(value) {}

		template <typename... ARGS>
		constexpr enum_flags(enum_type base_value, ARGS... args) noexcept : bits(flag_bits<VALUE_TYPE>(base_value, args...)) {}

		[[nodiscard]]
		constexpr static self_type from_bits(value_type val) noexcept {
			self_type ret;
			ret.bits = val;
			return ret;
		}

		[[nodiscard]]
		constexpr static self_type all() noexcept { return self_type::from_bits(~VALUE_TYPE{ 0 }); }
		[[nodiscard]]
		constexpr static self_type all(enum_type last) noexcept { return self_type::from_bits(flag_bits<VALUE_TYPE>(last) | (flag_bits<VALUE_TYPE>(last) - 1)); }
		[[nodiscard]]
		constexpr static self_type none() noexcept { return {}; }

		[[nodiscard]]
		constexpr bool is_set(enum_type flag) const noexcept { return (bits & flag_bits<VALUE_TYPE>(flag)) != 0; }

		template <typename T, typename... ARGS>
		[[nodiscard]]
		constexpr bool are_any_set(T arg, ARGS... args) const noexcept
		{
			return this->is_set(arg) || (this->is_set(args) || ...);
		}

		constexpr bool are_any_set() const noexcept { return bits != 0; }

		/// are_any_set({}) is true, which is correct or not, depending on whether you want it to be correct or not :P
		[[nodiscard]]
		constexpr bool are_any_set(self_type other) const noexcept { return other.bits == 0 /* empty set */ || (bits & other.bits) != 0; }

		template <typename... ARGS>
		[[nodiscard]]
		constexpr bool are_all_set(ARGS... args) const noexcept
		{
			return (this->is_set(args) && ...);
		}

		[[nodiscard]]
		constexpr bool are_all_set(self_type other) const noexcept { return (bits & other.bits) == other.bits; }
		
		constexpr explicit operator bool() const noexcept { return bits != 0; }
		[[nodiscard]]
		constexpr enum_type to_enum_type() const noexcept { return (enum_type)bits; }

		template <typename... ARGS>
		constexpr self_type& set(ARGS... args) noexcept { bits |= flag_bits<VALUE_TYPE>(args...); return *this; }
		constexpr self_type& set(self_type other) noexcept { bits |= other.bits; return *this; }

		template <typename... ARGS>
		constexpr self_type& unset(ARGS... args) noexcept { bits &= ~ flag_bits<VALUE_TYPE>(args...); return *this; }
		constexpr self_type& unset(self_type other) noexcept { bits &= ~other.bits; return *this; }

		template <typename... ARGS>
		constexpr self_type& toggle(ARGS... args) noexcept { bits ^= flag_bits<VALUE_TYPE>(args...); return *this; }
		constexpr self_type& toggle(self_type other) noexcept { bits ^= other.bits; return *this; }

		template <typename... ARGS>
		constexpr self_type& set_to(bool val, ARGS... args) noexcept
		{
			if (val) bits |= flag_bits<VALUE_TYPE>(args...); else bits &= ~flag_bits<VALUE_TYPE>(args...);
			return *this;
		}

		constexpr self_type& set_to(bool val, self_type other) noexcept
		{
			if (val) bits |= other.bits; else bits &= ~other.bits;
			return *this;
		}

		[[nodiscard]]
		constexpr self_type operator+(enum_type flag) const noexcept { return self_type::from_bits(bits | flag_bits<VALUE_TYPE>(flag)); }
		[[nodiscard]]
		constexpr self_type operator-(enum_type flag) const noexcept { return self_type::from_bits(bits & ~ flag_bits<VALUE_TYPE>(flag)); }

		[[nodiscard]]
		constexpr self_type operator+(self_type flags) const noexcept { return self_type::from_bits(bits | flags.bits); }
		[[nodiscard]]
		constexpr self_type operator-(self_type flags) const noexcept { return self_type::from_bits(bits & ~flags.bits); }

		constexpr self_type& operator+=(enum_type flag) noexcept { bits |= flag_bits<VALUE_TYPE>(flag); return *this; }
		constexpr self_type& operator-=(enum_type flag) noexcept { bits &= ~ flag_bits<VALUE_TYPE>(flag); return *this; }

		constexpr self_type& operator+=(self_type flags) noexcept { bits |= flags.bits; return *this; }
		constexpr self_type& operator-=(self_type flags) noexcept { bits &= ~flags.bits; return *this; }

		constexpr bool operator==(self_type other) const noexcept { return bits == other.bits; }
		constexpr bool operator!=(self_type other) const noexcept { return bits != other.bits; }

		/// TODO: begin() and end(), so we can iterate over the set bits
		/// TODO: See if this would be faster with a simple loop
		
		struct iterator
		{
			self_type::value_type bits;
			using bitset_type = std::make_unsigned_t<self_type::value_type>;

			using difference_type = int;
			using value_type = enum_type;
			using pointer_type = void;
			using reference = void;
			using iterator_category = std::forward_iterator_tag;


			enum_type operator*() const noexcept
			{ 
				const auto bitset = static_cast<bitset_type>(bits);
				return static_cast<enum_type>(std::countr_zero(static_cast<bitset_type>(bitset & -bitset)));
			}

			iterator& operator++() noexcept
			{
				const auto bitset = static_cast<bitset_type>(bits);
#pragma warning(suppress: 4146)
				const auto t = static_cast<bitset_type>(bitset & -bitset);
				bits = static_cast<self_type::value_type>(bitset ^ t);
				return *this;
			}

			iterator operator++(int) noexcept { const auto result = *this; operator++(); return result; }

			auto operator<=>(iterator const& other) const noexcept = default;
		};

		iterator begin() const noexcept { return iterator{ bits }; }
		iterator end() const noexcept { return iterator{ {} }; }
		iterator cbegin() const noexcept { return iterator{ bits }; }
		iterator cend() const noexcept { return iterator{ {} }; }

		template <typename FUNC>
		constexpr auto for_each(FUNC&& callback) const
		{
			//using return_type = decltype(callback(enum_type{}));
			using return_type = std::invoke_result_t<FUNC, enum_type>;
			using bitset_type = std::make_unsigned_t<value_type>;
			auto bitset = static_cast<bitset_type>(bits);
			while (bitset)
			{
#pragma warning(suppress: 4146)
				const auto t = static_cast<bitset_type>(bitset & -bitset);
				const auto r = std::countr_zero(t);
				if constexpr (std::is_convertible_v<return_type, bool>)
				{
					if (auto ret = callback((enum_type)r)) return ret;
				}
				else
					callback((enum_type)r);
				bitset ^= t;
			}
			if constexpr (std::is_convertible_v<return_type, bool>)
				return return_type{};
		}
	};

}