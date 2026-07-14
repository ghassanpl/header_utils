/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "../align.h"

#include <numbers>

namespace ghassanpl::geometry
{
	/// \defgroup Direction Direction
	/// Contains the `direction` type, which represents one of the eight cardinal compass directions
	/// 
	/// \ingroup Geometry
	/// @{

	enum class direction
	{
		none = -1,

		right = 0,	/// 0b00000
		right_down, /// 0b00001
		down,       /// 0b00010
		left_down,  /// 0b00011
		left,       /// 0b00100
		left_up,    /// 0b00101
		up,         /// 0b00110
		right_up,   /// 0b00111

		east = right,
		south_east = right_down,
		south = down,
		south_west = left_down,
		west = left,
		north_west = left_up,
		north = up,
		north_east = right_up,
	};

	static constexpr auto direction_count = 8;
	
	using direction_set = enum_flags<direction, uint8_t>;

	namespace detail
	{
		static constexpr const int direction_value[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	}

	constexpr direction operator+(direction dir, int d) { return (direction)(((int(dir) + d) % 8 + 8) % 8); }
	constexpr direction operator-(direction dir, int d) { return dir + (-d); }

	constexpr direction& operator++(direction& dir) { return dir = dir + 1; }
	constexpr direction& operator--(direction& dir) { return dir = dir + 7; }

	constexpr direction operator++(direction& dir, int) { auto res = dir; ++dir; return res; }
	constexpr direction operator--(direction& dir, int) { auto res = dir; --dir; return res; }

	constexpr direction opposite(direction dir) { return dir + 4; }
	constexpr direction next_cardinal(direction dir) { return direction(int(dir) & (~1)) + 2; }
	constexpr direction prev_cardinal(direction dir) { return direction(int(dir + 1) & (~1)) + 6; }
	
	/// Returns the smallest (by absolute value) offset such that `d1 + offset == d2`.
	constexpr int shortest_offset(direction d1, direction d2)
	{
		const auto diff = ((int(d2) - int(d1)) + 8) % 8; /// clockwise steps from d1 to d2, in [0, 7]
		return diff > 4 ? diff - 8 : diff;               /// fold the long way around into negative steps
	}

	constexpr int shortest_distance(direction d1, direction d2)
	{
		return cem::abs(shortest_offset(d1, d2));
	}

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

	constexpr bool are_perpendicular(direction d1, direction d2) { return shortest_distance(d1, d2) == 2; }

	/// Turns a diagonal direction into a set of cardinals (e.g. `to_cardinal_set(north_west)` -> `direction_set{north, west}`)
	constexpr direction_set to_cardinal_set(direction dir)
	{
		if (is_cardinal(dir)) return { dir };
		return { next_cardinal(dir), prev_cardinal(dir) };
	}

	/// Turns two perpendicular cardinal directions into a single diagonal direction (in either argument order)
	constexpr direction to_diagonal(direction c1, direction c2)
	{
		if (!is_cardinal(c1) || !is_cardinal(c2) || shortest_distance(c1, c2) != 2) return direction::none;
		return c1 + shortest_offset(c1, c2) / 2;
	}

	/// Returns the horizontal offset (-1, 0, or 1) of `dir`
	constexpr int horizontal(direction dir) { return detail::direction_value[(int)dir]; }

	//constexpr bool has_vertical(direction dir) { ... }
	constexpr bool is_only_vertical(direction dir) { return horizontal(dir) == 0; }
	
	/// Returns the vertical offset (-1, 0, or 1) of `dir`
	constexpr int vertical(direction dir) { return detail::direction_value[int(dir + 6)]; }

	constexpr bool is_only_horizontal(direction dir) { return vertical(dir) == 0; }

	constexpr degrees to_angle(direction val)
	{
		return degrees{ (float(val) * 45.0f) };
	}

	/// \internal
	/// TODO: Do we really want to include the entirety of geometry_common.h (which is a BIG header) just for the few functions below?
	/// \endinternal

	constexpr glm::ivec2 to_ivec(direction val) { return { horizontal(val), vertical(val) }; }
	constexpr glm::vec2 to_vec(direction val) { const glm::vec2 d = to_ivec(val); return is_diagonal(val) ? d / std::numbers::sqrt2_v<float> : d; }

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
		/// ensure_positive first: cem::fmod keeps the dividend's sign, so negative angles would otherwise truncate to `right` or `none`
		const auto positive = angles::ensure_positive(angle).get() + (45.0f / 2.0f);
		return direction(int(cem::fmod(positive, 360.0f) / 45.0f) % 8);
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

	/// @}

	enum class bend
	{
		up_left, /// ⌜
		up_right, /// ⌝
		down_left, /// ⌞
		down_right, /// ⌟
	};

	/// Given a path that travels in direction `first` and then turns to travel in direction `second`,
	/// returns the corner glyph traced at the turn (e.g. `get_bend(right, down)` -> `up_right` ⌝: the
	/// path enters from the left and leaves downward). Order matters, since the path is directed.
	/// \pre `first` and `second` must be perpendicular cardinal directions.
	constexpr bend get_bend(direction first, direction second)
	{
		const direction incoming = opposite(first);                    /// the arm to the previous cell points back the way we came
		const int h = horizontal(incoming) + horizontal(second);       /// +1 for a rightward arm, -1 for a leftward one
		const int v = vertical(incoming) + vertical(second);           /// +1 for a downward arm, -1 for an upward one
		const int corner_is_right = (1 - h) / 2;                       /// arm points left  => corner is on the right
		const int corner_is_down  = (1 - v) / 2;                       /// arm points up    => corner is at the bottom
		return bend(corner_is_down * 2 + corner_is_right);
	}
}