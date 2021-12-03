/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/align.h"

#include <gtest/gtest.h>

#include <set>
#include <algorithm>
#include <numeric>
#include <vector>
#include <magic_enum.hpp>
#if __has_include(<experimental/generator>) && !defined(__clang__)
#include <experimental/generator>

template <typename RANGE1, typename RANGE2>
auto cartesian_product(RANGE1&& r1, RANGE2&& r2) -> std::experimental::generator<std::pair<std::ranges::range_value_t<RANGE1>, std::ranges::range_value_t<RANGE2>>>
{
  for (auto& e1 : r1)
    for (auto& e2 : r2)
      co_yield std::pair{ e1, e2 };
}

#else
template <typename RANGE1, typename RANGE2>
auto cartesian_product(RANGE1&& r1, RANGE2&& r2)
{
  std::vector<std::pair<std::ranges::range_value_t<RANGE1>, std::ranges::range_value_t<RANGE2>>> result;
  for (auto&& e1 : std::forward<RANGE1>(r1))
    for (auto&& e2 : std::forward<RANGE2>(r2))
      result.emplace_back(std::forward<std::ranges::range_value_t<RANGE1>>(e1), std::forward<std::ranges::range_value_t<RANGE2>>(e2));
  return result;
}
#endif
#include <ranges>

using namespace ghassanpl;

const std::set<align> all_aligns{
  align::top_left, align::middle_left, align::bottom_left,
  align::top_center, align::middle_center, align::bottom_center,
  align::top_right, align::middle_right, align::bottom_right,
};

const std::set<horizontal_align> horizontal_aligns{
  horizontal_align::left, horizontal_align::center, horizontal_align::right
};

const std::set<vertical_align> vertical_aligns{
  vertical_align::top, vertical_align::bottom, vertical_align::middle
};

TEST(alignment_test, basics_work)
{
  align a{ align::top_left };
  EXPECT_EQ(horizontal_align::left | vertical_align::bottom, a |= vertical_align::bottom);

  ASSERT_EQ(vertical_aligns.size(), 3);

  ASSERT_EQ(vertical_aligns.size(), 3);

  ASSERT_EQ(vertical_aligns.size(), 3);

  auto range = cartesian_product(vertical_aligns, horizontal_aligns);
  auto other_range = range | std::ranges::views::transform([](auto p) { return p.first | p.second; });
  ASSERT_EQ(all_aligns, (std::set<align> { other_range.begin(), other_range.end() }));
}

TEST(alignment_test, names_work)
{
#define E(t, a) EXPECT_EQ(#a, t##_names[int(t::a)])
  E(horizontal_align, left);
  E(horizontal_align, center);
  E(horizontal_align, right);
  E(vertical_align, top);
  E(vertical_align, middle);
  E(vertical_align, bottom);

  E(align, top_left);
  E(align, middle_left);
  E(align, bottom_left);
  E(align, top_center);
  E(align, middle_center);
  E(align, bottom_center);
  E(align, top_right);
  E(align, middle_right);
  E(align, bottom_right);

  for (auto v : vertical_aligns_in_order)
  {
    EXPECT_EQ(magic_enum::enum_name(v), to_name(v));
  }
  for (auto h : horizontal_aligns_in_order)
  {
    EXPECT_EQ(magic_enum::enum_name(h), to_name(h));
  }
  for (auto a : aligns_in_order)
  {
    EXPECT_EQ(magic_enum::enum_name(a), to_name(a));
  }
}

