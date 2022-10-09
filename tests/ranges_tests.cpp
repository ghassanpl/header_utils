/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>
using std::operator""s;

#include "../include/ghassanpl/ranges.h"

using namespace ghassanpl;

TEST(ranges_test, to_works)
{
	std::map<std::string, int64_t> vals = { {"hello", 64}, {"yo", 32}, {"motehrfuck", 12} };
	const auto vec = to<std::set>(std::views::keys(vals));
	EXPECT_EQ(vec, std::set({ "hello"s, "yo"s, "motehrfuck"s }));

	auto result = to<std::map>(vals | std::views::take(2));
	EXPECT_EQ(result.size(), 2);
	EXPECT_EQ(result["hello"], 64);
	EXPECT_FALSE(result.contains("yo"));
}

/*
TEST(ranges_test, fold_works)
{
	std::vector<int> nums{ 10,20,30,40 };
	EXPECT_EQ(fold(nums, 0, std::plus<int>{}), 100);
}
*/