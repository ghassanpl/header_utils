/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/assuming.h"

#include <gtest/gtest.h>

struct assuming_test;
assuming_test* current_test = nullptr;

using namespace ghassanpl;

using name_value_pair = std::pair<std::string_view, std::string>;

struct assuming_test : public ::testing::Test
{
  bool assumption_failed = false;
  std::source_location last_where;
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

  void ReportAssumptionFailure(std::string_view expectation, std::initializer_list<name_value_pair> values, std::string data, std::source_location where)
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
  }

  void TearDown() override
  {
    current_test = nullptr;
  }
};

#define SELF(...) (__VA_ARGS__)

#if ASSUMING_DEBUG
#define EXPECT_ASSUMPTION_FAILED(func_name, ...) \
  func_name SELF(__VA_ARGS__, "test({}, {})", 0, 5) ; auto current_location = std::source_location::current(); \
  EXPECT_TRUE(assumption_failed) << #func_name; assumption_failed = false; \
  EXPECT_EQ(last_where.line(), current_location.line()); \
  EXPECT_EQ(last_where.file_name(), current_location.file_name()); \
  EXPECT_STREQ(last_where.function_name(), current_location.function_name()); \
  EXPECT_EQ("test(0, 5)", last_data);
#else
#define EXPECT_ASSUMPTION_FAILED(func_name, ...) \
  func_name SELF(__VA_ARGS__, "test({}, {})", 0, 5);
#endif
#define EXPECT_ASSUMPTION_SUCCEEDED(func_name, ...) func_name SELF(__VA_ARGS__); EXPECT_FALSE(assumption_failed) << #func_name; assumption_failed = false;

TEST_F(assuming_test, Assuming_works)
{
  EXPECT_ASSUMPTION_SUCCEEDED(Assuming, true);

  const bool value = false;
  EXPECT_ASSUMPTION_FAILED(Assuming, value);

  if (assumption_failed)
  {
    name_value_pair const values[] = { {"value", "false"} };
    ASSERT_TRUE(compare(values, last_values));
  }
}

int object{};

TEST_F(assuming_test, AssumingNotNull_works)
{
  EXPECT_ASSUMPTION_SUCCEEDED(AssumingNotNull, &object);

  const decltype(&object) value = nullptr;
  EXPECT_ASSUMPTION_FAILED(AssumingNotNull, value);

  if (assumption_failed)
  {
    name_value_pair const values[] = { {"value", "0x0"} };
    ASSERT_TRUE(compare(values, last_values));
  }
}

TEST_F(assuming_test, AssumingNull_works)
{
  const decltype(&object) value = nullptr;
  EXPECT_ASSUMPTION_SUCCEEDED(AssumingNull, value);

  EXPECT_ASSUMPTION_FAILED(AssumingNull, &object);

  if (assumption_failed)
  {
    name_value_pair const values[] = { {"&object", std::format("{}", (const void*)&object)} };
    ASSERT_TRUE(compare(values, last_values));
  }
}

TEST_F(assuming_test, AssumingEqual_works)
{
  std::pair<int, double> q = { 5, 6 };
  EXPECT_ASSUMPTION_FAILED(AssumingEqual, q.first, q.second);

  if (assumption_failed)
  {
    name_value_pair const values[] = { {"q.first", "5"}, {"q.second", "6"} };
    ASSERT_TRUE(compare(values, last_values));
  }

  const double value = 0.4;
  EXPECT_ASSUMPTION_SUCCEEDED(AssumingEqual, value, 0.4);
}

TEST_F(assuming_test, AssumingNotEqual_works)
{
  EXPECT_ASSUMPTION_SUCCEEDED(AssumingNotEqual, 5, 6);

  const std::string value = "hello";
  AssumingNotEqual(value, "hello");
  EXPECT_ASSUMPTION_FAILED(AssumingNotEqual, value, "hello");

  if (assumption_failed)
  {
    name_value_pair const values[] = { {"value", "0.4"}, {"0.4", "0.4"} };
    ASSERT_TRUE(compare(values, last_values));
  }
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

struct UnCopyable
{
  UnCopyable() = default;
  UnCopyable(UnCopyable const&) = delete;
  UnCopyable(UnCopyable&&) = default;
  UnCopyable& operator=(UnCopyable const&) = delete;
  UnCopyable& operator=(UnCopyable&&) = default;

  bool operator==(UnCopyable const& other) const noexcept { return true; }
};

struct UnMovable
{
  UnMovable() = default;
  UnMovable(UnMovable const&) = delete;
  UnMovable(UnMovable&&) = delete;
  UnMovable& operator=(UnMovable const&) = delete;
  UnMovable& operator=(UnMovable&&) = delete;

  bool operator==(UnMovable const& other) const noexcept { return true; }
};

template <class CharT>
struct std::formatter<UnCopyable, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template<class FormatContext>
  auto format(UnCopyable const& t, FormatContext& fc) {
    return std::formatter<std::basic_string<CharT>, CharT>::format("UnCopyable", fc);
  }
};
template <class CharT>
struct std::formatter<UnMovable, CharT> : std::formatter<std::basic_string<CharT>, CharT>
{
  template<class FormatContext>
  auto format(UnMovable const& t, FormatContext& fc) {
    return std::formatter<std::basic_string<CharT>, CharT>::format("UnMovable", fc);
  }
};

TEST_F(assuming_test, assumings_dont_copy_unnecessarily)
{
  //int i = 0;
  UnCopyable a;
  EXPECT_ASSUMPTION_SUCCEEDED(AssumingEqual, a, UnCopyable{});
  UnMovable b;
  EXPECT_ASSUMPTION_SUCCEEDED(AssumingEqual, b, UnMovable{});
}

/// TODO: Make sure everything works when ASSUMING_DEBUG is not defined

void ghassanpl::ReportAssumptionFailure(std::string_view expectation, std::initializer_list<name_value_pair> values, std::string data, std::source_location where)
{
  current_test->ReportAssumptionFailure(expectation, std::move(values), std::move(data), where);
}
