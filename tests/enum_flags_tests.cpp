/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/enum_flags.h"
#include "../include/ghassanpl/atomic_enum_flags.h"
#include "../include/ghassanpl/bit_view.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <numeric>
#include <ranges>

using namespace ghassanpl;

using integer_types = ::testing::Types<
  short int, unsigned short int, int, unsigned int, long int, unsigned long int, long long int, unsigned long long int,
  signed char, unsigned char, char, wchar_t, char16_t, char32_t, char8_t>;

template <typename RESULT_TYPE>
class bits_test : public ::testing::Test {
public:

	using result_type = RESULT_TYPE;

};
TYPED_TEST_SUITE(bits_test, integer_types);

TYPED_TEST(bits_test, bit_reference_works)
{
	TypeParam value = 10;

	static_assert(sizeof(bit_reference<TypeParam>) > sizeof(bit_reference<TypeParam, 2>));

	bit_reference<TypeParam> bit_2_of_value{ value, 2 };
	bit_reference<TypeParam> bit_2_of_value_s{ value, detail::bit_num<2> };

	bit_2_of_value = true;
	EXPECT_EQ(value, 14);
	bit_2_of_value_s = false;
	EXPECT_EQ(value, 10);

	EXPECT_EQ(&bit_2_of_value.integer_value(), &value);
	EXPECT_EQ(bit_2_of_value.bit_number(), 2);

	if constexpr (std::is_signed_v<TypeParam>)
	{
		bit_reference msb{ value, detail::bit_num<bit_count<TypeParam> - 1> };
		msb = true;
		EXPECT_LT(value, 0);
	}
}

TYPED_TEST(bits_test, bit_view_works)
{
	std::vector<int> ints{ 20,30,40 };
	bit_view view{ ints };

	auto bit_42_of_value = make_bit_reference(ints, 42);
	auto bit_42_of_value_s = make_bit_reference<42>(ints);

	EXPECT_TRUE(bit_42_of_value == bit_42_of_value_s);
	bit_42_of_value = true;
	EXPECT_TRUE(bit_42_of_value == bit_42_of_value_s);
	bit_42_of_value_s = false;
	EXPECT_TRUE(bit_42_of_value == bit_42_of_value_s);

	std::vector<int> const const_ints{ 20,30,40 };
	const bit_view const_view{ const_ints };

	auto bit_42_of_const_value = make_bit_reference(const_ints, 42);
	auto bit_42_of_const_value_s = make_bit_reference<42>(const_ints);

	EXPECT_EQ(bit_42_of_const_value_s.bit_number(), 10);

	std::string out;
	std::ranges::transform(const_view, std::back_inserter(out), [](auto bit) { return bit ? '1' : '0'; });

	EXPECT_EQ(out, 
		"00101000000000000000000000000000"
		"01111000000000000000000000000000"
		"00010100000000000000000000000000");
}

TYPED_TEST(bits_test, bit_view_works_for_empty_range)
{
	std::vector<TypeParam> const const_values{};
	const bit_view const_view{ const_values };
	EXPECT_THROW({ std::ignore = const_view.at(0); }, std::invalid_argument);
}

template <typename RESULT_TYPE>
class flag_bits_test : public ::testing::Test {
public:

	using result_type = RESULT_TYPE;

};
TYPED_TEST_SUITE(flag_bits_test, integer_types);

enum class TestEnum : int64_t
{
  Negative = -1, /// causes undefined behavior
  Zero = 0,
  ZeroTwo = 0,
  One = 1,
  OneTwo = 1,
  Seven = 7,
  Eight,
  Nine,
  Fifteen = 15,
  Sixteen,
  Seventeen,
  ThirtyOne = 31,
  ThirtyTwo,
  ThirtyThree,
  SixtyThree = 63,
  SixtyFour,
  SixtyFive,
  Small = std::numeric_limits<int64_t>::lowest(),
  Big = std::numeric_limits<int64_t>::max(),
};

