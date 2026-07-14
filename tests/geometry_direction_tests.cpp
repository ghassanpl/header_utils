/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/geometry/direction.h"

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace ghassanpl::geometry;

TEST(direction, enum_layout_and_validity)
{
	EXPECT_EQ(int(direction::right), 0);
	EXPECT_EQ(int(direction::right_down), 1);
	EXPECT_EQ(int(direction::down), 2);
	EXPECT_EQ(int(direction::left_down), 3);
	EXPECT_EQ(int(direction::left), 4);
	EXPECT_EQ(int(direction::left_up), 5);
	EXPECT_EQ(int(direction::up), 6);
	EXPECT_EQ(int(direction::right_up), 7);
	EXPECT_EQ(int(direction::none), -1);

	/// Compass aliases (y-down screen coordinates: south == down).
	EXPECT_EQ(direction::east, direction::right);
	EXPECT_EQ(direction::south_east, direction::right_down);
	EXPECT_EQ(direction::south, direction::down);
	EXPECT_EQ(direction::north_west, direction::left_up);

	for (int i = 0; i < 8; ++i)
		EXPECT_TRUE(is_valid(direction(i))) << i;
	EXPECT_FALSE(is_valid(direction::none));
	EXPECT_FALSE(is_valid(direction(8)));

	/// Cardinals are even, diagonals odd; mutually exclusive for valid directions.
	for (int i = 0; i < 8; ++i)
	{
		EXPECT_EQ(is_cardinal(direction(i)), i % 2 == 0) << i;
		EXPECT_NE(is_cardinal(direction(i)), is_diagonal(direction(i))) << i;
	}
}

TEST(direction, arithmetic_works)
{
	/// Addition rotates clockwise (in y-down screen coordinates), wrapping.
	EXPECT_EQ(direction::right + 1, direction::right_down);
	EXPECT_EQ(direction::right_up + 1, direction::right);
	EXPECT_EQ(direction::right + 4, direction::left);
	EXPECT_EQ(direction::up + 3, direction::right_down);
	EXPECT_EQ(direction::right + 8, direction::right);

	/// Negative offsets wrap correctly instead of escaping the enum range.
	EXPECT_EQ(direction::right + (-1), direction::right_up);
	EXPECT_EQ(direction::right + (-9), direction::right_up);

	/// Subtraction rotates counter-clockwise (regression: used to be identical to addition).
	EXPECT_EQ(direction::right - 1, direction::right_up);
	EXPECT_EQ(direction::down - 2, direction::right);
	EXPECT_EQ(direction::right - 9, direction::right_up);
	EXPECT_EQ(direction::right - (-1), direction::right_down);

	/// Increment/decrement, pre and post.
	direction d = direction::right;
	EXPECT_EQ(++d, direction::right_down);
	EXPECT_EQ(d++, direction::right_down);
	EXPECT_EQ(d, direction::down);
	EXPECT_EQ(--d, direction::right_down);
	EXPECT_EQ(d--, direction::right_down);
	EXPECT_EQ(d, direction::right);

	EXPECT_EQ(opposite(direction::right), direction::left);
	EXPECT_EQ(opposite(direction::up), direction::down);
	EXPECT_EQ(opposite(direction::right_down), direction::left_up);

	EXPECT_EQ(shortest_distance(direction::down, direction::right), 2);
	EXPECT_EQ(shortest_distance(direction::right, direction::down), 2);
	EXPECT_EQ(shortest_distance(direction::left, direction::left), 0);
	EXPECT_EQ(shortest_distance(direction::right, direction::left), 4);
}

