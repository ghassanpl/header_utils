/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "flag_bits.h"
#include <bit>
#include <iterator>

namespace ghassanpl
{
	template <integral_or_enum ENUM, detail::valid_integral VALUE_TYPE = unsigned long long>
	struct enum_flag_changes;

	/// A value struct that represents a set of bits mapped to an enum.
	/// \ingroup Flags
	/// 
	/// \par Example
	/// \code{.cpp}
	/// enum class door_flags { closed, locked, blue };
	/// enum_flags<door_flags> flags;
	/// flags.set(door_flags::closed, door_flags::blue);
	/// if (flags.is_set(door_flags::locked)) { ... }
	/// if (flags.are_all_set(door_flags::closed, door_flags::locked)) { print("Not getting in"); }
	/// flags.toggle(door_flags::locked);
	/// for (auto flag : flags)
	///   print("{} is set", flag);
	/// \endcode
	///     
	/// * Supports +/-/+=/-=/==/!= (decided not to support &/|/^/~ to not confuse the model and implementation).
	/// * Is as constexpr as I could make it
	/// * You can specify the base integral value to store bits in (`uint64_t` by default)
	/// * Uses concepts to ensure most functions do not overflow or produce garbage results (e.g. enum values of -5 or 100 won't work with uint64_t)
	/// * The first template parameter doesn't have to be an enum, any integral value will work
	/// * It is also a range
	/// 
	/// \note The members of this class will only work correctly if the enum values you give are greater than 0 and lesser than the bit-width of the VALUE_TYPE
	/// 
	/// \tparam ENUM the type containing the value flags the set will hold
	/// \tparam VALUE_TYPE the underlying integral value that stores the bits representing the flags
	template <integral_or_enum ENUM, detail::valid_integral VALUE_TYPE = unsigned long long>
	struct enum_flags
	{
		/// The underlying integral value type that holds the bits representing the flags
		using value_type = VALUE_TYPE;

		/// Type of the enum that is the source of the flags
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

		/// Constructs the value with all the given flags set
		template <typename... ARGS>
		constexpr enum_flags(enum_type base_value, ARGS... args) noexcept : bits(flag_bits<VALUE_TYPE>(base_value, args...)) {}

		/// Creates the \ref enum_flags set from the given underlying bits
		[[nodiscard]]
		constexpr static self_type from_bits(value_type val) noexcept {
			self_type ret;
			ret.bits = val;
			return ret;
		}

		/// Returns a value with all bits set (including the ones not in the enum, if any)
		[[nodiscard]]
		constexpr static self_type all() noexcept { return self_type::from_bits(~VALUE_TYPE{ 0 }); }

		/// Returns a value with all bits set, up to and including the `last`
		[[nodiscard]]
		constexpr static self_type all(enum_type last) noexcept { return self_type::from_bits(flag_bits<VALUE_TYPE>(last) | (flag_bits<VALUE_TYPE>(last) - 1)); }
		
		/// Returns a value with none of the bits set
		[[nodiscard]]
		constexpr static self_type none() noexcept { return {}; }

		/// Returns whether or not `flag` is set
		[[nodiscard]]
		constexpr bool is_set(enum_type flag) const noexcept { return (bits & flag_bits<VALUE_TYPE>(flag)) != 0; }
		
		/// Same as \ref is_set
		[[nodiscard]]
		constexpr bool contain(enum_type flag) const noexcept { return this->is_set(flag); }
		/// Same as \ref is_set
		[[nodiscard]]
		constexpr bool contains(enum_type flag) const noexcept { return this->is_set(flag); }

		/// Returns the number of flags set
		[[nodiscard]]
		constexpr int count() const noexcept { return std::popcount(bits); }

		/// Returns the value of the `n`th set bit in the set
		[[nodiscard]]
		constexpr ENUM nth_set(size_t n) const noexcept {
			auto b = bits;
			while (n--) { b ^= (VALUE_TYPE{ 1 } << std::countr_zero(b)); }
			return static_cast<ENUM>(std::countr_zero(b));
		}

		/// Returns the lowest numerical value in the set. Returns an unspecified value if no values are in the set.
		[[nodiscard]]
		constexpr ENUM first_set() const noexcept { return static_cast<ENUM>(std::countr_zero(bits)); }

		/// Returns whether or not *any* of the given flags are set
		template <typename T, typename... ARGS>
		[[nodiscard]]
		constexpr bool are_any_set(T arg, ARGS... args) const noexcept
		{
			return this->is_set(arg) || (this->is_set(args) || ...);
		}

		/// Returns whether or not *any* of the given flags are set
		[[nodiscard]]
		constexpr bool are_any_set() const noexcept { return bits != 0; }

		/// Returns whether or not *any* of the flags in the `other` set are set
		/// \note are_any_set({}) is true, which is correct or not, depending on whether you want it to be correct or not :P
		[[nodiscard]]
		constexpr bool are_any_set(self_type other) const noexcept { return other.bits == 0 /* empty set */ || (bits & other.bits) != 0; }