enum class UnsignedTestEnum : uint64_t
{
  Big = std::numeric_limits<uint64_t>::max(),
};

using namespace ghassanpl;

template <class U, auto... ARGS>
concept flag_bits_v_overload_exists = requires { ::ghassanpl::flag_bits_v<U, ARGS...>; };
template <class U, auto... ARGS>
concept flag_bits_overload_exists = requires { ::ghassanpl::flag_bits<U>(std::declval<ARGS>()...); };

#define EXPECT_EQ_MEH(a, b) EXPECT_EQ(uint64_t(a), uint64_t(b))

TYPED_TEST(flag_bits_test, work_with_template_parameters)
{
  EXPECT_EQ_MEH((flag_bits_v<TypeParam>), TypeParam{ 0 });
  EXPECT_EQ_MEH((flag_bits_v<TypeParam, TestEnum::Zero>), TypeParam{ 1 });
  EXPECT_EQ_MEH((flag_bits_v<TypeParam, TestEnum::Zero, TestEnum::ZeroTwo>), TypeParam{ 1 });
  EXPECT_EQ_MEH((flag_bits_v<TypeParam, TestEnum::One>), TypeParam{ 2 });
  EXPECT_EQ_MEH((flag_bits_v<TypeParam, TestEnum::Zero, TestEnum::ZeroTwo, TestEnum::One>), TypeParam{ 3 });
}

TYPED_TEST(flag_bits_test, is_flag_set_v_works)
{
  EXPECT_TRUE((is_flag_set_v<flag_bits_v<TypeParam, TestEnum::Zero>, TestEnum::Zero>));
  EXPECT_FALSE((is_flag_set_v<flag_bits_v<TypeParam, TestEnum::Zero>, TestEnum::One>));
  EXPECT_FALSE((is_flag_set_v<flag_bits_v<TypeParam, TestEnum::One>, TestEnum::Zero>));
}

TYPED_TEST(flag_bits_test, are_all_flags_set_v_works)
{
  EXPECT_TRUE((are_all_flags_set_v<flag_bits_v<TypeParam, TestEnum::One, TestEnum::Zero, TestEnum::Seven>, TestEnum::Zero, TestEnum::Seven, TestEnum::One>));
  EXPECT_FALSE((are_all_flags_set_v<flag_bits_v<TypeParam, TestEnum::One, TestEnum::Seven>, TestEnum::Zero, TestEnum::Seven, TestEnum::One>));
  EXPECT_TRUE((are_all_flags_set_v<flag_bits_v<TypeParam, TestEnum::One, TestEnum::Zero, TestEnum::Seven>, TestEnum::Zero, TestEnum::One>));
}

TYPED_TEST(flag_bits_test, are_any_flags_set_v_works)
{
  EXPECT_TRUE((are_any_flags_set_v<flag_bits_v<TypeParam, TestEnum::One>, TestEnum::Zero, TestEnum::Seven, TestEnum::One>));
  EXPECT_FALSE((are_any_flags_set_v<flag_bits_v<TypeParam, TestEnum::Zero>, TestEnum::Seven, TestEnum::One>));
  EXPECT_TRUE((are_any_flags_set_v<flag_bits_v<TypeParam, TestEnum::Zero, TestEnum::Seven>, TestEnum::Seven, TestEnum::One>));
}

TYPED_TEST(flag_bits_test, set_flag_v_works)
{
  constexpr auto one_set = flag_bits_v<TypeParam, TestEnum::One>;
  EXPECT_EQ_MEH((set_flag_v<one_set>), one_set);
  EXPECT_EQ_MEH((set_flag_v<one_set, TestEnum::Seven>), (flag_bits_v<TypeParam, TestEnum::Seven, TestEnum::One>));
  EXPECT_EQ_MEH((set_flag_v<TypeParam(0), TestEnum::Seven>), (flag_bits_v<TypeParam, TestEnum::Seven>));
  EXPECT_EQ_MEH((set_flag_v<TypeParam(0)>), (flag_bits_v<TypeParam>));
  EXPECT_EQ_MEH((set_flag_v<one_set>), one_set);
}