TEST(direction, cardinal_helpers_work)
{
	EXPECT_EQ(next_cardinal(direction::right), direction::down);
	EXPECT_EQ(next_cardinal(direction::right_down), direction::down);
	EXPECT_EQ(next_cardinal(direction::down), direction::left);
	EXPECT_EQ(next_cardinal(direction::right_up), direction::right);

	EXPECT_EQ(prev_cardinal(direction::right), direction::up);
	EXPECT_EQ(prev_cardinal(direction::right_down), direction::right);
	EXPECT_EQ(prev_cardinal(direction::left_down), direction::down);
	EXPECT_EQ(prev_cardinal(direction::right_up), direction::up);

	/// A cardinal maps to the set containing just itself.
	EXPECT_EQ(to_cardinal_set(direction::up), direction_set{ direction::up });
	/// A diagonal maps to its two components.
	EXPECT_EQ(to_cardinal_set(direction::right_down), (direction_set{ direction::right, direction::down }));
	EXPECT_EQ(to_cardinal_set(direction::left_up), (direction_set{ direction::left, direction::up }));

	/// Perpendicularity is symmetric (regression: used to depend on argument order).
	EXPECT_TRUE(are_perpendicular(direction::right, direction::down));
	EXPECT_TRUE(are_perpendicular(direction::down, direction::right));
	EXPECT_TRUE(are_perpendicular(direction::right, direction::up));
	EXPECT_TRUE(are_perpendicular(direction::right_down, direction::left_down)); /// diagonals can be perpendicular too
	EXPECT_FALSE(are_perpendicular(direction::right, direction::left));
	EXPECT_FALSE(are_perpendicular(direction::right, direction::right));
	EXPECT_FALSE(are_perpendicular(direction::right, direction::right_down));

	/// to_diagonal combines two perpendicular cardinals, in either order.
	/// (Regression: used to pick the diagonal on the wrong side, or return none.)
	EXPECT_EQ(to_diagonal(direction::right, direction::down), direction::right_down);
	EXPECT_EQ(to_diagonal(direction::down, direction::right), direction::right_down);
	EXPECT_EQ(to_diagonal(direction::right, direction::up), direction::right_up);
	EXPECT_EQ(to_diagonal(direction::up, direction::right), direction::right_up);
	EXPECT_EQ(to_diagonal(direction::left, direction::down), direction::left_down);
	EXPECT_EQ(to_diagonal(direction::down, direction::left), direction::left_down);
	EXPECT_EQ(to_diagonal(direction::left, direction::up), direction::left_up);
	EXPECT_EQ(to_diagonal(direction::up, direction::left), direction::left_up);

	/// Invalid combinations yield none.
	EXPECT_EQ(to_diagonal(direction::right, direction::left), direction::none);   /// opposite, not perpendicular
	EXPECT_EQ(to_diagonal(direction::right, direction::right), direction::none);
	EXPECT_EQ(to_diagonal(direction::right_down, direction::left_down), direction::none); /// diagonals not accepted
	EXPECT_EQ(to_diagonal(direction::none, direction::right), direction::none);
}

TEST(direction, offsets_and_vectors_work)
{
	/// (horizontal, vertical) for all eight directions, y-down.
	constexpr std::pair<int, int> expected[] = {
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 },
	};
	for (int i = 0; i < 8; ++i)
	{
		const auto dir = direction(i);
		EXPECT_EQ(horizontal(dir), expected[i].first) << to_name(dir);
		EXPECT_EQ(vertical(dir), expected[i].second) << to_name(dir);
		EXPECT_EQ(to_ivec(dir), (glm::ivec2{ expected[i].first, expected[i].second })) << to_name(dir);
	}

	EXPECT_TRUE(is_only_vertical(direction::up));
	EXPECT_TRUE(is_only_vertical(direction::down));
	EXPECT_FALSE(is_only_vertical(direction::right));
	EXPECT_FALSE(is_only_vertical(direction::right_down));

	EXPECT_TRUE(is_only_horizontal(direction::left));
	EXPECT_TRUE(is_only_horizontal(direction::right));
	EXPECT_FALSE(is_only_horizontal(direction::up));
	EXPECT_FALSE(is_only_horizontal(direction::left_up));

	/// Cardinal vectors are unchanged...
	EXPECT_EQ(to_vec(direction::right), (glm::vec2{ 1.0f, 0.0f }));
	EXPECT_EQ(to_vec(direction::up), (glm::vec2{ 0.0f, -1.0f }));

	/// ...and diagonal vectors are normalized to unit length (regression: used to be scaled UP by sqrt2).
	for (auto dir : { direction::right_down, direction::left_down, direction::left_up, direction::right_up })
	{
		const auto v = to_vec(dir);
		EXPECT_NEAR(glm::length(v), 1.0f, 1e-6f) << to_name(dir);
		/// Direction is preserved.
		EXPECT_EQ(glm::sign(v.x), float(horizontal(dir))) << to_name(dir);
		EXPECT_EQ(glm::sign(v.y), float(vertical(dir))) << to_name(dir);
	}
}

