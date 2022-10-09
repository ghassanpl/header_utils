/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/buffers.h"

#include <gtest/gtest.h>

using namespace ghassanpl;


GTEST_API_ void testing::internal::PrintU8StringTo(const ::std::u8string& s, ::std::ostream* os)
{
	(*os) << std::string_view{ (const char*)s.data(), s.size() };
}

TEST(buffers, basics)
{
	const char arr[] =  "yo ";

	std::u8string dest;

	EXPECT_EQ(buffer_append_range(dest, arr), 4);
	EXPECT_EQ(buffer_append_range(dest, std::string_view{ arr }), 3);
	const char8_t expected[] = u8"yo \0yo ";
	EXPECT_EQ(dest, (std::u8string_view{ expected, sizeof(expected) - 1}));

	dest = {};
	buffer_append_utf8(dest, U'Z'); buffer_append_utf8(dest, U'a'); buffer_append_utf8(dest, U'ż'); buffer_append_utf8(dest, U'ó'); buffer_append_utf8(dest, U'ł'); buffer_append_utf8(dest, U'ć');
	buffer_append_utf8(dest, std::u32string_view{ U" gęślą" });
	EXPECT_EQ(dest, u8"Zażółć gęślą");

	dest = {};
	uint32_t bleh = 'damn';
	buffer_append_range(dest, as_chars(bleh));
	EXPECT_EQ(dest, u8"nmad");
	buffer_append_pod(dest, bleh);
	EXPECT_EQ(dest, u8"nmadnmad");
}