TYPED_TEST(flag_bits_test, unset_flag_v_works)
{
  constexpr auto one_set = flag_bits_v<TypeParam, TestEnum::One>;
  EXPECT_EQ_MEH((unset_flag_v<one_set>), one_set);
  EXPECT_EQ_MEH((unset_flag_v<one_set, TestEnum::Seven>), one_set);
  EXPECT_EQ_MEH((unset_flag_v<one_set, TestEnum::One>), TypeParam{ 0 });
  constexpr auto seventyone = flag_bits_v<TypeParam, TestEnum::Seven, TestEnum::One>;
  EXPECT_EQ_MEH((unset_flag_v<seventyone, TestEnum::One>), (flag_bits_v<TypeParam, TestEnum::Seven>));
  EXPECT_EQ_MEH((unset_flag_v<seventyone, TestEnum::Seven>), (flag_bits_v<TypeParam, TestEnum::One>));
  EXPECT_EQ_MEH((unset_flag_v<seventyone, TestEnum::Seven, TestEnum::One>), (flag_bits_v<TypeParam>));
  EXPECT_EQ_MEH((unset_flag_v<seventyone, TestEnum::Zero, TestEnum::One>), (flag_bits_v<TypeParam, TestEnum::Seven>));
}

TYPED_TEST(flag_bits_test, toggle_flag_v_works)
{
  constexpr auto bits = flag_bits_v<TypeParam, TestEnum::Seven, TestEnum::One, TestEnum::Seven, TestEnum::Zero>;
  constexpr auto bits2 = flag_bits_v<TypeParam, TestEnum::Seven>;
  EXPECT_EQ_MEH((toggle_flag_v<bits, TestEnum::Zero, TestEnum::One>), bits2);
  EXPECT_EQ_MEH((toggle_flag_v<bits>), bits);
}

TYPED_TEST(flag_bits_test, set_flag_to_v_works)
{
  constexpr auto bits = flag_bits_v<TypeParam, TestEnum::Seven, TestEnum::One, TestEnum::Seven, TestEnum::Zero>;
  EXPECT_EQ_MEH((set_flag_to_v<bits, false, TestEnum::Seven, TestEnum::Zero>), (flag_bits_v<TypeParam, TestEnum::One>));
  EXPECT_EQ_MEH((set_flag_to_v<(flag_bits_v<TypeParam, TestEnum::One>), true, TestEnum::Seven, TestEnum::Zero>), bits);
}

TYPED_TEST(flag_bits_test, disallow_invalid_bit_numbers_for_template_parameters)
{
  EXPECT_FALSE((flag_bits_v_overload_exists<TypeParam, TestEnum::Negative>));

  EXPECT_FALSE((flag_bits_v_overload_exists<TypeParam, TestEnum::Small>));
  EXPECT_FALSE((flag_bits_v_overload_exists<TypeParam, TestEnum::Big>));

  EXPECT_FALSE((flag_bits_v_overload_exists<TypeParam, UnsignedTestEnum::Big>));
}

