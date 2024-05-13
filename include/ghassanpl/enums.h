/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"

/// TODO: Make magic_enum optional, and force the user to define the array count if not available.
#include <magic_enum.hpp>
#include <magic_enum_utility.hpp>
#include <array>

namespace ghassanpl
{
	namespace enums
	{
		using namespace magic_enum;
	}
	
	/// TODO: Finish this
	template <typename ENUM_TYPE>
	GHPL_REQUIRES(std::is_enum_v<ENUM_TYPE>)
	struct circular_enum
	{
		using enum_type = ENUM_TYPE;
		using underlying_type = std::underlying_type_t<enum_type>;
		enum_type value;

		//auto& operator=(enum_type val) noexcept { value = val; return *this; }
		circular_enum& operator++() noexcept {
			value = magic_enum::enum_next_value_circular(value);
			return *this;
		}
		operator enum_type() const noexcept { return value; }
	};
	
	template <typename ENUM_TYPE>
	GHPL_REQUIRES(std::is_enum_v<ENUM_TYPE>)
	constexpr auto IncWrap(ENUM_TYPE& val) noexcept
	{
		val = magic_enum::enum_next_value_circular(val);
		return val;
	}
	
	template <typename ENUM_TYPE>
	GHPL_REQUIRES(std::is_enum_v<ENUM_TYPE>)
	constexpr auto DecWrap(ENUM_TYPE& val) noexcept
	{
		val = magic_enum::enum_prev_value_circular(val);
		return val;
	}

	/// Same as `std::array` except it takes an enum type instead of the size, and uses `magic_enum` to determine the array size; 
	/// also allows for indexing with the enum type.
	template <typename VALUE_TYPE, typename ENUM_TYPE>
	GHPL_REQUIRES(std::is_enum_v<ENUM_TYPE>)
	struct enum_array : public std::array<VALUE_TYPE, magic_enum::enum_count<ENUM_TYPE>()>
	{
		using array_type = std::array<VALUE_TYPE, magic_enum::enum_count<ENUM_TYPE>()>;

		using value_type = VALUE_TYPE;
		using size_type = typename array_type::size_type;
		using difference_type = typename array_type::difference_type;
		using pointer = typename array_type::pointer;
		using const_pointer = typename array_type::const_pointer;
		using reference = typename array_type::reference;
		using const_reference = typename array_type::const_reference;
		using iterator = typename array_type::iterator;
		using const_iterator = typename array_type::const_iterator;
		using reverse_iterator = typename array_type::reverse_iterator;
		using const_reverse_iterator = typename array_type::const_reverse_iterator;

		/// TODO: Some way to iterate over pairs of {index (as enum), value}

		using enum_type = ENUM_TYPE;

		using array_type::at;

		[[nodiscard]] constexpr reference at(ENUM_TYPE pos) {
			return this->array_type::at(magic_enum::enum_index(pos).value());
		}

		[[nodiscard]] constexpr const_reference at(ENUM_TYPE pos) const {
			return this->array_type::at(magic_enum::enum_index(pos).value());
		}

		using array_type::operator[];

		[[nodiscard]] constexpr reference operator[](ENUM_TYPE pos) noexcept {
			return this->array_type::operator[](magic_enum::enum_index(pos).value());
		}

		[[nodiscard]] constexpr const_reference operator[](ENUM_TYPE pos) const noexcept {
			return this->array_type::operator[](magic_enum::enum_index(pos).value());
		}
	};
}