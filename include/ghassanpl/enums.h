/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp17.h"

#include <magic_enum.hpp>
#include <array>

namespace ghassanpl
{
	namespace enums
	{
		using namespace magic_enum;
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

		using enum_type = ENUM_TYPE;

		[[nodiscard]] constexpr reference at(ENUM_TYPE pos) {
			return this->array_type::at(static_cast<size_type>(pos));
		}

		[[nodiscard]] constexpr const_reference at(ENUM_TYPE pos) const {
			return this->array_type::at(static_cast<size_type>(pos));
		}

		[[nodiscard]] constexpr reference operator[](ENUM_TYPE pos) noexcept {
			return this->array_type::operator[](static_cast<size_type>(pos));
		}

		[[nodiscard]] constexpr const_reference operator[](ENUM_TYPE pos) const noexcept {
			return this->array_type::operator[](static_cast<size_type>(pos));
		}
	};
}