		/// Returns whether or not *all* of the given flags are set
		template <typename... ARGS>
		[[nodiscard]]
		constexpr bool are_all_set(ARGS... args) const noexcept
		{
			return (this->is_set(args) && ...);
		}

		/// Returns whether or not *all* of the flags in the `other` set are set
		[[nodiscard]]
		constexpr bool are_all_set(self_type other) const noexcept { return (bits & other.bits) == other.bits; }
		
		constexpr explicit operator bool() const noexcept { return bits != 0; }

		/// Returns the underlying value representing this set cast to the enum_type
		[[nodiscard]]
		constexpr enum_type to_enum_type() const noexcept { return (enum_type)bits; }

		/// Sets the given flags
		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& set(ARGS... args) noexcept { bits |= flag_bits<VALUE_TYPE>(args...); return *this; }
		/// Sets the flags in the `other`
		constexpr self_type& set(self_type other) noexcept { bits |= other.bits; return *this; }

		/// Unsets the given flags
		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& unset(ARGS... args) noexcept { bits &= ~ flag_bits<VALUE_TYPE>(args...); return *this; }
		/// Unsets the flags in the `other` set
		constexpr self_type& unset(self_type other) noexcept { bits &= ~other.bits; return *this; }

		/// Toggles the given flags
		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& toggle(ARGS... args) noexcept { bits ^= flag_bits<VALUE_TYPE>(args...); return *this; }
		/// Toggles the flags in the `other` set
		constexpr self_type& toggle(self_type other) noexcept { bits ^= other.bits; return *this; }

		/// Sets the given flags to `val`
		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& set_to(bool val, ARGS... args) noexcept
		{
			if (val) bits |= flag_bits<VALUE_TYPE>(args...); else bits &= ~flag_bits<VALUE_TYPE>(args...);
			return *this;
		}

		/// Sets the flags in the `other` set to `val`
		constexpr self_type& set_to(bool val, self_type other) noexcept
		{
			if (val) bits |= other.bits; else bits &= ~other.bits;
			return *this;
		}

		/// Returns a value with our bits from only the flags that are also set in `flags` (an intersection)
		[[nodiscard]]
		constexpr self_type but_only(self_type flags) const noexcept { return self_type::from_bits(bits & flags.bits); }

		/// Returns a value with our bits from only the flags that are also set in `flags` (an intersection)
		[[nodiscard]]
		constexpr self_type intersected_with(self_type flags) const noexcept { return self_type::from_bits(bits & flags.bits); }

		[[nodiscard]]
		constexpr self_type operator+(enum_type flag) const noexcept { return self_type::from_bits(bits | flag_bits<VALUE_TYPE>(flag)); }
		[[nodiscard]]
		constexpr self_type operator-(enum_type flag) const noexcept { return self_type::from_bits(bits & ~ flag_bits<VALUE_TYPE>(flag)); }

		[[nodiscard]]
		constexpr self_type operator+(self_type flags) const noexcept { return self_type::from_bits(bits | flags.bits); }
		[[nodiscard]]
		constexpr self_type operator-(self_type flags) const noexcept { return self_type::from_bits(bits & ~flags.bits); }
		[[nodiscard]]
		constexpr self_type except_for(self_type flags) const noexcept { return self_type::from_bits(bits & ~flags.bits); }
		[[nodiscard]]
		constexpr self_type without(self_type flags) const noexcept { return self_type::from_bits(bits & ~flags.bits); }

		template <std::convertible_to<ENUM>... ARGS>
		[[nodiscard]] constexpr self_type except_for(ARGS... args) const noexcept { return self_type::from_bits(bits & ~flag_bits<VALUE_TYPE>(args...)); }
		template <std::convertible_to<ENUM>... ARGS>
		[[nodiscard]] constexpr self_type without(ARGS... args) const noexcept { return self_type::from_bits(bits & ~flag_bits<VALUE_TYPE>(args...)); }

		constexpr self_type& operator+=(enum_type flag) noexcept { bits |= flag_bits<VALUE_TYPE>(flag); return *this; }
		constexpr self_type& operator-=(enum_type flag) noexcept { bits &= ~ flag_bits<VALUE_TYPE>(flag); return *this; }

		constexpr self_type& operator+=(self_type flags) noexcept { bits |= flags.bits; return *this; }
		constexpr self_type& operator-=(self_type flags) noexcept { bits &= ~flags.bits; return *this; }

		constexpr bool operator==(self_type other) const noexcept { return bits == other.bits; }
		constexpr bool operator!=(self_type other) const noexcept { return bits != other.bits; }

		constexpr auto operator<=>(enum_flags const& other) const noexcept { return bits <=> other.bits; }

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
#pragma warning(suppress: 4146)
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

		constexpr bool empty() const noexcept { return bits == 0; }
		constexpr void clear() noexcept { bits = 0; }