TEST(flag_bits_test, disallow_invalid_bit_numbers_for_ints)
{
  EXPECT_TRUE((flag_bits_v_overload_exists<uint8_t, TestEnum::Seven>));
  EXPECT_FALSE((flag_bits_v_overload_exists<uint8_t, TestEnum::Eight>));
  EXPECT_TRUE((flag_bits_v_overload_exists<uint16_t, TestEnum::Nine>));
  EXPECT_TRUE((flag_bits_v_overload_exists<uint16_t, TestEnum::Fifteen>));
  EXPECT_FALSE((flag_bits_v_overload_exists<uint16_t, TestEnum::Sixteen>));
  EXPECT_TRUE((flag_bits_v_overload_exists<uint32_t, TestEnum::Seventeen>));
  EXPECT_TRUE((flag_bits_v_overload_exists<uint32_t, TestEnum::ThirtyOne>));
  EXPECT_FALSE((flag_bits_v_overload_exists<uint32_t, TestEnum::ThirtyTwo>));
  EXPECT_TRUE((flag_bits_v_overload_exists<uint64_t, TestEnum::ThirtyThree>));
  EXPECT_TRUE((flag_bits_v_overload_exists<uint64_t, TestEnum::SixtyThree>));
  EXPECT_FALSE((flag_bits_v_overload_exists<uint64_t, TestEnum::SixtyFour>));
  EXPECT_FALSE((flag_bits_v_overload_exists<uint64_t, TestEnum::SixtyFive>));

  EXPECT_TRUE((flag_bits_v_overload_exists<int8_t, TestEnum::Seven>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int8_t, TestEnum::Eight>));
  EXPECT_TRUE((flag_bits_v_overload_exists<int16_t, TestEnum::Nine>));
  EXPECT_TRUE((flag_bits_v_overload_exists<int16_t, TestEnum::Fifteen>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int16_t, TestEnum::Sixteen>));
  EXPECT_TRUE((flag_bits_v_overload_exists<int32_t, TestEnum::Seventeen>));
  EXPECT_TRUE((flag_bits_v_overload_exists<int32_t, TestEnum::ThirtyOne>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int32_t, TestEnum::ThirtyTwo>));
  EXPECT_TRUE((flag_bits_v_overload_exists<int64_t, TestEnum::ThirtyThree>));
  EXPECT_TRUE((flag_bits_v_overload_exists<int64_t, TestEnum::SixtyThree>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int64_t, TestEnum::SixtyFour>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int64_t, TestEnum::SixtyFive>));
}


TEST(flag_bits_test, disallow_non_integral_types)
{
  EXPECT_FALSE((flag_bits_v_overload_exists<bool>));
  EXPECT_FALSE((flag_bits_v_overload_exists<std::nullptr_t>));
  EXPECT_FALSE((flag_bits_v_overload_exists<float>));
  EXPECT_FALSE((flag_bits_v_overload_exists<double>));
  EXPECT_FALSE((flag_bits_v_overload_exists<long double>));
  EXPECT_FALSE((flag_bits_v_overload_exists<void>));

  class _c {};
  union _u {};
  enum _e {};
  enum class _es {};

  EXPECT_FALSE((flag_bits_v_overload_exists<_c>));
  EXPECT_FALSE((flag_bits_v_overload_exists<_u>));
  EXPECT_FALSE((flag_bits_v_overload_exists<_e>));
  EXPECT_FALSE((flag_bits_v_overload_exists<_es>));

  EXPECT_FALSE((flag_bits_v_overload_exists<int&>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int*>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int&&>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int(*)()>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int[5]>));
  EXPECT_FALSE((flag_bits_v_overload_exists<int(std::string::*)>));
}

TYPED_TEST(flag_bits_test, allows_cv_types)
{
  EXPECT_TRUE((flag_bits_v_overload_exists<std::add_const_t<TypeParam>>));
  EXPECT_TRUE((flag_bits_v_overload_exists<std::add_volatile_t<TypeParam>>));
  EXPECT_TRUE((flag_bits_v_overload_exists<std::add_cv_t<TypeParam>>));
}

TEST(enum_flags_test, changes_work)
{
	enum_flags<TestEnum> test;
	test.set(TestEnum::Eight, TestEnum::Fifteen);
	enum_flag_changes<TestEnum> changes;
	changes.unset(TestEnum::Eight);
	changes.toggle(TestEnum::Fifteen, TestEnum::Nine);
	EXPECT_EQ(test + changes, enum_flags<TestEnum>{TestEnum::Nine});
}

static_assert(enum_flags<int>::all().contains_all_of());
static_assert(enum_flags<int>{5}.contains_all_of());
static_assert(enum_flags<int>::none().contains_all_of());
static_assert(enum_flags<int>::all().full());
static_assert(!enum_flags<int>{5}.full());
static_assert(!enum_flags<int>::none().full());
