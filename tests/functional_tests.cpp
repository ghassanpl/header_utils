/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/functional.h"
#include "tests_common.h"

#include <gtest/gtest.h>

using namespace ghassanpl;

TEST(multicast_function, works)
{
	mutlticast_function<int(int)> delegate;

	int called = 0;
	bool called_a{}, called_b{}, called_c{};
	auto handle_a = delegate += [&](int a) { called_a = true; called++; return a; };
	auto handle_b = delegate += [&](int a) { called_b = true; called++; return a * 2; };

	auto result = delegate(10);

	EXPECT_EQ(result, (std::vector{ 10,20 }));
	EXPECT_EQ(called, 2);
	EXPECT_TRUE(called_a);
	EXPECT_TRUE(called_b);
	EXPECT_FALSE(called_c);

	called = 0, called_a = false, called_b = false, called_c = false;

	delegate.remove(handle_a);

	result = delegate(20);

	EXPECT_EQ(result, (std::vector{ 40 }));
	EXPECT_EQ(called, 1);
	EXPECT_FALSE(called_a);
	EXPECT_TRUE(called_b);
	EXPECT_FALSE(called_c);

	called = 0, called_a = false, called_b = false, called_c = false;

	auto handle_c = delegate += [&](int a) { called_c = true; called++; return a * 3; };

	result = delegate(50);

	EXPECT_EQ(result, (std::vector{ 100, 150 }));
	EXPECT_EQ(called, 2);
	EXPECT_FALSE(called_a);
	EXPECT_TRUE(called_b);
	EXPECT_TRUE(called_c);

	delegate.clear();

	called = 0, called_a = false, called_b = false, called_c = false;

	result = delegate(40);

	EXPECT_EQ(result, (std::vector<int>{}));
	EXPECT_EQ(called, 0);
	EXPECT_FALSE(called_a);
	EXPECT_FALSE(called_b);
	EXPECT_FALSE(called_c);
}

TEST(multicast_function, doesnt_break_references)
{
	{
		mutlticast_function<void(UnCopyable const&)> delegate;
		delegate += [](UnCopyable const& u) {};
		delegate(uncopyable);
		delegate(UnCopyable{});
	}

	{
		mutlticast_function<void(UnMovable const&)> delegate;
		delegate += [](UnMovable const& u) {};
		delegate(unmovable);
		delegate(UnMovable{});
	}
}

TEST(make_single_time_function, works)
{
	int called = 0;
	auto f = make_single_time_function([&]() { called++; });
	std::function<void()> meh = [&]() { called++; };
	auto f2 = make_single_time_function(meh);

	for (int i = 0; i < 10; ++i)
	{
		f();
		EXPECT_EQ(called, 1);
	}

	for (int i = 0; i < 10; ++i)
	{
		f2();
		EXPECT_EQ(called, 2);
	}
}

TEST(optional_transform, works)
{
	std::optional<int> i = 50;
	auto res = transform(i, [](int i) { return std::to_string(i); });
	EXPECT_EQ(res, "50");
	auto ores = transform(i, [](int i) { return std::optional{ std::to_string(i) }; });
	EXPECT_TRUE(ores.has_value());
	EXPECT_EQ(ores.value(), "50");
	std::optional<int> j = std::nullopt;
	auto nres = transform(j, [](int i) { return std::to_string(i); });
	EXPECT_FALSE(nres.has_value());

	auto res2 = i.transform([](int i) { return std::to_string(i); });
	EXPECT_EQ(res2, "50");
}
