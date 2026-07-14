/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "../include/ghassanpl/containers.h"

#include <memory>
#include <string>
#include <vector>

TEST(map_find_value, works)
{
	std::map<int, std::string> m;
	auto [it, ins] = m.emplace(0, "hello");
	EXPECT_NE(ghassanpl::map_find_value(m, &it->second), nullptr);
}

using ghassanpl::move_to_end;
using ghassanpl::move_to_start;

TEST(move_to_end, moves_element_to_the_back_preserving_the_order_of_the_rest)
{
	std::vector<int> v = { 1, 2, 3, 4, 5 };

	move_to_end(v, v.begin() + 2);
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 4, 5, 3 }));

	move_to_end(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 2, 4, 5, 3, 1 }));
}

TEST(move_to_end, moving_the_last_element_is_a_noop)
{
	std::vector<int> v = { 1, 2, 3 };
	move_to_end(v, std::prev(v.end()));
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3 }));
}

TEST(move_to_end, moving_the_end_iterator_is_a_noop)
{
	std::vector<int> v = { 1, 2, 3 };
	move_to_end(v, v.end());
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3 }));
}

TEST(move_to_end, works_on_single_element_vectors)
{
	std::vector<int> v = { 42 };
	move_to_end(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 42 }));
}

TEST(move_to_end, works_on_empty_vectors)
{
	std::vector<int> v;
	move_to_end(v, v.begin());
	EXPECT_TRUE(v.empty());
}

TEST(move_to_end, does_not_change_the_size_of_the_vector)
{
	std::vector<int> v = { 1, 2, 3, 4, 5 };
	const auto size = v.size();
	move_to_end(v, v.begin() + 1);
	EXPECT_EQ(v.size(), size);
}

TEST(move_to_end, works_with_non_trivial_types)
{
	std::vector<std::string> v = { "a", "b", "c", "d" };
	move_to_end(v, v.begin() + 1);
	EXPECT_EQ(v, (std::vector<std::string>{ "a", "c", "d", "b" }));
}

TEST(move_to_end, works_with_move_only_types)
{
	std::vector<std::unique_ptr<int>> v;
	v.push_back(std::make_unique<int>(1));
	v.push_back(std::make_unique<int>(2));
	v.push_back(std::make_unique<int>(3));
	const auto* const moved = v[0].get();

	move_to_end(v, v.begin());

	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(*v[0], 2);
	EXPECT_EQ(*v[1], 3);
	EXPECT_EQ(v[2].get(), moved);
}

TEST(move_to_end, repeated_calls_cycle_the_elements)
{
	std::vector<int> v = { 1, 2, 3 };
	move_to_end(v, v.begin());
	move_to_end(v, v.begin());
	move_to_end(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3 }));
}

TEST(move_to_end, is_constexpr)
{
	static_assert([] {
		std::vector<int> v = { 1, 2, 3, 4 };
		move_to_end(v, v.begin() + 1);
		return v == std::vector<int>{ 1, 3, 4, 2 };
	}());
}

TEST(move_to_start, moves_element_to_the_front_preserving_the_order_of_the_rest)
{
	std::vector<int> v = { 1, 2, 3, 4, 5 };

	move_to_start(v, v.begin() + 2);
	EXPECT_EQ(v, (std::vector<int>{ 3, 1, 2, 4, 5 }));

	move_to_start(v, std::prev(v.end()));
	EXPECT_EQ(v, (std::vector<int>{ 5, 3, 1, 2, 4 }));
}

TEST(move_to_start, moving_the_first_element_is_a_noop)
{
	std::vector<int> v = { 1, 2, 3 };
	move_to_start(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3 }));
}

TEST(move_to_start, moving_the_end_iterator_is_a_noop)
{
	std::vector<int> v = { 1, 2, 3 };
	move_to_start(v, v.end());
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3 }));
}

TEST(move_to_start, works_on_single_element_vectors)
{
	std::vector<int> v = { 42 };
	move_to_start(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 42 }));
}

TEST(move_to_start, works_on_empty_vectors)
{
	std::vector<int> v;
	move_to_start(v, v.begin());
	EXPECT_TRUE(v.empty());
}

TEST(move_to_start, does_not_change_the_size_of_the_vector)
{
	std::vector<int> v = { 1, 2, 3, 4, 5 };
	const auto size = v.size();
	move_to_start(v, v.begin() + 3);
	EXPECT_EQ(v.size(), size);
}

TEST(move_to_start, works_with_non_trivial_types)
{
	std::vector<std::string> v = { "a", "b", "c", "d" };
	move_to_start(v, v.begin() + 2);
	EXPECT_EQ(v, (std::vector<std::string>{ "c", "a", "b", "d" }));
}

TEST(move_to_start, works_with_move_only_types)
{
	std::vector<std::unique_ptr<int>> v;
	v.push_back(std::make_unique<int>(1));
	v.push_back(std::make_unique<int>(2));
	v.push_back(std::make_unique<int>(3));
	const auto* const moved = v[2].get();

	move_to_start(v, std::prev(v.end()));

	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(v[0].get(), moved);
	EXPECT_EQ(*v[1], 1);
	EXPECT_EQ(*v[2], 2);
}

TEST(move_to_start, repeated_calls_cycle_the_elements)
{
	std::vector<int> v = { 1, 2, 3 };
	move_to_start(v, std::prev(v.end()));
	move_to_start(v, std::prev(v.end()));
	move_to_start(v, std::prev(v.end()));
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3 }));
}

TEST(move_to_start, is_constexpr)
{
	static_assert([] {
		std::vector<int> v = { 1, 2, 3, 4 };
		move_to_start(v, v.begin() + 2);
		return v == std::vector<int>{ 3, 1, 2, 4 };
	}());
}

TEST(move_to_end_and_start, are_inverses_of_each_other)
{
	std::vector<int> v = { 1, 2, 3, 4, 5 };

	move_to_end(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 2, 3, 4, 5, 1 }));
	move_to_start(v, std::prev(v.end()));
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 3, 4, 5 }));

	/// moving a middle element to the end and back restores the original order
	move_to_end(v, v.begin() + 2);
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 4, 5, 3 }));
	move_to_start(v, std::prev(v.end()));
	EXPECT_EQ(v, (std::vector<int>{ 3, 1, 2, 4, 5 }));
	move_to_end(v, v.begin());
	EXPECT_EQ(v, (std::vector<int>{ 1, 2, 4, 5, 3 }));
}