TEST(direction, angles_work)
{
	EXPECT_FLOAT_EQ(to_angle(direction::right).get(), 0.0f);
	EXPECT_FLOAT_EQ(to_angle(direction::right_down).get(), 45.0f);
	EXPECT_FLOAT_EQ(to_angle(direction::down).get(), 90.0f);
	EXPECT_FLOAT_EQ(to_angle(direction::up).get(), 270.0f);

	EXPECT_EQ(to_direction(degrees{ 0.0f }), direction::right);
	EXPECT_EQ(to_direction(degrees{ 45.0f }), direction::right_down);
	EXPECT_EQ(to_direction(degrees{ 90.0f }), direction::down);
	EXPECT_EQ(to_direction(degrees{ 270.0f }), direction::up);
	EXPECT_EQ(to_direction(degrees{ 315.0f }), direction::right_up);

	/// Sector boundaries: each direction owns +/- 22.5 degrees around its angle.
	EXPECT_EQ(to_direction(degrees{ 22.4f }), direction::right);
	EXPECT_EQ(to_direction(degrees{ 22.6f }), direction::right_down);
	EXPECT_EQ(to_direction(degrees{ 359.0f }), direction::right);

	/// Angles outside [0, 360) wrap (regression: negative angles used to produce none or right).
	EXPECT_EQ(to_direction(degrees{ 360.0f }), direction::right);
	EXPECT_EQ(to_direction(degrees{ 450.0f }), direction::down);
	EXPECT_EQ(to_direction(degrees{ -90.0f }), direction::up);
	EXPECT_EQ(to_direction(degrees{ -45.0f }), direction::right_up);
	EXPECT_EQ(to_direction(degrees{ -10.0f }), direction::right);

	/// to_angle and to_direction are inverses for all eight directions.
	for (int i = 0; i < 8; ++i)
		EXPECT_EQ(to_direction(to_angle(direction(i))), direction(i)) << i;

	/// From (float) vectors, y-down.
	EXPECT_EQ(to_direction(glm::vec2{ 1.0f, 0.0f }), direction::right);
	EXPECT_EQ(to_direction(glm::vec2{ 0.0f, 1.0f }), direction::down);
	EXPECT_EQ(to_direction(glm::vec2{ 1.0f, 1.0f }), direction::right_down);
	EXPECT_EQ(to_direction(glm::vec2{ -1.0f, -1.0f }), direction::left_up);
	EXPECT_EQ(to_direction(glm::vec2{ 0.0f, -1.0f }), direction::up);
	EXPECT_EQ(to_direction(glm::vec2{ 10.0f, 0.5f }), direction::right); /// magnitude doesn't matter

	/// From integer vectors: uses component signs only.
	EXPECT_EQ(to_direction(glm::ivec2{ 0, 0 }), direction::none);
	EXPECT_EQ(to_direction(glm::ivec2{ 5, 0 }), direction::right);
	EXPECT_EQ(to_direction(glm::ivec2{ 3, 3 }), direction::right_down);
	EXPECT_EQ(to_direction(glm::ivec2{ -2, 0 }), direction::left);
	EXPECT_EQ(to_direction(glm::ivec2{ 0, -7 }), direction::up);
	EXPECT_EQ(to_direction(glm::ivec2{ -1, 1 }), direction::left_down);
}

