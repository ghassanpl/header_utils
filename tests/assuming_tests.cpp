/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef __clang__

#define ASSUMING_TESTS
#define ASSUMING_INCLUDE_MAGIC_ENUM 1
#define ASSUMING_DEBUG 1
#include "../include/ghassanpl/assuming.h"
#include "tests_common.h"

#include <gtest/gtest.h>
#include <format>
#include <thread>

struct assuming_test;
assuming_test* current_test = nullptr;

enum class test_enum { asadf, bcvbvc, czxcv };

using namespace ghassanpl;

using name_value_pair = std::pair<std::string_view, std::string>;

struct assuming_test : public ::testing::Test
{
	bool assumption_failed = false;
	source_location last_where;
	std::string last_expectation;
	std::vector<name_value_pair> last_values;
	std::string last_data;

	size_t evaluation_count = 0;

	template <typename T>
	auto single_eval_check(T&& v)
	{
		evaluation_count++;
		return std::forward<T>(v);
	}

	void ReportAssumptionFailure(std::string_view expectation, std::initializer_list<name_value_pair> values, std::string data, source_location where)
	{
		assumption_failed = true;
		last_where = where;
		last_expectation = expectation;
		//fmt::print("Failed assumption that {}\n", expectation);
		last_values = { values.begin(), values.end() };
		last_data = std::move(data);
	}

	template <typename T, typename U>
	static inline bool compare(T const& a, U const& b)
	{
		return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b));
	}

	void SetUp() override
	{
		current_test = this;

		AssumptionFailureHandler = [](std::string_view expectation, std::initializer_list<name_value_pair> values, std::string data, source_location where
#if ASSUMING_USE_STACKTRACE
			, std::stacktrace stacktrace
#endif
			)
		{
			current_test->ReportAssumptionFailure(expectation, std::move(values), std::move(data), where);
		};
	}

	void TearDown() override
	{
		current_test = nullptr;
		AssumptionFailureHandler = nullptr;
	}
};

#define SELF(...) (__VA_ARGS__)

#define ORCOMMA(...) __VA_ARGS__ __VA_OPT__(,)

#if ASSUMING_DEBUG
#define EXPECT_ASSUMPTION_FAILED(func_name, ...) {\
	assumption_failed = false; \
	func_name (__VA_ARGS__ __VA_OPT__(,) "test({}, {})", 0, 5) ; auto current_location = source_location::current(); \
	EXPECT_TRUE(assumption_failed) << #func_name; \
	EXPECT_EQ(last_where.line(), current_location.line()); \
	EXPECT_STREQ(last_where.file_name(), current_location.file_name()); \
	EXPECT_STREQ(last_where.function_name(), current_location.function_name()); \
	EXPECT_EQ("test(0, 5)", last_data); }
#else
#define EXPECT_ASSUMPTION_FAILED(func_name, ...) \
	func_name SELF(__VA_ARGS__, "test({}, {})", 0, 5);
#endif
#define EXPECT_ASSUMPTION_SUCCEEDED(func_name, ...) { assumption_failed = false; func_name(__VA_ARGS__); EXPECT_FALSE(assumption_failed) << #func_name; }

TEST_F(assuming_test, Assuming_works)
{
	EXPECT_ASSUMPTION_SUCCEEDED(Assuming, true);

	const bool value = false;
	EXPECT_ASSUMPTION_FAILED(Assuming, value);

	name_value_pair const values[] = { {"value", "false"} };
	ASSERT_TRUE(compare(values, last_values));
}

int object{};

TEST_F(assuming_test, AssumingNotNull_works)
{
	EXPECT_ASSUMPTION_SUCCEEDED(AssumingNotNull, &object);

	const decltype(&object) value = nullptr;
	EXPECT_ASSUMPTION_FAILED(AssumingNotNull, value);

	name_value_pair const values[] = { {"value", "0x0"} };
	ASSERT_TRUE(compare(values, last_values));
}

TEST_F(assuming_test, AssumingNull_works)
{
	const decltype(&object) value = nullptr;
	EXPECT_ASSUMPTION_SUCCEEDED(AssumingNull, value);

	EXPECT_ASSUMPTION_FAILED(AssumingNull, &object);

	name_value_pair const values[] = { {"&object", std::format("{}", (const void*)&object)} };
	ASSERT_TRUE(compare(values, last_values));
}

TEST_F(assuming_test, AssumingEqual_works)
{
	std::pair<int, double> q = { 5, 6 };
	EXPECT_ASSUMPTION_FAILED(AssumingEqual, q.first, q.second);

	name_value_pair const values[] = { {"q.first", "5"}, {"q.second", "6"} };
	ASSERT_TRUE(compare(values, last_values));

	const double value = 0.4;
	EXPECT_ASSUMPTION_SUCCEEDED(AssumingEqual, value, 0.4);
}

