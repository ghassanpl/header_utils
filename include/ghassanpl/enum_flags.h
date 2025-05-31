/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "flag_bits.h"
#include <bit>
#include <iterator>

/// TODO: std::bitset is constexpr since C++23, so we can internally use that as soon as support for C++23 is widespread enough
///		For example, we could have a version of enum_flags that takes an ENUM MIN_VALUE and an ENUM MAX_VALUE template parameters,
///		and uses those to create a std::bitset<MAX_VALUE - MIN_VALUE + 1> internally, and then use that to store the bits.

namespace ghassanpl
{
	template <integral_or_enum ENUM, detail::valid_integral VALUE_TYPE = unsigned long long>
	struct enum_flag_changes;

	constexpr inline struct all_flags_t {} all_flags;
	constexpr inline struct no_flags_t {} no_flags;
	
	/// A (constexpr) value struct that represents a set of bits mapped to an enum (implemented as a bitset)
	/// \ingroup Flags
	/// 
	/// \par Example
	/// \code{.cpp}
	/// enum class door_flags { closed, locked, blue };
	/// enum_flags<door_flags> flags;
	/// flags.set(door_flags::closed, door_flags::blue);
	/// if (flags.is_set(door_flags::locked)) { ... }
	/// if (flags.contains_all_of(door_flags::closed, door_flags::locked)) { print("Not getting in"); }
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
	/// \attention The members of this class will only work correctly if the enum values you give are greater than 0 and lesser than the bit-width of the VALUE_TYPE
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
		template<typename U = VALUE_TYPE>
		requires (!std::is_same_v<U, enum_type>)
		constexpr explicit enum_flags(value_type value) noexcept
			: bits(value) 
		{
		}

		/// Constructs the value with all the given flags set
		template <typename... ARGS>
		constexpr enum_flags(enum_type base_value, ARGS... args) noexcept : bits(flag_bits<VALUE_TYPE>(base_value, args...)) {}

		constexpr enum_flags(std::initializer_list<enum_type> values) noexcept {
			for (auto val : values) { bits |= flag_bits<VALUE_TYPE>(val); }
		}

		/// Creates the \ref enum_flags set from the given underlying bits
		template <integral_or_enum BITS_TYPE>
		requires std::convertible_to<decltype(detail::to_underlying_type(BITS_TYPE{})), VALUE_TYPE>
		[[nodiscard]] constexpr static self_type from_bits(BITS_TYPE val) noexcept {
			self_type ret;
			ret.bits = static_cast<VALUE_TYPE>(detail::to_underlying_type(val));
			return ret;
		}

		/// Returns a value with all bits set (including the ones not in the enum, if any)
		[[nodiscard]]
		constexpr static self_type all() noexcept { return self_type::from_bits(~VALUE_TYPE{ 0 }); }

		constexpr enum_flags(all_flags_t) noexcept : bits(~VALUE_TYPE{0}) {}

		/// Returns a value with all bits set, up to and including the `last`
		[[nodiscard]]
		constexpr static self_type all(enum_type last) noexcept { return self_type::from_bits(flag_bits<VALUE_TYPE>(last) & ~(flag_bits<VALUE_TYPE>(last + 1) - 1)); }

		/// Returns a value with bits starting with `first`, up to and including `last`, set
		[[nodiscard]]
		constexpr static self_type all_between(enum_type first, enum_type last) noexcept { return all(last) - self_type::from_bits(~(flag_bits<VALUE_TYPE>(first) - 1)); }

		/// Returns a value with none of the bits set
		[[nodiscard]]
		constexpr static self_type none() noexcept { return {}; }
		
		constexpr enum_flags(no_flags_t) noexcept : bits(VALUE_TYPE{0}) {}

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
		/// Preconditions: `n` must be less than the number of bits set (`n < count()`)
		[[nodiscard]]
		constexpr enum_type nth_set(size_t n) const noexcept {
			auto b = bits;
			while (n--) { b ^= (VALUE_TYPE{ 1 } << std::countr_zero(b)); }
			return static_cast<enum_type>(std::countr_zero(b));
		}

		/// Returns the lowest numerical value in the set. Returns an unspecified value if no values are in the set.
		[[nodiscard]]
		constexpr enum_type first_set() const noexcept { return static_cast<enum_type>(std::countr_zero(bits)); }

		/// Returns the highest numerical value in the set, or enum_type(-1) if no values are in the set.
		[[nodiscard]]
		constexpr enum_type last_set() const noexcept { return static_cast<enum_type>((sizeof(value_type) * CHAR_BIT - std::countl_zero(bits)) - 1); }

		/// Returns whether or not *any* of the given flags are set
		template <typename... ARGS>
		[[nodiscard]]
		constexpr bool contains_any_of(enum_type arg, ARGS... args) const noexcept
		{
			return this->is_set(arg) || (this->is_set(args) || ...);
		}