TEST(direction, alignment_roundtrip_works)
{
	EXPECT_EQ(to_alignment(direction::right), align::middle_right);
	EXPECT_EQ(to_alignment(direction::left_up), align::top_left);
	EXPECT_EQ(to_alignment(direction::down), align::bottom_center);
	EXPECT_EQ(to_alignment(direction::none), align::center);

	EXPECT_EQ(to_direction(align::center), direction::none);
	for (int i = 0; i < 8; ++i)
		EXPECT_EQ(to_direction(to_alignment(direction(i))), direction(i)) << i;
}

TEST(direction, names_work)
{
	EXPECT_STREQ(to_name(direction::right), "right");
	EXPECT_STREQ(to_name(direction::right_down), "right down");
	EXPECT_STREQ(to_name(direction::up), "up");

	EXPECT_STREQ(to_compass_name(direction::right), "east");
	EXPECT_STREQ(to_compass_name(direction::left_down), "south west");
	EXPECT_STREQ(to_compass_name(direction::up), "north");

	/// The name tables follow enum order for all values.
	for (int i = 0; i < 8; ++i)
	{
		EXPECT_STREQ(to_name(direction(i)), names::all_directions[i]);
		EXPECT_STREQ(to_compass_name(direction(i)), names::compass::all_directions[i]);
	}
}

TEST(direction, direction_sets_work)
{
	EXPECT_EQ(all_cardinal_directions.count(), 4);
	EXPECT_EQ(all_diagonal_directions.count(), 4);
	EXPECT_EQ(all_directions.count(), 8);

	for (int i = 0; i < 8; ++i)
	{
		const auto dir = direction(i);
		EXPECT_TRUE(all_directions.is_set(dir)) << to_name(dir);
		EXPECT_EQ(all_cardinal_directions.is_set(dir), is_cardinal(dir)) << to_name(dir);
		EXPECT_EQ(all_diagonal_directions.is_set(dir), is_diagonal(dir)) << to_name(dir);
	}
}

TEST(direction, bends_work)
{
	/// get_bend(first, second) is a directed path: travel `first`, then turn to travel `second`.
	/// The corner glyph connects the edge we entered from (opposite `first`) and the edge we exit (`second`).

	/// Entering from the left (traveling right), then turning:
	EXPECT_EQ(get_bend(direction::right, direction::down), bend::up_right);   /// enter left, exit down  => ⌝
	EXPECT_EQ(get_bend(direction::right, direction::up), bend::down_right);   /// enter left, exit up    => ⌟
	/// Entering from the right (traveling left), then turning:
	EXPECT_EQ(get_bend(direction::left, direction::down), bend::up_left);     /// enter right, exit down => ⌜
	EXPECT_EQ(get_bend(direction::left, direction::up), bend::down_left);     /// enter right, exit up   => ⌞
	/// Entering from the top (traveling down), then turning:
	EXPECT_EQ(get_bend(direction::down, direction::right), bend::down_left);  /// enter top, exit right  => ⌞
	EXPECT_EQ(get_bend(direction::down, direction::left), bend::down_right);  /// enter top, exit left   => ⌟
	/// Entering from the bottom (traveling up), then turning:
	EXPECT_EQ(get_bend(direction::up, direction::right), bend::up_left);      /// enter bottom, exit right => ⌜
	EXPECT_EQ(get_bend(direction::up, direction::left), bend::up_right);      /// enter bottom, exit left  => ⌝

	/// Order matters, since the path is directed: right-then-down curves the opposite way to down-then-right.
	EXPECT_NE(get_bend(direction::right, direction::down), get_bend(direction::down, direction::right));

	/// Walking the same physical corner in reverse traces the same glyph:
	/// get_bend(a, b) == get_bend(opposite(b), opposite(a)).
	const std::pair<direction, direction> turns[] = {
		{ direction::right, direction::down }, { direction::right, direction::up },
		{ direction::left, direction::down },  { direction::left, direction::up },
		{ direction::down, direction::right }, { direction::down, direction::left },
		{ direction::up, direction::right },   { direction::up, direction::left },
	};
	for (auto [first, second] : turns)
		EXPECT_EQ(get_bend(first, second), get_bend(opposite(second), opposite(first)))
			<< to_name(first) << " then " << to_name(second);
}
