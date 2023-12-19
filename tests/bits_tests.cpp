#include <gtest/gtest.h>
#include "../include/ghassanpl/bits.h"

TEST(BitTest, BitIntegral) {
    bool a = 5;
    ASSERT_FALSE(ghassanpl::bit_integral<decltype(a)>);
}

TEST(BitTest, BitCount) {
    int a = 5;
    ASSERT_EQ(ghassanpl::bit_count<decltype(a)>, sizeof(a) * CHAR_BIT);
}

TEST(BitTest, AllBits) {
    ASSERT_EQ(ghassanpl::all_bits, ~uint64_t{ 0 });
}

TEST(BitTest, BitMaskV) {
    ASSERT_EQ((ghassanpl::bit_mask_v<1, 32>), (ghassanpl::all_bits >> 32) << 1);
}

TEST(BitTest, BitMaskForV) {
    uint8_t a = 5;
    ASSERT_EQ(ghassanpl::bit_mask_for_v<decltype(a)>, (ghassanpl::all_bits >> (64 - ghassanpl::bit_count<decltype(a)>)));
}

TEST(BitTest, MostSignificantHalf) {
    uint16_t a = 0b1100110011001100;
    ASSERT_EQ(ghassanpl::most_significant_half(a), 0b11001100);
}

TEST(BitTest, LeastSignificantHalf) {
    uint16_t a = 0b1100110011001100;
    ASSERT_EQ(ghassanpl::least_significant_half(a), 0b11001100);
}

TEST(BitTest, ToBigEndian) {
    uint16_t a = 0b1100110000111100;
    ASSERT_EQ(ghassanpl::to_big_endian(a), std::endian::native == std::endian::big ? a : 0b0011110011001100);
}

TEST(BitTest, ToLittleEndian) {
    uint16_t a = 0b1100110000111100;
    ASSERT_EQ(ghassanpl::to_little_endian(a), std::endian::native == std::endian::little ? a : 0b0011110011001100);
}

TEST(BitTest, BitReference) {
    uint8_t a = 0b11001100;
    ghassanpl::bit_reference ref(a, 2);
    ASSERT_EQ(static_cast<bool>(ref), true);
    ref = false;
    ASSERT_EQ(static_cast<bool>(ref), false);
    ASSERT_EQ(ref.integer_value(), 0b11001000);
    ASSERT_EQ(ref.bit_number(), 2);
}
