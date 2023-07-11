#pragma once

#if __cplusplus >= 201103L || defined(__cpp_constexpr)
#define EF_CONSTEXPR constexpr
#define EF_EXPLICITOP explicit
#define EF_NOEXCEPT noexcept
#if __has_cpp_attribute(nodiscard)
#define EF_NODISCARD [[nodiscard]]
#else
#define EF_NODISCARD
#endif
#else
#define EF_CONSTEXPR
#define EF_EXPLICITOP
#define EF_NOEXCEPT
#define EF_NODISCARD
#endif

namespace ghassanpl
{
	template <typename RESULT_TYPE, typename ENUM_TYPE>
	EF_CONSTEXPR RESULT_TYPE flag_bit(ENUM_TYPE e) EF_NOEXCEPT
	{
		return ((static_cast<RESULT_TYPE>(1) << static_cast<RESULT_TYPE>(e)));
	}

	template <typename ENUM_TYPE, typename VALUE_TYPE = unsigned long long>
	struct enum_flags
	{
		/// The underlying integral value type that holds the bits representing the flags
		typedef VALUE_TYPE value_type;

		/// Type of the enum that is the source of the flags
		typedef ENUM_TYPE enum_type;
		typedef enum_flags<ENUM_TYPE, VALUE_TYPE> self_type;

		value_type bits;

		EF_CONSTEXPR enum_flags() EF_NOEXCEPT : bits(0) {}
		EF_CONSTEXPR enum_flags(const enum_flags& other) EF_NOEXCEPT : bits(other.bits) {};
		EF_CONSTEXPR enum_flags& operator=(const enum_flags& other) EF_NOEXCEPT { bits = other.bits; return *this; }

		EF_CONSTEXPR enum_flags(enum_type base_value) EF_NOEXCEPT : bits(flag_bit<VALUE_TYPE>(base_value)) {}
		EF_CONSTEXPR explicit enum_flags(value_type value) EF_NOEXCEPT : bits(value) {}

		/// Creates the \ref enum_flags set from the given underlying bits
		EF_NODISCARD EF_CONSTEXPR static self_type from_bits(value_type val) EF_NOEXCEPT {
			self_type ret;
			ret.bits = val;
			return ret;
		}

		/// Returns a value with all bits set (including the ones not in the enum, if any)
		EF_NODISCARD EF_CONSTEXPR static self_type all() EF_NOEXCEPT { return self_type::from_bits(~static_cast<VALUE_TYPE>(0)); }

		/// Returns a value with all bits set, up to and including the `last`
		EF_NODISCARD EF_CONSTEXPR static self_type all(enum_type last) EF_NOEXCEPT { return self_type::from_bits(flag_bit<VALUE_TYPE>(last) | (flag_bit<VALUE_TYPE>(last) - 1)); }

		/// Returns a value with none of the bits set
		EF_NODISCARD EF_CONSTEXPR static self_type none() EF_NOEXCEPT { return self_type(); }

		/// Returns whether or not `flag` is set
		EF_NODISCARD EF_CONSTEXPR bool is_set(enum_type flag) const EF_NOEXCEPT { return (bits & flag_bit<VALUE_TYPE>(flag)) != 0; }

		/// Same as \ref is_set
		EF_NODISCARD EF_CONSTEXPR bool contain(enum_type flag) const EF_NOEXCEPT { return this->is_set(flag); }
		/// Same as \ref is_set
		EF_NODISCARD EF_CONSTEXPR bool contains(enum_type flag) const EF_NOEXCEPT { return this->is_set(flag); }

		/// Returns whether or not *any* of the given flags are set
		EF_NODISCARD EF_CONSTEXPR bool are_any_set() const EF_NOEXCEPT { return bits != 0; }

		/// Returns whether or not *any* of the flags in the `other` set are set
		/// \note are_any_set({}) is true, which is correct or not, depending on whether you want it to be correct or not :P
		EF_NODISCARD EF_CONSTEXPR bool are_any_set(self_type other) const EF_NOEXCEPT { return other.bits == 0 /* empty set */ || (bits & other.bits) != 0; }

		/// Returns whether or not *all* of the flags in the `other` set are set
		EF_NODISCARD EF_CONSTEXPR bool are_all_set(self_type other) const EF_NOEXCEPT { return (bits & other.bits) == other.bits; }

		EF_CONSTEXPR EF_EXPLICITOP operator bool() const EF_NOEXCEPT { return bits != 0; }

		/// Returns the underlying value representing this set cast to the enum_type
		EF_NODISCARD EF_CONSTEXPR enum_type to_enum_type() const EF_NOEXCEPT { return static_cast<enum_type>(bits); }

