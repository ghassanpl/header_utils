/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/multicast.h"
#include "../include/ghassanpl/functional.h"
#include "tests_common.h"
#include <print>
#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_literals;

TEST(multicast_function, works)
{
	mutlticast_function<int(int)> delegate;

	int called = 0;
	bool called_a{}, called_b{}, called_c{};
	auto handle_a = delegate += [&](int a) { called_a = true; called++; return a; };
	auto handle_b = delegate += [&](int a) { called_b = true; called++; return a * 2; };
	(void)handle_b;

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
	(void)handle_c;

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
	auto res = transformed(i, [](int i) { return std::to_string(i); });
	EXPECT_EQ(res, "50");
	auto ores = transformed(i, [](int i) { return std::optional{ std::to_string(i) }; });
	EXPECT_TRUE(ores.has_value());
	EXPECT_EQ(ores.value(), "50");
	std::optional<int> j = std::nullopt;
	auto nres = transformed(j, [](int i) { return std::to_string(i); });
	EXPECT_FALSE(nres.has_value());

	auto res2 = i.transform([](int i) { return std::to_string(i); });
	EXPECT_EQ(res2, "50");
}

TEST(predicates, work)
{

}

template <typename... ARGS>
constexpr bool resulting_works()
{
	return requires () { ghassanpl::resulting([](ARGS...) {}); };
}

template <typename... ARGS>
constexpr bool resulting_call_auto()
{
	return requires () { ghassanpl::resulting([](auto&, ARGS...) {}); };
}


TEST(variant_transform, works)
{
	{
		std::variant<int, std::string> v = 50;
		auto res = transformed(v, overloaded{
			[](int i) { return std::to_string(i); },
			[](std::string const& s) { return s; }
		});
		static_assert(std::is_same_v<decltype(res), std::variant<std::string>>);
		EXPECT_EQ(res, std::variant<std::string>{ "50"s });
	}
	{
		auto res = transformed(std::variant<int>{50}, overloaded{
			[](int i) { return std::to_string(i); },
			[](std::string const& s) { return s; },
		});
		static_assert(std::is_same_v<decltype(res), std::variant<std::string>>);
		EXPECT_EQ(res, std::variant<std::string>{ "50"s });
	}
	{
		auto res = transformed(std::variant<int, std::string>{50}, overloaded{
			[](int i) { return std::to_string(i); },
			[](std::string const& s) { return s; },
			});
		static_assert(std::is_same_v<decltype(res), std::variant<std::string>>);
		EXPECT_EQ(res, std::variant<std::string>{ "50"s });
	}
	{
		auto res = transformed(std::variant<int, std::string>{50}, overloaded{
			[](int i) { return i; },
			[](std::string const& s) { return s; },
		});
		static_assert(std::is_same_v<decltype(res), std::variant<int, std::string>>);
		EXPECT_EQ(res, (std::variant<int, std::string>(50)));
	}
	{
		auto res = transformed(std::variant<int>{50}, overloaded{
			[](int i) { return i; },
			[](std::string const& s) { return s; },
			[](std::tuple<int, double> s) { return s; },
		});
		static_assert(std::is_same_v<decltype(res), std::variant<int>>);
		EXPECT_EQ(res, std::variant<int>{ 50 });
	}

	std::variant<int> v1 = convertible_to_variant(std::variant<int>{ 50 });
	EXPECT_EQ(v1, std::variant<int>{ 50 });
	std::variant<int, std::string> v2 = convertible_to_variant(std::variant<int>{ 50 });
	EXPECT_EQ(v2, (std::variant<int, std::string>{ 50 }));
	std::variant<int, std::string> v3 = convertible_to_variant(std::variant<std::string, int>{ 50 });
	EXPECT_EQ(v3, (std::variant<int, std::string>{ 50 }));

	constexpr auto b1 = requires (std::variant<int> v1) {
		{ convertible_to_variant(std::variant<int>{ 50 }) } -> std::convertible_to<decltype(v1)>;
	};
	constexpr auto b4 = requires (std::variant<int> v4) {
		{ convertible_to_variant(std::variant<std::string, int>{ 50 }) } -> std::convertible_to<decltype(v4)>;
	};
	constexpr auto b5 = requires (std::variant<int> v5) {
		{ convertible_to_variant(std::variant<std::string>{ "asd"s }) } -> std::convertible_to<decltype(v5)>;
	};
	EXPECT_TRUE(b1);
	EXPECT_FALSE(b4);
	EXPECT_FALSE(b5);

	using str = detail::variant_flat_t<std::string, std::string>;
	using str2 = detail::variant_flat_t<std::string>;

#ifndef __clang__
	{
		enum class NegativeInt { };
		enum class PositiveInt { };
		auto res = ghassanpl::transformed_flattened(std::variant<int, std::string>{2}, overloaded{
			[](int x) -> std::variant<NegativeInt, PositiveInt> {
				if (x < 0)
					return NegativeInt{x};
				else
					return PositiveInt{x};
			},
			xf::identity_l
			});
		static_assert(std::is_same_v<decltype(res), std::variant<NegativeInt, PositiveInt, std::string>>);
		EXPECT_TRUE(std::holds_alternative<PositiveInt>(res));
		EXPECT_EQ(std::get<PositiveInt>(res), PositiveInt{ 2 });
	}

	{
		enum class NegativeInt { };
		enum class PositiveInt { };
		auto res = transformed_flattened(std::variant<int, std::string>{-2}, overloaded{
			[](int x) -> std::variant<NegativeInt, PositiveInt> {
				if (x < 0)
					return NegativeInt{x};
				else
					return PositiveInt{x};
			},
			xf::identity_l
		});
		static_assert(std::is_same_v<decltype(res), std::variant<NegativeInt, PositiveInt, std::string>>);
		EXPECT_TRUE(std::holds_alternative<NegativeInt>(res));
		EXPECT_EQ(std::get<NegativeInt>(res), NegativeInt{ -2 });
	}
	{
		enum class NegativeInt { };
		enum class PositiveInt { };
		auto res = transformed_flattened(std::variant<int, std::string>{"lol"s}, overloaded{
			[](int x) -> std::variant<NegativeInt, PositiveInt> {
				if (x < 0)
					return NegativeInt{x};
				else
					return PositiveInt{x};
			},
			xf::identity_l
		});
		static_assert(std::is_same_v<decltype(res), std::variant<NegativeInt, PositiveInt, std::string>>);
		EXPECT_TRUE(std::holds_alternative<std::string>(res));
		EXPECT_EQ(std::get<std::string>(res), "lol");
	}
#endif
}


TEST(op_functions, work)
{
	static_assert(resulting_works<int&>());
	static_assert(!resulting_works<int&, int>());
	static_assert(!resulting_works<int const&>());
	static_assert(!resulting_works<int>());
	static_assert(!resulting_works<int const>());
	static_assert(!resulting_works<>());
	static_assert(!resulting_call_auto());
}