		/// Returns whether or not *any* of the given flags are set
		template <typename... ARGS>
		[[nodiscard]]
		constexpr bool contains_none_of(enum_type arg, ARGS... args) const noexcept
		{
			return !(this->is_set(arg) || (this->is_set(args) || ...));
		}

		/// Returns whether or not *any* of the given flags are set
		[[nodiscard]]
		constexpr bool are_any_set() const noexcept { return bits != 0; }

		/// Returns whether *this == all()
		[[nodiscard]]
		constexpr bool full() const noexcept { return (~bits) == 0; }

		/// Returns whether or not *any* of the flags in the `other` set are set
		/// \note contains_any_of({}) is true, which is correct or not, depending on whether you want it to be correct or not :P
		[[nodiscard]]
		constexpr bool contains_any_of(self_type other) const noexcept { return other.bits == 0 /* empty set */ || (bits & other.bits) != 0; }

		/// Returns whether or not *all* of the given flags are set
		template <typename... ARGS>
		[[nodiscard]]
		constexpr bool contains_all_of(ARGS... args) const noexcept
		{
			return (this->is_set(args) && ...);
		}

		/// Returns whether or not *all* of the flags in the `other` set are set
		[[nodiscard]]
		constexpr bool contains_all_of(self_type other) const noexcept { return (bits & other.bits) == other.bits; }
		
		constexpr explicit operator bool() const noexcept { return bits != 0; }

		/// Returns the underlying value representing this set cast to the enum_type
		[[nodiscard]]
		constexpr enum_type to_enum_type() const noexcept { return (enum_type)bits; }

		/// Sets the given flags
		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& set(ARGS... args) noexcept { bits |= flag_bits<VALUE_TYPE>(args...); return *this; }
		/// Sets the flags in the `other`
		constexpr self_type& set(self_type other) noexcept { bits |= other.bits; return *this; }

		/// TODO: `insert()` as an alias for `set()`

		/// Unsets the given flags
		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& unset(ARGS... args) noexcept { bits &= ~ flag_bits<VALUE_TYPE>(args...); return *this; }
		/// Unsets the flags in the `other` set
		constexpr self_type& unset(self_type other) noexcept { bits &= ~other.bits; return *this; }

		/// TODO: `erase()` as an alias for `unset()`
		
		/// Toggles the given flags
		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& toggle(ARGS... args) noexcept { bits ^= flag_bits<VALUE_TYPE>(args...); return *this; }
		/// Toggles the flags in the `other` set
		constexpr self_type& toggle(self_type other) noexcept { bits ^= other.bits; return *this; }

		/// Sets the given flags to `val`
		template <std::convertible_to<enum_type>... ARGS>
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
		constexpr self_type operator-() const noexcept { return self_type::from_bits(~bits); }
		[[nodiscard]]
		constexpr self_type except_for(self_type flags) const noexcept { return self_type::from_bits(bits & ~flags.bits); }
		[[nodiscard]]
		constexpr self_type without(self_type flags) const noexcept { return self_type::from_bits(bits & ~flags.bits); }

		template <std::convertible_to<enum_type>... ARGS>
		[[nodiscard]] constexpr self_type except_for(ARGS... args) const noexcept { return self_type::from_bits(bits & ~flag_bits<VALUE_TYPE>(args...)); }
		template <std::convertible_to<enum_type>... ARGS>
		[[nodiscard]] constexpr self_type without(ARGS... args) const noexcept { return self_type::from_bits(bits & ~flag_bits<VALUE_TYPE>(args...)); }

		constexpr self_type& operator+=(enum_type flag) noexcept { bits |= flag_bits<VALUE_TYPE>(flag); return *this; }
		constexpr self_type& operator-=(enum_type flag) noexcept { bits &= ~ flag_bits<VALUE_TYPE>(flag); return *this; }

		constexpr self_type& operator+=(self_type flags) noexcept { bits |= flags.bits; return *this; }
		constexpr self_type& operator-=(self_type flags) noexcept { bits &= ~flags.bits; return *this; }

		constexpr auto operator<=>(enum_flags const& other) const noexcept = default;

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

		using flag_changes = enum_flag_changes<enum_type, value_type>;

		constexpr void apply(flag_changes changes) noexcept 
		{
			const auto bits_to_xor = changes.bits_to_set.bits & changes.bits_to_unset.bits;
			bits ^= bits_to_xor; /// toggle only bits set in both sets

			/// remove bits that are in both sets from both sets
			changes.bits_to_set.bits &= ~bits_to_xor;
			changes.bits_to_unset.bits &= ~bits_to_xor;

			/// only bits left are bits in one or zero sets
			bits &= ~changes.bits_to_unset.bits;
			bits |= changes.bits_to_set.bits;
		}

