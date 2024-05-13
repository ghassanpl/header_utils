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
	constexpr direction prev_cardinal(direction dir) { return direction(int(dir + 1) & (~1)) + 6; }

	constexpr inline direction_set all_cardinal_directions{ direction::left, direction::right, direction::up, direction::down };
	constexpr inline direction_set all_diagonal_directions{ direction::left_up, direction::right_up, direction::right_down, direction::left_down };
	constexpr inline direction_set all_directions{ direction::left, direction::right, direction::up, direction::down, direction::left_up, direction::right_up, direction::right_down, direction::left_down };

	namespace names
	{
		namespace compass
		{
			constexpr inline const char* cardinal_directions[] = { "east", "south", "west", "north" };
			constexpr inline const char* diagonal_directions[] = { "south east", "south west", "north west", "north east" };
			constexpr inline const char* all_directions[] = { "east", "south east", "south", "south west", "west", "north west", "north", "north east" };
		}
		
		constexpr inline const char* cardinal_directions[] = { "right", "down", "left", "up" };
		constexpr inline const char* diagonal_directions[] = { "right down", "left down", "left up", "right up" };
		constexpr inline const char* all_directions[] = { "right", "right down", "down", "left down", "left", "left up", "up", "right up" };
	}

	constexpr const char* to_name(direction dir) { return names::all_directions[int(dir)]; }
	constexpr const char* to_compass_name(direction dir) { return names::compass::all_directions[int(dir)]; }

	constexpr bool is_valid(direction dir) { return int(dir) >= 0 && int(dir) <= 7; }
	constexpr bool is_cardinal(direction dir) { return (int(dir) & 1) == 0; }
	constexpr bool is_diagonal(direction dir) { return (int(dir) & 1) != 0; }

	constexpr direction_set to_cardinal_set(direction dir)
	{
		if (is_cardinal(dir)) return { dir };
		return { next_cardinal(dir), prev_cardinal(dir) };
	}

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

	constexpr align to_alignment(direction val)
	{
		switch (val)
		{
		case direction::right: return align::middle_right;
		case direction::right_down: return align::bottom_right;
		case direction::down: return align::bottom_center;
		case direction::left_down: return align::bottom_left;
		case direction::left: return align::middle_left;
		case direction::left_up: return align::top_left;
		case direction::up: return align::top_center;
		case direction::right_up: return align::top_right;
		default: return align::center;
		}
	}

	constexpr direction to_direction(align val)
	{
		switch (val)
		{
		case align::middle_right: return direction::right;
		case align::bottom_right: return direction::right_down;
		case align::bottom_center: return direction::down;
		case align::bottom_left: return direction::left_down;
		case align::middle_left: return direction::left;
		case align::top_left: return direction::left_up;
		case align::top_center: return direction::up;
		case align::top_right: return direction::right_up;
		default: return direction::none;
		}
	}

	constexpr direction to_direction(degrees angle)
	{
		return direction(int(cem::fmod(angle.get() + (45.0 / 2.0), 360.0) / 45.0) % 8);
	}

	constexpr direction to_direction(glm::vec2 val)
	{
		return to_direction(geometry::angles::ensure_positive(degrees{ glm::degrees(glm::atan(val.y, val.x)) }));
	}

	constexpr direction to_direction(glm::ivec2 vec)
	{
		constexpr const int vec_value[] = { 5, 6, 7, 4, -1, 0, 3, 2, 1 };
		return (direction)vec_value[glm::sign(vec.x) + glm::sign(vec.y) * 3 + 4];
	}

}