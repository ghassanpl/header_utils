#include "../include/ghassanpl/align+rec2.h"
#include "../include/ghassanpl/assuming.h"
#include "../include/ghassanpl/enum_flags.h"
#include "../include/ghassanpl/atomic_enum_flags.h"
#include "../include/ghassanpl/di.h"
#include "../include/ghassanpl/named.h"
#include "../include/ghassanpl/rec2.h"
#include "../include/ghassanpl/string_ops.h"

#include <set>
#include <algorithm>
#include <numeric>
#include <experimental/generator>
#include <ranges>

#include <gtest/gtest.h>

using namespace ghassanpl;

template <typename RANGE1, typename RANGE2>
auto cartesian_product(RANGE1&& r1, RANGE2&& r2) -> std::experimental::generator<std::pair<std::ranges::range_value_t<RANGE1>, std::ranges::range_value_t<RANGE2>>>
{
  for (auto& e1 : r1)
    for (auto& e2 : r2)
      co_yield std::pair{ e1, e2 };
}

TEST(align, basics_work)
{
  align a{ align::left_top };
  EXPECT_EQ(horizontal_align::left | vertical_align::bottom, a |= vertical_align::bottom);

  std::set<vertical_align> vertical_aligns{
    vertical_align::top, vertical_align::bottom, vertical_align::middle
  };
  ASSERT_EQ(vertical_aligns.size(), 3);

  std::set<horizontal_align> horizontal_aligns{
    horizontal_align::left, horizontal_align::center, horizontal_align::right
  };
  ASSERT_EQ(vertical_aligns.size(), 3);

  std::set<align> all_aligns{
    align::left_top, align::left_middle, align::left_bottom,
    align::center_top, align::center_middle, align::center_bottom,
    align::right_top, align::right_middle, align::right_bottom,
  };
  ASSERT_EQ(vertical_aligns.size(), 3);

  auto range = cartesian_product(vertical_aligns, horizontal_aligns);
  auto other_range = range | std::ranges::views::transform([](auto p) { return p.first | p.second; });
  ASSERT_EQ(all_aligns, (std::set<align> { other_range.begin(), other_range.end() }));
}


int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}