TEST(alignment_test, conversions_work)
{
  for (auto v : vertical_aligns_in_order)
  {
    ASSERT_EQ(to_vertical(to_horizontal(v)), v);
  }
  for (auto h : horizontal_aligns_in_order)
  {
    ASSERT_EQ(to_horizontal(to_vertical(h)), h);
  }

  EXPECT_EQ(to_opposite(horizontal_align::left), horizontal_align::right);
  EXPECT_EQ(to_opposite(horizontal_align::center), horizontal_align::center);
  EXPECT_EQ(to_opposite(horizontal_align::right), horizontal_align::left);

  EXPECT_EQ(to_opposite(vertical_align::top), vertical_align::bottom);
  EXPECT_EQ(to_opposite(vertical_align::middle), vertical_align::middle);
  EXPECT_EQ(to_opposite(vertical_align::bottom), vertical_align::top);

  EXPECT_EQ(to_opposite(align::top_left), align::bottom_right);
  EXPECT_EQ(to_opposite(align::top_center), align::bottom_center);
  EXPECT_EQ(to_opposite(align::top_right), align::bottom_left);
  EXPECT_EQ(to_opposite(align::middle_left), align::middle_right);
  EXPECT_EQ(to_opposite(align::center), align::center);
  EXPECT_EQ(to_opposite(align::middle_right), align::middle_left);
  EXPECT_EQ(to_opposite(align::bottom_left), align::top_right);
  EXPECT_EQ(to_opposite(align::bottom_center), align::top_center);
  EXPECT_EQ(to_opposite(align::bottom_right), align::top_left);

  EXPECT_EQ(rotated_clockwise(align::top_left), align::top_right);
  EXPECT_EQ(rotated_clockwise(align::top_center), align::middle_right);
  EXPECT_EQ(rotated_clockwise(align::top_right), align::bottom_right);
  EXPECT_EQ(rotated_clockwise(align::middle_left), align::top_center);
  EXPECT_EQ(rotated_clockwise(align::center), align::center);
  EXPECT_EQ(rotated_clockwise(align::middle_right), align::bottom_center);
  EXPECT_EQ(rotated_clockwise(align::bottom_left), align::top_left);
  EXPECT_EQ(rotated_clockwise(align::bottom_center), align::middle_left);
  EXPECT_EQ(rotated_clockwise(align::bottom_right), align::bottom_left);

  EXPECT_EQ(rotated_counter_clockwise(align::top_left), align::bottom_left);
  EXPECT_EQ(rotated_counter_clockwise(align::top_center), align::middle_left);
  EXPECT_EQ(rotated_counter_clockwise(align::top_right), align::top_left);
  EXPECT_EQ(rotated_counter_clockwise(align::middle_left), align::bottom_center);
  EXPECT_EQ(rotated_counter_clockwise(align::center), align::center);
  EXPECT_EQ(rotated_counter_clockwise(align::middle_right), align::top_center);
  EXPECT_EQ(rotated_counter_clockwise(align::bottom_left), align::bottom_right);
  EXPECT_EQ(rotated_counter_clockwise(align::bottom_center), align::middle_right);
  EXPECT_EQ(rotated_counter_clockwise(align::bottom_right), align::top_right);

  EXPECT_EQ(flipped_vertically(align::top_left), align::bottom_left);
  EXPECT_EQ(flipped_vertically(align::top_center), align::bottom_center);
  EXPECT_EQ(flipped_vertically(align::top_right), align::bottom_right);
  EXPECT_EQ(flipped_vertically(align::middle_left), align::middle_left);
  EXPECT_EQ(flipped_vertically(align::center), align::center);
  EXPECT_EQ(flipped_vertically(align::middle_right), align::middle_right);
  EXPECT_EQ(flipped_vertically(align::bottom_left), align::top_left);
  EXPECT_EQ(flipped_vertically(align::bottom_center), align::top_center);
  EXPECT_EQ(flipped_vertically(align::bottom_right), align::top_right);

  EXPECT_EQ(flipped_horizontally(align::top_left), align::top_right);
  EXPECT_EQ(flipped_horizontally(align::top_center), align::top_center);
  EXPECT_EQ(flipped_horizontally(align::top_right), align::top_left);
  EXPECT_EQ(flipped_horizontally(align::middle_left), align::middle_right);
  EXPECT_EQ(flipped_horizontally(align::center), align::center);
  EXPECT_EQ(flipped_horizontally(align::middle_right), align::middle_left);
  EXPECT_EQ(flipped_horizontally(align::bottom_left), align::bottom_right);
  EXPECT_EQ(flipped_horizontally(align::bottom_center), align::bottom_center);
  EXPECT_EQ(flipped_horizontally(align::bottom_right), align::bottom_left);

  EXPECT_EQ(to_vertical(horizontal_align::left), vertical_align::top);
  EXPECT_EQ(to_vertical(horizontal_align::center), vertical_align::middle);
  EXPECT_EQ(to_vertical(horizontal_align::right), vertical_align::bottom);
  EXPECT_EQ(horizontal_align::left, to_horizontal(vertical_align::top));
  EXPECT_EQ(horizontal_align::center, to_horizontal(vertical_align::middle));
  EXPECT_EQ(horizontal_align::right, to_horizontal(vertical_align::bottom));

  EXPECT_EQ(to_vertical(vertical_align::top), vertical_align::top);
  EXPECT_EQ(to_vertical(vertical_align::middle), vertical_align::middle);
  EXPECT_EQ(to_vertical(vertical_align::bottom),  vertical_align::bottom);
  EXPECT_EQ(horizontal_align::left, to_horizontal(horizontal_align::left));
  EXPECT_EQ(horizontal_align::center, to_horizontal(horizontal_align::center));
  EXPECT_EQ(horizontal_align::right, to_horizontal(horizontal_align::right));

  EXPECT_EQ(horizontal_from(align::top_center), horizontal_align::center);
  EXPECT_EQ(horizontal_from(align::middle_center), horizontal_align::center);
  EXPECT_EQ(horizontal_from(align::bottom_center), horizontal_align::center);
  EXPECT_EQ(horizontal_from(align::top_left), horizontal_align::left);
  EXPECT_EQ(horizontal_from(align::middle_left), horizontal_align::left);
  EXPECT_EQ(horizontal_from(align::bottom_left), horizontal_align::left);
  EXPECT_EQ(horizontal_from(align::top_right), horizontal_align::right);
  EXPECT_EQ(horizontal_from(align::middle_right), horizontal_align::right);
  EXPECT_EQ(horizontal_from(align::bottom_right), horizontal_align::right);

  EXPECT_EQ(vertical_from(align::top_center), vertical_align::top);
  EXPECT_EQ(vertical_from(align::middle_center), vertical_align::middle);
  EXPECT_EQ(vertical_from(align::bottom_center), vertical_align::bottom);
  EXPECT_EQ(vertical_from(align::top_left), vertical_align::top);
  EXPECT_EQ(vertical_from(align::middle_left), vertical_align::middle);
  EXPECT_EQ(vertical_from(align::bottom_left), vertical_align::bottom);
  EXPECT_EQ(vertical_from(align::top_right), vertical_align::top);
  EXPECT_EQ(vertical_from(align::middle_right), vertical_align::middle);
  EXPECT_EQ(vertical_from(align::bottom_right), vertical_align::bottom);

  EXPECT_EQ(only_horizontal(align::top_center), align::top_center);
  EXPECT_EQ(only_horizontal(align::middle_center), align::top_center);
  EXPECT_EQ(only_horizontal(align::bottom_center), align::top_center);
  EXPECT_EQ(only_horizontal(align::top_left), align::top_left);
  EXPECT_EQ(only_horizontal(align::middle_left), align::top_left);
  EXPECT_EQ(only_horizontal(align::bottom_left), align::top_left);
  EXPECT_EQ(only_horizontal(align::top_right), align::top_right);
  EXPECT_EQ(only_horizontal(align::middle_right), align::top_right);
  EXPECT_EQ(only_horizontal(align::bottom_right), align::top_right);

  EXPECT_EQ(only_vertical(align::top_center), align::top_left);
  EXPECT_EQ(only_vertical(align::middle_center), align::middle_left);
  EXPECT_EQ(only_vertical(align::bottom_center), align::bottom_left);
  EXPECT_EQ(only_vertical(align::top_left), align::top_left);
  EXPECT_EQ(only_vertical(align::middle_left), align::middle_left);
  EXPECT_EQ(only_vertical(align::bottom_left), align::bottom_left);
  EXPECT_EQ(only_vertical(align::top_right), align::top_left);
  EXPECT_EQ(only_vertical(align::middle_right), align::middle_left);
  EXPECT_EQ(only_vertical(align::bottom_right), align::bottom_left);
}