		/// Sets the given flag
		EF_CONSTEXPR self_type& set(enum_type e) EF_NOEXCEPT { bits |= flag_bit<VALUE_TYPE>(e); return *this; }
		/// Sets the flags in the `other`
		EF_CONSTEXPR self_type& set(self_type other) EF_NOEXCEPT { bits |= other.bits; return *this; }

		/// Unsets the given flag
		EF_CONSTEXPR self_type& unset(enum_type e) EF_NOEXCEPT { bits &= ~flag_bit<VALUE_TYPE>(e); return *this; }
		/// Unsets the flags in the `other` set
		EF_CONSTEXPR self_type& unset(self_type other) EF_NOEXCEPT { bits &= ~other.bits; return *this; }

		/// Toggles the given flag
		EF_CONSTEXPR self_type& toggle(enum_type e) EF_NOEXCEPT { bits ^= flag_bit<VALUE_TYPE>(e); return *this; }
		/// Toggles the flags in the `other` set
		EF_CONSTEXPR self_type& toggle(self_type other) EF_NOEXCEPT { bits ^= other.bits; return *this; }

		/// Sets the given flags to `val`
		EF_CONSTEXPR self_type& set_to(enum_type e, bool val) EF_NOEXCEPT
		{
			if (val) bits |= flag_bit<VALUE_TYPE>(e); else bits &= ~flag_bit<VALUE_TYPE>(e);
			return *this;
		}

		/// Sets the flags in the `other` set to `val`
		EF_CONSTEXPR self_type& set_to(self_type other, bool val) EF_NOEXCEPT
		{
			if (val) bits |= other.bits; else bits &= ~other.bits;
			return *this;
		}

		/// Returns a value with our bits from only the flags that are also set in `flags` (an intersection)
		EF_NODISCARD EF_CONSTEXPR self_type but_only(self_type flags) const EF_NOEXCEPT { return self_type::from_bits(bits & flags.bits); }

		/// Returns a value with our bits from only the flags that are also set in `flags` (an intersection)
		EF_NODISCARD EF_CONSTEXPR self_type intersected_with(self_type flags) const EF_NOEXCEPT { return self_type::from_bits(bits & flags.bits); }

		EF_NODISCARD EF_CONSTEXPR self_type operator+(enum_type flag) const EF_NOEXCEPT { return self_type::from_bits(bits | flag_bit<VALUE_TYPE>(flag)); }
		EF_NODISCARD EF_CONSTEXPR self_type operator-(enum_type flag) const EF_NOEXCEPT { return self_type::from_bits(bits & ~flag_bit<VALUE_TYPE>(flag)); }

		EF_NODISCARD EF_CONSTEXPR self_type operator+(self_type flags) const EF_NOEXCEPT { return self_type::from_bits(bits | flags.bits); }
		EF_NODISCARD EF_CONSTEXPR self_type operator-(self_type flags) const EF_NOEXCEPT { return self_type::from_bits(bits & ~flags.bits); }

		EF_CONSTEXPR self_type& operator+=(enum_type flag) EF_NOEXCEPT { bits |= flag_bit<VALUE_TYPE>(flag); return *this; }
		EF_CONSTEXPR self_type& operator-=(enum_type flag) EF_NOEXCEPT { bits &= ~flag_bit<VALUE_TYPE>(flag); return *this; }

		EF_CONSTEXPR self_type& operator+=(self_type flags) EF_NOEXCEPT { bits |= flags.bits; return *this; }
		EF_CONSTEXPR self_type& operator-=(self_type flags) EF_NOEXCEPT { bits &= ~flags.bits; return *this; }

		EF_CONSTEXPR bool operator==(self_type other) const EF_NOEXCEPT { return bits == other.bits; }
		EF_CONSTEXPR bool operator!=(self_type other) const EF_NOEXCEPT { return bits != other.bits; }

		EF_CONSTEXPR bool operator<(enum_flags const& other) const EF_NOEXCEPT { return bits < other.bits; }
		EF_CONSTEXPR bool operator>(enum_flags const& other) const EF_NOEXCEPT { return bits > other.bits; }
		EF_CONSTEXPR bool operator<=(enum_flags const& other) const EF_NOEXCEPT { return bits <= other.bits; }
		EF_CONSTEXPR bool operator>=(enum_flags const& other) const EF_NOEXCEPT { return bits >= other.bits; }

		EF_CONSTEXPR bool empty() const EF_NOEXCEPT { return bits == 0; }
		EF_CONSTEXPR void clear() EF_NOEXCEPT { bits = 0; }
	};
}
#undef EF_CONSTEXPR
#undef EF_EXPLICITOP
#undef EF_NOEXCEPT
#undef EF_NODISCARD