		/// Calls `callback` for each enum flag in the set
		///
		/// \param callback The function to call. It can return something convertible bool, and if it does, the iteration will stop when the function returns something true
		/// \returns void, or the value returned by the `callback` if it returned something true, or the value-initialized return type
		/// 
		/// TODO: See if this would be faster with a simple loop
		template <typename FUNC>
		constexpr auto for_each(FUNC&& callback) const
		{
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

		using flag_changes = enum_flag_changes<ENUM, VALUE_TYPE>;

		constexpr void apply(flag_changes changes) noexcept 
		{
			const auto bits_to_xor = changes.bits[0].bits & changes.bits[1].bits;
			bits ^= bits_to_xor; /// toggle only bits set in both sets

			/// remove bits that are in both sets from both sets
			changes.bits[0].bits &= ~bits_to_xor;
			changes.bits[1].bits &= ~bits_to_xor;

			/// only bits left are bits in one or zero sets
			bits &= ~changes.bits[1].bits;
			bits |= changes.bits[0].bits;
		}

		constexpr self_type operator+(flag_changes changes) const noexcept
		{
			auto result = *this;
			result.apply(changes);
			return result;
		}
	};

	enum class enum_flag_change : uint8_t
	{
		no_change,
		set,
		unset,
		toggle,
	};

	template <integral_or_enum ENUM, detail::valid_integral VALUE_TYPE>
	struct enum_flag_changes
	{
		/// The underlying integral value type that holds the bits representing the flags
		using value_type = VALUE_TYPE;

		/// Type of the enum that is the source of the flags
		using enum_type = ENUM;
		using self_type = enum_flag_changes;

		using flags_type = enum_flags<ENUM, VALUE_TYPE>;

		constexpr enum_flag_changes() noexcept = default;
		constexpr enum_flag_changes(enum_flag_changes const&) noexcept = default;
		constexpr enum_flag_changes(enum_flag_changes&&) noexcept = default;
		constexpr enum_flag_changes& operator=(enum_flag_changes const&) noexcept = default;
		constexpr enum_flag_changes& operator=(enum_flag_changes&&) noexcept = default;

		static constexpr self_type no_changes() noexcept { return self_type{}; }

		template <std::convertible_to<ENUM>... ARGS>
		static constexpr self_type to_set(ARGS... args) noexcept { return self_type{ flag_bits<VALUE_TYPE>(args...), 0 }; }
		static constexpr self_type to_set(flags_type other) noexcept { return self_type{ other, 0 }; }
		template <std::convertible_to<ENUM>... ARGS>
		static constexpr self_type to_unset(ARGS... args) noexcept { return self_type{ 0, flag_bits<VALUE_TYPE>(args...) }; }
		static constexpr self_type to_unset(flags_type other) noexcept { return self_type{ 0, other }; }
		template <std::convertible_to<ENUM>... ARGS>
		static constexpr self_type to_toggle(ARGS... args) noexcept { return self_type{ flag_bits<VALUE_TYPE>(args...), flag_bits<VALUE_TYPE>(args...) }; }
		static constexpr self_type to_toggle(flags_type other) noexcept { return self_type{  other, other }; }

		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& set_change_of(enum_flag_change change, ARGS... args) noexcept
		{
			switch (change)
			{
			case enum_flag_change::no_change: return dont_change(args...);
			case enum_flag_change::set: return set(args...);
			case enum_flag_change::unset: return unset(args...);
			case enum_flag_change::toggle: return toggle(args...);
			}
			std::unreachable();
		}

		constexpr enum_flag_change change_of(enum_type flag) const noexcept
		{
			return static_cast<enum_flag_change>(bits[0].is_set(flag) * 1 + bits[1].is_set(flag) * 2);
		}

		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& set(ARGS... args) noexcept { bits[0].set(args...); bits[1].unset(args...); return *this; }
		constexpr self_type& set(flags_type other) noexcept { bits[0].set(other); bits[1].unset(other); return *this; }

		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& unset(ARGS... args) noexcept { bits[0].unset(args...); bits[1].set(args...); return *this; }
		constexpr self_type& unset(flags_type other) noexcept { bits[0].unset(other); bits[1].set(other); return *this; }

		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& toggle(ARGS... args) noexcept { bits[0].set(args...); bits[1].set(args...); return *this; }
		constexpr self_type& toggle(flags_type other) noexcept { bits[0].set(other); bits[1].set(other); return *this; }

		template <std::convertible_to<ENUM>... ARGS>
		constexpr self_type& dont_change(ARGS... args) noexcept { bits[0].unset(args...); bits[1].unset(args...); return *this; }
		constexpr self_type& dont_change(flags_type other) noexcept { bits[0].unset(other); bits[1].unset(other); return *this; }

		constexpr self_type& dont_change_any() noexcept { bits[0].clear(); bits[1].clear(); return *this; }

		constexpr bool operator==(self_type const& other) const noexcept = default;
		constexpr auto operator<=>(self_type const& other) const noexcept = default;

		flags_type bits[2]{ {}, {} };

	private:

		constexpr enum_flag_changes(flags_type b0, flags_type b1) noexcept
			: bits(b0, b1)
		{
		}

	};

}