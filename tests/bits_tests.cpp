/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "tests_common.h"
#include "../include/ghassanpl/bits.h"
#include "../include/ghassanpl/bit_view.h"

TYPED_TEST_SUITE(bits_test, integer_types);

TYPED_TEST(bits_test, bit_reference_works)
{
	using namespace ghassanpl;

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

	if constexpr (std::is_signed_v<TypeParam>) {
		bit_reference msb{ value, detail::bit_num<bit_count<TypeParam> -1> };
		msb = true;
		EXPECT_LT(value, 0);
	}
}

TYPED_TEST(bits_test, bit_view_works)
{
	using namespace ghassanpl;

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
	using namespace ghassanpl;

	std::vector<TypeParam> const const_values{};
	const bit_view const_view{ const_values };
	EXPECT_THROW({ std::ignore = const_view.at(0); }, std::invalid_argument);
}

TEST(bits_test, BitIntegral) {
	bool a = 5;
	ASSERT_FALSE(ghassanpl::bit_integral<decltype(a)>);
}

TEST(bits_test, BitCount) {
	int a = 5;
	ASSERT_EQ(ghassanpl::bit_count<decltype(a)>, sizeof(a) * CHAR_BIT);
}

TEST(bits_test, AllBits) {
	ASSERT_EQ(ghassanpl::all_bits, ~uint64_t{ 0 });
}

TEST(bits_test, BitMaskV) {
	ASSERT_EQ((ghassanpl::bit_mask_v<1, 32>), (ghassanpl::all_bits >> 32) << 1);
}

TEST(bits_test, BitMaskForV) {
	uint8_t a = 5;
	ASSERT_EQ(ghassanpl::bit_mask_for_v<decltype(a)>, (ghassanpl::all_bits >> (64 - ghassanpl::bit_count<decltype(a)>)));
}

TEST(bits_test, MostSignificantHalf) {
	uint16_t a = 0b1100110011001100;
	ASSERT_EQ(ghassanpl::most_significant_half(a), 0b11001100);
}

TEST(bits_test, LeastSignificantHalf) {
	uint16_t a = 0b1100110011001100;
	ASSERT_EQ(ghassanpl::least_significant_half(a), 0b11001100);
}

TEST(bits_test, ToBigEndian) {
	uint16_t a = 0b1100110000111100;
	ASSERT_EQ(ghassanpl::to_big_endian(a), std::endian::native == std::endian::big ? a : 0b0011110011001100);
}

TEST(bits_test, ToLittleEndian) {
	uint16_t a = 0b1100110000111100;
	ASSERT_EQ(ghassanpl::to_little_endian(a), std::endian::native == std::endian::little ? a : 0b0011110011001100);
}

TEST(bits_test, BitReference) {
	uint8_t a = 0b11001100;
	ghassanpl::bit_reference ref(a, 2);
	ASSERT_EQ(static_cast<bool>(ref), true);
	ref = false;
	ASSERT_EQ(static_cast<bool>(ref), false);
	ASSERT_EQ(ref.integer_value(), 0b11001000);
	ASSERT_EQ(ref.bit_number(), 2);
}

TEST(bits_test, extract_bits_works)
{
	auto a = 0xF0F0;
	EXPECT_EQ((ghassanpl::extract_bits<4, 8>(a)), 0xF);
	EXPECT_EQ((ghassanpl::extract_bits<8, 12>(a)), 0);
	EXPECT_EQ((ghassanpl::extract_bits<12, 16>(a)), 0xF);
	EXPECT_EQ((ghassanpl::extract_bits<0, 4>(a)), 0x0);
	EXPECT_EQ((ghassanpl::extract_bits<0, 8>(a)), 0xF0);
	EXPECT_EQ((ghassanpl::extract_bits<0, 12>(a)), 0xF0);
	EXPECT_EQ((ghassanpl::extract_bits<0, 16>(a)), 0xF0F0);
	EXPECT_EQ((ghassanpl::extract_bits<16, 32>(a)), 0);
	EXPECT_EQ((ghassanpl::extract_bits<15, 16>(a)), 1);
}

TEST(bits_test, split_bit_ranges_works)
{
	auto [a, b, c] = ghassanpl::split_bit_ranges<4, 4, 8>(uint16_t(0xF0F0));
	EXPECT_EQ(a, 0);
	EXPECT_EQ(b, 0xF);
	EXPECT_EQ(c, 0xF0);
}
