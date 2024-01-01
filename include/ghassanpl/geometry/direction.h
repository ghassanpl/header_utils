/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

#include <numbers>

namespace ghassanpl::geometry
{
	enum class direction
	{
		none = -1,

		right = 0,
		right_down,
		down,
		left_down,
		left,
		left_up,
		up,
		right_up,

		east = 0,
		south_east,
		south,
		south_west,
		west,
		north_west,
		north,
		north_east
	};

	static constexpr auto direction_count = 8;

	/// TODO: names, compass name

	using direction_set = enum_flags<direction, uint8_t>;

	namespace detail
	{
		static constexpr const int direction_value[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	}

	constexpr direction operator+(direction dir, int d) { return (direction)((int(dir) + d) % 8); }
	constexpr direction operator-(direction dir, int d) { return (direction)((int(dir) + (8 + (d % 8))) % 8); }

	constexpr direction& operator++(direction& dir) { return dir = dir + 1; }
	constexpr direction& operator--(direction& dir) { return dir = dir + 7; }

	constexpr direction operator++(direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr direction operator--(direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr direction opposite(direction dir) { return dir + 4; }
	constexpr direction next_cardinal(direction dir) { return direction(int(dir) & (~1)) + 2; }
	constexpr direction prev_cardinal(direction dir) { return direction(int(dir) & (~1)) + 6; }

	constexpr inline static direction_set all_cardinal_directions{ direction::left, direction::right, direction::up, direction::down };
	constexpr inline static direction_set all_diagonal_directions{ direction::left_up, direction::right_up, direction::right_down, direction::left_down };
	constexpr inline static direction_set all_directions{ direction::left, direction::right, direction::up, direction::down, direction::left_up, direction::right_up, direction::right_down, direction::left_down };

	constexpr bool is_valid(direction dir) { return int(dir) >= 0 && int(dir) <= 7; }
	constexpr bool is_cardinal(direction dir) { return (int(dir) & 1) == 0; }
	constexpr bool is_diagonal(direction dir) { return (int(dir) & 1) != 0; }

	constexpr int horizontal(direction dir) { return detail::direction_value[(int)dir]; }
	constexpr int vertical(direction dir) { return detail::direction_value[int(dir + 6)]; }

	constexpr degrees to_angle(direction val)
	{
		return degrees{ (float(val) * 45.0f) };
	}

	/// \internal
	/// TODO: Do we really want to include the entirety of geometry_common.h (which is a BIG header) just for the few functions below?
	/// \endinternal

	constexpr glm::ivec2 to_ivec(direction val) { return { horizontal(val), vertical(val) }; }
	constexpr glm::vec2 to_vec(direction val) { const glm::vec2 d = to_ivec(val); return is_diagonal(val) ? d * std::numbers::sqrt2_v<float> : d; }

	constexpr ghassanpl::align to_alignment(direction val);

	/// \internal
	/// TODO: Use constexpr math for fmod and sign to make `to_direction` functions constexpr
	/// \endinternal

	inline direction to_direction(degrees angle)
	{
		return direction(int(glm::mod(angle.get() + (45.0 / 2.0), 360.0) / 45.0) % 8);
	}

	inline direction to_direction(glm::vec2 val);
	inline direction to_direction(glm::ivec2 vec)
	{
		constexpr const int vec_value[] = { 5, 6, 7, 4, -1, 0, 3, 2, 1 };
		return (direction)vec_value[glm::sign(vec.x) + glm::sign(vec.y) * 3 + 4];
	}

}