		constexpr self_type operator+(flag_changes changes) const noexcept
		{
			auto result = *this;
			result.apply(changes);
			return result;
		}
	};

	template <typename TYPE>
	struct is_enum_flags : std::false_type {};

	template <typename ENUM, typename VALUE_TYPE>
	struct is_enum_flags<enum_flags<ENUM, VALUE_TYPE>> : std::true_type {};

	template <typename TYPE>
	constexpr bool is_enum_flags_v = is_enum_flags<TYPE>::value;
	
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

		template <std::convertible_to<enum_type>... ARGS>
		static constexpr self_type to_set(ARGS... args) noexcept { return self_type{ flags_type{args...}, flags_type{} }; }
		static constexpr self_type to_set(flags_type other) noexcept { return self_type{ other, flags_type{} }; }
		static constexpr self_type to_set_all() noexcept { return self_type{ flags_type::all(), flags_type{}}; }
		template <std::convertible_to<enum_type>... ARGS>
		static constexpr self_type to_unset(ARGS... args) noexcept { return self_type{ flags_type{}, flags_type{args...} }; }
		static constexpr self_type to_unset(flags_type other) noexcept { return self_type{ flags_type{}, other }; }
		static constexpr self_type to_unset_all() noexcept { return self_type{ flags_type{}, flags_type::all() }; }
		template <std::convertible_to<enum_type>... ARGS>
		static constexpr self_type to_toggle(ARGS... args) noexcept { return self_type{ flags_type{args...}, flags_type{args...} }; }
		static constexpr self_type to_toggle(flags_type other) noexcept { return self_type{ other, other }; }
		static constexpr self_type to_toggle_all() noexcept { return self_type{ flags_type::all(), flags_type::all() }; }

		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& set_change_of(enum_flag_change change, ARGS... args) noexcept
		{
			switch (change)
			{
			case enum_flag_change::no_change: return dont_change(args...);
			case enum_flag_change::set: return set(args...);
			case enum_flag_change::unset: return unset(args...);
			case enum_flag_change::toggle: return toggle(args...);
			}
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
			std::unreachable();
#endif
		}

		constexpr enum_flag_change change_of(enum_type flag) const noexcept
		{
			return static_cast<enum_flag_change>(bits_to_set.is_set(flag) * 1 + bits_to_unset.is_set(flag) * 2);
		}

		constexpr flags_type flags_to_set() const noexcept { return bits_to_set - bits_to_unset; }
		constexpr flags_type flags_to_unset() const noexcept { return bits_to_unset - bits_to_set; }
		constexpr flags_type flags_to_toggle() const noexcept { return bits_to_unset.but_only(bits_to_set); }
		constexpr flags_type flags_to_not_change() const noexcept { return -(bits_to_set + bits_to_unset); }

		constexpr flags_type flags_to(enum_flag_change change) const noexcept
		{
			switch (change)
			{
			case enum_flag_change::set: return flags_to_set();
			case enum_flag_change::unset: return flags_to_unset();
			case enum_flag_change::toggle: return flags_to_toggle();
			case enum_flag_change::no_change: return flags_to_not_change();
			}
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
			std::unreachable();
#endif
		}


		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& set(ARGS... args) noexcept { bits_to_set.set(args...); bits_to_unset.unset(args...); return *this; }
		constexpr self_type& set(flags_type other) noexcept { bits_to_set.set(other); bits_to_unset.unset(other); return *this; }

		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& unset(ARGS... args) noexcept { bits_to_set.unset(args...); bits_to_unset.set(args...); return *this; }
		constexpr self_type& unset(flags_type other) noexcept { bits_to_set.unset(other); bits_to_unset.set(other); return *this; }

		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& toggle(ARGS... args) noexcept { bits_to_set.set(args...); bits_to_unset.set(args...); return *this; }
		constexpr self_type& toggle(flags_type other) noexcept { bits_to_set.set(other); bits_to_unset.set(other); return *this; }

		template <std::convertible_to<enum_type>... ARGS>
		constexpr self_type& dont_change(ARGS... args) noexcept { bits_to_set.unset(args...); bits_to_unset.unset(args...); return *this; }
		constexpr self_type& dont_change(flags_type other) noexcept { bits_to_set.unset(other); bits_to_unset.unset(other); return *this; }

		constexpr self_type& dont_change_any() noexcept { bits_to_set.clear(); bits_to_unset.clear(); return *this; }

		constexpr auto operator<=>(self_type const& other) const noexcept = default;

		flags_type bits_to_set{};
		flags_type bits_to_unset{};

	private:

		constexpr enum_flag_changes(flags_type to_set, flags_type to_unset) noexcept
			: bits_to_set(to_set)
			, bits_to_unset(to_unset)
		{
		}

	};

}