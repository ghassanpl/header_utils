/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/hashes.h"

#include <gtest/gtest.h>
#include <print>

using namespace ghassanpl;

/*
TEST(crc32, works_at_compile_time)
{
}
TEST(crc64, works_at_compile_time)
{
}
TEST(fnv, works_at_compile_time)
{
}
*/

TEST(constexpr_hashes, work_for_all_supported_types)
{
	constexpr auto ui8_hash = ce_hash64(uint8_t(14));
	constexpr auto i8_hash = ce_hash64(uint8_t(14));
	static_assert(ui8_hash == i8_hash);
	constexpr auto ui16_hash = ce_hash64(uint16_t(14));
	constexpr auto i16_hash = ce_hash64(int16_t(14));
	static_assert(ui16_hash == i16_hash);
	constexpr auto ui32_hash = ce_hash64(uint32_t(14));
	constexpr auto i32_hash = ce_hash64(int32_t(14));
	static_assert(ui32_hash == i32_hash);
	constexpr auto szt_hash = ce_hash64(size_t(14));
	constexpr auto dbl_hash = ce_hash64(14.0);
	constexpr auto flt_hash = ce_hash64(14.0f);
	constexpr auto null_hash = ce_hash64(nullptr);

	/// This only works on MSVC because it uses the same hash algo (fnv)
	/// EXPECT_EQ(std::hash<float>{}(14.0f), flt_hash);
}