TEST(alignment_test, axis_alignments_work)
{
  EXPECT_EQ(aligned(10, 100, horizontal_align::left), 0);
  EXPECT_EQ(aligned(10.0, 100.0, horizontal_align::center), 45.0);
  EXPECT_EQ(aligned(10.0f, 100.0f, horizontal_align::right), 90.0f);

  EXPECT_EQ(aligned(10, 100, vertical_align::top), 0);
  EXPECT_EQ(aligned(10, 100, vertical_align::middle), 45);
  EXPECT_EQ(aligned(10, 100, vertical_align::bottom), 90);

  EXPECT_EQ(aligned(0, 0, horizontal_align::left), 0);
  EXPECT_EQ(aligned(0, 0, horizontal_align::center), 0);
  EXPECT_EQ(aligned(0, 0, horizontal_align::right), 0);

  EXPECT_EQ(aligned(100.0, 100.0, horizontal_align::left), 0.0);
  EXPECT_EQ(aligned(100.0, 100.0, horizontal_align::center), 0.0);
  EXPECT_EQ(aligned(100.0, 100.0, horizontal_align::right), 0.0);

  EXPECT_EQ(aligned(10.0f, INFINITY, horizontal_align::left), 0.0f);
  EXPECT_EQ(aligned(10.0f, INFINITY, horizontal_align::center), INFINITY);
  EXPECT_EQ(aligned(10.0f, INFINITY, horizontal_align::right), INFINITY);

  EXPECT_EQ(aligned(100.0, 10.0, horizontal_align::left), 0.0);
  EXPECT_EQ(aligned(100.0, 10.0, horizontal_align::center), -45.0);
  EXPECT_EQ(aligned(100.0, 10.0, horizontal_align::right), -90.0);

  EXPECT_EQ(aligned(INFINITY, 10.0f, horizontal_align::left), 0.0f);
  EXPECT_EQ(aligned(INFINITY, 10.0f, horizontal_align::center), -INFINITY);
  EXPECT_EQ(aligned(INFINITY, 10.0f, horizontal_align::right), -INFINITY);
}

