/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/bytes.h"
#include "../include/ghassanpl/bits.h"

#include <algorithm>

#include <gtest/gtest.h>

#include "test_system.h"

using namespace ghassanpl;

TEST(to_bytelike_array, works_like_reinterpret_cast)
{
	constexpr uint64_t hello_local = 0xFFEEDDCC'BBAA9988ULL;
	constexpr uint64_t hello_le = to_little_endian(hello_local);
	constexpr uint64_t hello_be = to_big_endian(hello_local);
	constexpr auto chars_le = to_bytelike_array<char>(hello_le);
	constexpr auto chars_be = to_bytelike_array<char>(hello_be);

	const auto chars_le2 = as_chars(hello_le);
	const auto chars_be2 = as_chars(hello_be);

	if constexpr (std::endian::native == std::endian::little)
	{
		EXPECT_TRUE(std::ranges::equal(chars_le, chars_le2));
		EXPECT_FALSE(std::ranges::equal(chars_le, chars_be2));
	}
	else
	{
		EXPECT_TRUE(std::ranges::equal(chars_be, chars_be2));
		EXPECT_FALSE(std::ranges::equal(chars_be, chars_le2));
	}
}

TEST(bytelike_range, applies_to_common_types)
{
	static_assert(bytelike_range<std::string>);
	static_assert(bytelike_range<std::string_view>);
	static_assert(bytelike_range<std::span<uint8_t>>);
	static_assert(bytelike_range<std::span<uint8_t const>>);
	static_assert(!bytelike_range<const char*>);
}

ConceptUnderTest(bytelike_range)
{
	ForEachType(std::string, std::string_view, std::span<uint8_t>, std::span<uint8_t const>)
	{
		CheckingIfConcept("is true for common type {}", typeid(TypeParam).name())
		{
			ConceptShouldBe(TrueForThisType).WhenTrue(bytelike_range<TypeParam>);

			ConceptShouldBe(TrueForConstVersionOfThisType);
			IsTrueForConstVersionOfThisType.WhenTrue(bytelike_range<const TypeParam>);
		}
	};

	CheckingIfConcept("is false for pointer types")
	{
		ConceptShouldBe(FalseForConstChar).WhenTrue(bytelike_range<const char*> == false);
	}
}

TEST(byte_range_align_front_to, works)
{
	const std::span<char> span{ (char*)1, 32 };

	auto [prefix, aligned] = align_front_to<16>(span);
	ASSERT_FALSE(prefix.empty());
	ASSERT_FALSE(aligned.empty());
	EXPECT_EQ(aligned.size(), 17);
	EXPECT_EQ(prefix.size() + aligned.size(), span.size());

	EXPECT_EQ(prefix.data(), span.data());
	EXPECT_EQ(prefix.data() + prefix.size(), aligned.data());

	/// Not technically portable, but if the logic behind the implementation is sound,
	/// the test will succeed.
	EXPECT_EQ(((uintptr_t)aligned.data()) % 16, 0);
}

TEST(byte_range_align_back_to, works)
{
	const std::span<char> span{ (char*)16, 63 };

	auto [aligned, suffix] = align_back_to<16>(span);
	ASSERT_FALSE(aligned.empty());
	ASSERT_FALSE(suffix.empty());
	EXPECT_EQ(aligned.size(), 48);
	EXPECT_EQ(aligned.size() + suffix.size(), span.size());

	EXPECT_EQ(aligned.data() + aligned.size(), suffix.data());
	EXPECT_EQ(suffix.data() + suffix.size(), span.data() + span.size());

	/// Not technically portable, but if the logic behind the implementation is sound,
	/// the test will succeed.
	EXPECT_EQ(aligned.size() % 16, 0);
}

TEST(byte_range_align_to, works)
{
	const std::span<char> span{ (char*)13, 69 };

	auto [prefix, aligned, suffix] = align_to<16>(span);
	ASSERT_FALSE(prefix.empty());
	ASSERT_FALSE(aligned.empty());
	ASSERT_FALSE(suffix.empty());
	EXPECT_EQ(aligned.size(), 64);
	EXPECT_EQ(prefix.size() + aligned.size() + suffix.size(), span.size());

	EXPECT_EQ(prefix.data(), span.data());
	EXPECT_EQ(prefix.data() + prefix.size(), aligned.data());
	EXPECT_EQ(aligned.data() + aligned.size(), suffix.data());
	EXPECT_EQ(suffix.data() + suffix.size(), span.data() + span.size());

	/// Not technically portable, but if the logic behind the implementation is sound,
	/// the test will succeed.
	EXPECT_EQ(((uintptr_t)aligned.data()) % 16, 0);
	EXPECT_EQ(aligned.size() % 16, 0);

	EXPECT_EQ(((uintptr_t)suffix.data()) % 16, 0);
}

TEST(byte_range_align_front_to, handles_failures_gracefully)
{
	const std::span<char> span{ (char*)1, 32 };

	{
		auto [prefix, aligned] = align_front_to<64>(span);
		ASSERT_TRUE(aligned.empty());
		EXPECT_EQ(prefix.size(), span.size());
		EXPECT_EQ(prefix.data(), span.data());
	}

	{
		auto [prefix, aligned] = align_front_to<32>(span);
		ASSERT_TRUE(aligned.empty());
		EXPECT_EQ(prefix.size(), span.size());
		EXPECT_EQ(prefix.data(), span.data());
	}
}
