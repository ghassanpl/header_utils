/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "tests_common.h"
#include <string>
using std::operator""s;
using std::operator""sv;

#include "../include/ghassanpl/ranges.h"
#include "../include/ghassanpl/string_ops.h"

using namespace ghassanpl;
using ghassanpl::string_ops::make_sv;

TEST(ranges_test, to_works)
{
	std::map<std::string, int64_t, std::less<>> vals = { {"hello", 64}, {"yo", 32}, {"motehrfuck", 12} };
	const auto vec = to<std::set>(std::views::keys(vals));
	EXPECT_EQ(vec, std::set({ "hello"s, "yo"s, "motehrfuck"s }));

	auto result = to<std::map>(vals | std::views::take(2));
	EXPECT_EQ(result.size(), 2);
	EXPECT_EQ(result["hello"], 64);
	EXPECT_FALSE(result.contains("yo"));
}

TEST(ranges_span_functions, work)
{
	using std::span;
	char hello[] = "hello world";
	auto hello_span = span{ hello };
	{
		auto [left, right] = split_at(hello_span, 5);
		EXPECT_EQ(make_sv(left), "hello");
		EXPECT_EQ(make_sv(right), " world\0"sv);
		auto [full, empty] = split_at(hello_span, 12);
		EXPECT_EQ(make_sv(full), make_sv(hello_span));
		EXPECT_TRUE(empty.empty());

		EXPECT_TRUE(are_adjacent(left, right));
		EXPECT_TRUE(are_overlapping(hello_span, left));
		EXPECT_TRUE(are_overlapping(hello_span, right));
	}

	{
		auto [left, middle, right] = split_at(hello_span, 5, 1);
		EXPECT_EQ(make_sv(left), "hello");
		EXPECT_EQ(make_sv(right), "world\0"sv);
		EXPECT_EQ(make_sv(middle), " "sv);
		auto [full, empty, empty2] = split_at(hello_span, 12, 10);
		EXPECT_EQ(make_sv(full), make_sv(hello_span));
		EXPECT_TRUE(empty.empty());
		EXPECT_TRUE(empty2.empty());

		EXPECT_TRUE(are_adjacent(left, middle));
		EXPECT_TRUE(are_adjacent(middle, right));
		EXPECT_FALSE(are_adjacent(left, right));
		EXPECT_FALSE(are_adjacent(left, left));

		EXPECT_TRUE(are_overlapping(left, left));
		EXPECT_TRUE(are_overlapping(hello_span, left));
		EXPECT_TRUE(are_overlapping(hello_span, middle));
		EXPECT_TRUE(are_overlapping(hello_span, right));
		
		EXPECT_FALSE(are_overlapping(left, right));
		EXPECT_FALSE(are_overlapping(left, middle));
		EXPECT_FALSE(are_overlapping(right, middle));
	}

	auto [left, right] = split_at(hello_span, 5);
	EXPECT_TRUE(are_adjacent(left, right));
}

TEST(ranges_array_functions, work)
{
	constexpr auto a1 = std::array{ 10, 20, 30 };
	constexpr auto a2 = std::array{ 40, 50, 60 };
	constexpr auto a3 = join(a1, a2);
	static_assert(a3 == (std::array{ 10, 20, 30, 40, 50, 60 }));

	constexpr auto a6 = join(std::array{ UnCopyable{}, UnCopyable{}, UnCopyable{} }, std::array{ UnCopyable{}, UnCopyable{}, UnCopyable{} });
	static_assert(a6.size() == 6);


	constexpr auto a7 = join(a1, 40, 50, 60);
	static_assert(a7 == (std::array{ 10, 20, 30, 40, 50, 60 }));
}
/*
TEST(ranges_test, fold_works)
{
	std::vector<int> nums{ 10,20,30,40 };
	EXPECT_EQ(fold(nums, 0, std::plus<int>{}), 100);
}
*/