TEST(alignment_test, justify_doesnt_break_anything)
{
  EXPECT_FALSE(horizontal_aligns.contains(horizontal_align::justify));
  EXPECT_FALSE(vertical_aligns.contains(vertical_align::justify));

  EXPECT_FALSE(all_aligns.contains(horizontal_align::justify | vertical_align::top));
  EXPECT_FALSE(all_aligns.contains(horizontal_align::justify | vertical_align::middle));
  EXPECT_FALSE(all_aligns.contains(horizontal_align::justify | vertical_align::bottom));

  EXPECT_FALSE(all_aligns.contains(vertical_align::justify | horizontal_align::left));
  EXPECT_FALSE(all_aligns.contains(vertical_align::justify | horizontal_align::center));
  EXPECT_FALSE(all_aligns.contains(vertical_align::justify | horizontal_align::right));

  EXPECT_EQ(to_horizontal(vertical_align::justify), horizontal_align::justify);
  EXPECT_EQ(to_vertical(horizontal_align::justify), vertical_align::justify);

  EXPECT_EQ(only_horizontal(vertical_align::justify | horizontal_align::left), align::top_left);
  EXPECT_EQ(only_horizontal(vertical_align::justify | horizontal_align::center), align::top_center);
  EXPECT_EQ(only_horizontal(vertical_align::justify | horizontal_align::right), align::top_right);

  EXPECT_EQ(vertical_from(vertical_align::justify | horizontal_align::justify), vertical_align::justify);
  EXPECT_EQ(horizontal_from(vertical_align::justify | horizontal_align::justify), horizontal_align::justify);

  EXPECT_EQ(only_vertical(horizontal_align::justify | vertical_align::top), align::top_left);
  EXPECT_EQ(only_vertical(horizontal_align::justify | vertical_align::middle), align::middle_left);
  EXPECT_EQ(only_vertical(horizontal_align::justify | vertical_align::bottom), align::bottom_left);

  EXPECT_EQ(to_opposite(horizontal_align::justify), horizontal_align::justify);
  EXPECT_EQ(to_opposite(vertical_align::justify), vertical_align::justify);

  /// This part is based on implementation details
  EXPECT_EQ(int(horizontal_align::justify) & int(vertical_align::justify), 0);
}