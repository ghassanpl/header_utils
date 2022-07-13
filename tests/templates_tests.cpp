/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/templates.h"
#include "../include/ghassanpl/string_ops.h"
#include "tests_common.h"
#include "test_system.h"

using ghassanpl::string_ops::to_string;
using std::to_string;

#include <gtest/gtest.h>
#include <format>
#include <source_location>


TEST(templates_test, enumerate_pack_works)
{
	int test = 0;
	ghassanpl::enumerate_pack([&](size_t i, auto&& a) {
		test++;
		if constexpr (std::is_same_v<decltype(a), int&>)
			a++;
		return 0;
	}, 10, 20, "hello", test);

	EXPECT_EQ(test, 5);

	std::vector<size_t> in_order;
	std::vector<std::string> values_in_order;
	ghassanpl::enumerate_pack([&](size_t i, auto&& a) {
		in_order.push_back(i);
		values_in_order.push_back(std::format("{}", a));
	}, 10, 20, "hello", test);

	EXPECT_EQ(in_order, (std::vector<size_t>{0, 1, 2, 3}));
	EXPECT_EQ(values_in_order, (std::vector<std::string>{"10", "20", "hello", "5"}));

	bool called_with_empty_pack = false;
	ghassanpl::enumerate_pack([&](size_t i, auto a) { called_with_empty_pack = true; });
	EXPECT_FALSE(called_with_empty_pack);

	bool called = false;
	ghassanpl::enumerate_pack([&](size_t i, auto&& a) { called = true; }, uncopyable);
	EXPECT_TRUE(called);
	called = false;
	ghassanpl::enumerate_pack([&](size_t i, auto&& a) { called = true; }, unmovable);
	EXPECT_TRUE(called);
}

/*
* 
TEST(templates_test, enumerate_pack_works)
{
	int test = 0;
	ghassanpl::enumerate_pack([&](size_t i, auto&& a) {
		test++;
		if constexpr (std::is_same_v<decltype(a), int&>)
			a++;
	}, 10, 20, "hello", test);

	EXPECT_EQ(test, 5);

	std::vector<size_t> in_order;
	std::vector<std::string> values_in_order;
	ghassanpl::enumerate_pack([&](size_t i, auto&& a) {
		in_order.push_back(i);
		values_in_order.push_back(std::format("{}", a));
	}, 10, 20, "hello", test);
*/

constexpr auto lambda = [](auto&& a) {
	if constexpr (std::is_same_v<decltype(a), int&>)
		return -1;
	else
		return (int)sizeof(a);
};

template <size_t I>
constexpr bool can_apply_to_nth = requires (decltype(lambda) lambda, int& test) {
	{ ghassanpl::apply_to_nth<I>(lambda, 10, 20, "hello", test) };
};

template <size_t I, typename FUNC, typename... ARGS, size_t... Idxs>
constexpr auto atn2(FUNC& f, std::index_sequence<Idxs...>, ARGS&&... args)
{
	return ([&] {
		static_assert(std::is_invocable_v<FUNC, ARGS>, "Cannot invoke callback with this type");
		if constexpr (Idxs == I)
			return f(std::forward<ARGS>(args));
		else
			return ghassanpl::detail::pass_identity{};
	}() * ...);
}

template <size_t I, typename FUNC, typename... ARGS>
requires (I < sizeof...(ARGS))
constexpr auto apply_to_nth(FUNC&& f, ARGS&&... args)
{
	return atn2<I>(f, std::make_index_sequence<sizeof...(args)>{}, std::forward<ARGS>(args)...);
}

TEST(templates_test, apply_to_nth_works)
{
	int test = 0;
	auto result = apply_to_nth<0>(lambda, 10, 20, "hello", test);
	EXPECT_EQ(result, (int)sizeof(10));
	//EXPECT_EQ(ghassanpl::apply_to_nth<1>(lambda, 10, 20, "hello", test), (int)sizeof(20));
	//EXPECT_EQ(ghassanpl::apply_to_nth<2>(lambda, 10, 20, "hello", test), (int)sizeof("hello"));
	//EXPECT_EQ(ghassanpl::apply_to_nth<3>(lambda, 10, 20, "hello", test), -1);
	
	EXPECT_TRUE(can_apply_to_nth<0>);
	EXPECT_TRUE(can_apply_to_nth<1>);
	EXPECT_TRUE(can_apply_to_nth<2>);
	EXPECT_TRUE(can_apply_to_nth<3>);
	EXPECT_FALSE(can_apply_to_nth<4>);
	EXPECT_FALSE(can_apply_to_nth<54>);
	EXPECT_FALSE(can_apply_to_nth<-1>);

	bool called = false;
	ghassanpl::apply_to_nth<0>([&](auto&& a) { called = true; }, UnCopyable{});
	EXPECT_TRUE(called);
	called = false;
	ghassanpl::apply_to_nth<0>([&](auto&& a) { called = true; }, unmovable);
	EXPECT_TRUE(called);
}

/*
TEST(templates_test, pack_slice_works)
{
	auto sum_pack = []<typename... ARGS>(ARGS&&... p) { std::string result; ((result += to_string(std::forward<ARGS>(p))), ...); return result; };

	EXPECT_EQ((ghassanpl::apply_to_slice<3>(sum_pack, 1, "hello", 3.14, "World", 42, "!")), "World42!");

	EXPECT_EQ((ghassanpl::apply_to_slice<1, 6, 2>(sum_pack, 1, "hello", 3.14, "World", 42, "!")), "helloWorld!");
	
	EXPECT_EQ((ghassanpl::apply_to_slice<0, 3>(sum_pack, 1, "hello", 3.14, "World", 42, "!", UnCopyable{}, unmovable)), "1hello3.14");

	EXPECT_EQ((ghassanpl::apply_to_slice<0, 0>(sum_pack, 1, "hello", 3.14, "World", 42, "!")), "");
}
*/