TEST_F(assuming_test, AssumingNotEqual_works)
{
	const auto enum_val = test_enum::asadf;
	EXPECT_ASSUMPTION_SUCCEEDED(AssumingNotEqual, enum_val, test_enum::bcvbvc);
	EXPECT_ASSUMPTION_FAILED(AssumingNotEqual, enum_val, enum_val);

	const std::string value = "hello";
	EXPECT_ASSUMPTION_FAILED(AssumingNotEqual, value, "hello");

	name_value_pair const values[] = { {"value", "hello"}, {"\"hello\"", "hello"} };
	ASSERT_TRUE(compare(values, last_values));
}

TEST_F(assuming_test, AssumingNotReachable_works)
{
	/// TODO: The below will not work because AssumingNotReachable calls std::unreachable
	//EXPECT_ASSUMPTION_FAILED(AssumingNotReachable);
}

TEST_F(assuming_test, AssumingNotRecursive_works)
{
	auto func = [](this auto&& func, int times) {
		if (times <= 0)
			return;

		AssumingNotRecursive();
		func(times - 1);
	};

	assumption_failed = false;
	func(1);
	EXPECT_FALSE(assumption_failed);
	func(2);
	EXPECT_TRUE(assumption_failed);
}

TEST_F(assuming_test, AssumingSingleThread_works)
{
	assumption_failed = false;

	auto func = []() {
		AssumingSingleThread();
	};

	EXPECT_FALSE(assumption_failed);

	func();

	EXPECT_FALSE(assumption_failed);

	func();

	EXPECT_FALSE(assumption_failed);

	std::thread thread{ func };
	thread.join();

	EXPECT_TRUE(assumption_failed);
}

#define EXPECT_EVAL_COUNT(count, ...) __VA_ARGS__; EXPECT_EQ(std::exchange(evaluation_count, 0), count);
TEST_F(assuming_test, assumings_evaluate_arguments_only_once)
{
	EXPECT_EVAL_COUNT(1, Assuming(single_eval_check(false)));

	EXPECT_EVAL_COUNT(1, AssumingNotNull(single_eval_check(decltype(&object)(nullptr))));
	EXPECT_EVAL_COUNT(1, AssumingNull(single_eval_check(&object)));

	EXPECT_EVAL_COUNT(2, AssumingEqual(single_eval_check(1), single_eval_check(2)));
	EXPECT_EVAL_COUNT(2, AssumingNotEqual(single_eval_check(1), single_eval_check(1)));
	EXPECT_EVAL_COUNT(2, AssumingGreater(single_eval_check(1), single_eval_check(1)));
	EXPECT_EVAL_COUNT(2, AssumingLess(single_eval_check(1), single_eval_check(1)));
	EXPECT_EVAL_COUNT(2, AssumingGreaterEqual(single_eval_check(1), single_eval_check(2)));
	EXPECT_EVAL_COUNT(2, AssumingLessEqual(single_eval_check(1), single_eval_check(0)));

	EXPECT_EVAL_COUNT(1, AssumingEmpty(single_eval_check(std::string{ "hello" })));
	EXPECT_EVAL_COUNT(1, AssumingNotEmpty(single_eval_check(std::string{})));

	EXPECT_EVAL_COUNT(1, AssumingNullOrEmpty(single_eval_check("hello")));
	const char* null_string = nullptr;
	std::string empty_string{};
	EXPECT_EVAL_COUNT(1, AssumingNotNullOrEmpty(single_eval_check(null_string)));

	EXPECT_EVAL_COUNT(2, AssumingValidIndex(single_eval_check(1), single_eval_check(empty_string)));
}

template <class CharT>
struct std::formatter<UnCopyable, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
	template<class FormatContext>
	auto format(UnCopyable const& t, FormatContext& fc) const {
		return std::formatter<std::basic_string<CharT>, CharT>::format("UnCopyable", fc);
	}
};
template <class CharT>
struct std::formatter<UnMovable, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
	template<class FormatContext>
	auto format(UnMovable const& t, FormatContext& fc) const {
		return std::formatter<std::basic_string<CharT>, CharT>::format("UnMovable", fc);
	}
};

TEST_F(assuming_test, assumings_dont_copy_unnecessarily)
{
	/// TODO: Fix formatting these
	//int i = 0;
	UnCopyable a;
	EXPECT_ASSUMPTION_SUCCEEDED(AssumingEqual, a, UnCopyable{});
	UnMovable b;
	EXPECT_ASSUMPTION_SUCCEEDED(AssumingEqual, b, UnMovable{});
}

extern "C" struct unknown {};
TEST(assuming_tests, AdditionalDataToString_works)
{
	EXPECT_EQ(detail::AdditionalDataToString("{}", "hello"), "hello");
	EXPECT_EQ(detail::AdditionalDataToString("{}", 5), "5");
	EXPECT_EQ(detail::AdditionalDataToString("{}", 5.5), "5.5");
	EXPECT_EQ(detail::AdditionalDataToString("{}", test_enum::asadf), "asadf");
	EXPECT_EQ(detail::AdditionalDataToString("{}", (int*)0), "0x0");
	EXPECT_TRUE(detail::AdditionalDataToString("{}", unknown{}).contains("unknown"));
}

/// TODO: Make sure everything works when ASSUMING_DEBUG is not defined

#endif