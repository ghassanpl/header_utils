/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/string_ops.h"

#include <gtest/gtest.h>

using namespace ghassanpl::string_ops;

using std::string;
using std::string_view;

template <typename F1, typename F2>
void same(F1&& f1, F2&& f2, const char* name)
{
	for (int cp = 0; cp < 256; ++cp)
	{
		auto a = f1(cp);
		auto b = f2(cp);
		EXPECT_EQ((decltype(b))a, b) << "function " << name << " for codepoint " << cp << "\n";
	}
}

#define FU(name) same(::name, ascii::name, #name)

TEST(string_ops_test, ascii_functions_are_correct)
{
	FU(isalpha);
	FU(isdigit);
	FU(isxdigit);
	FU(isalnum);
	FU(isspace);
	FU(ispunct);
	FU(islower);
	FU(isupper);
	FU(iscntrl);
	FU(isblank);
	FU(isgraph);
	FU(isprint);

	same(::toupper, (char32_t(*)(char32_t))ascii::toupper, "toupper");
	same(::tolower, (char32_t(*)(char32_t))ascii::tolower, "tolower");

	const char digits[] = "0123456789ABCDEF";
	for (int i = 0; i < 16; i++)
	{
		EXPECT_EQ(digits[i], ascii::toxdigit(i));
		if (i < 10)
			EXPECT_EQ(digits[i], ascii::todigit(i));
	}

	EXPECT_EQ(ascii::tolower("woof"), "woof");
	EXPECT_EQ(ascii::tolower("WoOf"), "woof");
	EXPECT_EQ(ascii::tolower(""), "");
	EXPECT_EQ(ascii::toupper("woof"), "WOOF");
	EXPECT_EQ(ascii::toupper("WoOf"), "WOOF");
	EXPECT_EQ(ascii::toupper(""), "");

	/// The below string should be longer than any reasonable SSO can handle
	constexpr const char longer_str[] = "R=;B!wxhRVhS@nYQ;cUy&pDp9pB]NMwiMTShSx{8MDRGjth9NM{7jG,H/AE_v5?67A3.}8q]2d]-2([pU{[9c!epj;-g%d,v@=CMnGGM$g8JdG@b3jp,dD:[B37y2.CFKD";
	constexpr const char longer_lower[] = "r=;b!wxhrvhs@nyq;cuy&pdp9pb]nmwimtshsx{8mdrgjth9nm{7jg,h/ae_v5?67a3.}8q]2d]-2([pu{[9c!epj;-g%d,v@=cmnggm$g8jdg@b3jp,dd:[b37y2.cfkd";
	constexpr const char longer_upper[] = "R=;B!WXHRVHS@NYQ;CUY&PDP9PB]NMWIMTSHSX{8MDRGJTH9NM{7JG,H/AE_V5?67A3.}8Q]2D]-2([PU{[9C!EPJ;-G%D,V@=CMNGGM$G8JDG@B3JP,DD:[B37Y2.CFKD";
	EXPECT_EQ(ascii::tolower(longer_str), longer_lower);
	EXPECT_EQ(ascii::tolower(string{ longer_str }), longer_lower);
	EXPECT_EQ(ascii::tolower(string_view{ longer_str }), longer_lower);

	EXPECT_EQ(ascii::toupper(longer_str), longer_upper);
	EXPECT_EQ(ascii::toupper(string{ longer_str }), longer_upper);
	EXPECT_EQ(ascii::toupper(string_view{ longer_str }), longer_upper);

	EXPECT_TRUE(ascii::strings_equal_ignore_case(longer_str, longer_lower));
	EXPECT_TRUE(ascii::strings_equal_ignore_case(longer_str, longer_upper));
	EXPECT_TRUE(ascii::strings_equal_ignore_case(longer_lower, longer_upper));

	EXPECT_TRUE(ascii::lexicographical_compare_ignore_case("a", "b"));
	EXPECT_FALSE(ascii::lexicographical_compare_ignore_case("a", "A"));
	EXPECT_TRUE(ascii::lexicographical_compare_ignore_case("a", "aa"));
	EXPECT_TRUE(ascii::lexicographical_compare_ignore_case("a", "Aa"));
	EXPECT_TRUE(ascii::lexicographical_compare_ignore_case("a", "bA"));
	EXPECT_TRUE(ascii::lexicographical_compare_ignore_case("a", "Ba"));
	EXPECT_FALSE(ascii::lexicographical_compare_ignore_case("B", "aB"));
	EXPECT_FALSE(ascii::lexicographical_compare_ignore_case("B", "Ab"));
	EXPECT_FALSE(ascii::lexicographical_compare_ignore_case("B", "AB"));

	EXPECT_FALSE(ascii::lexicographical_compare_ignore_case("", ""));
	EXPECT_FALSE(ascii::lexicographical_compare_ignore_case("a", ""));
	EXPECT_TRUE(ascii::lexicographical_compare_ignore_case("", "a"));
}

TEST(string_ops_test, contains_works)
{
	EXPECT_TRUE(contains("hello", 'e'));
	EXPECT_FALSE(contains("hello", 'w'));
	EXPECT_FALSE(contains("", 'w'));
	EXPECT_FALSE(contains("", '\0'));
	EXPECT_FALSE(contains("DAYUM", '\0'));
}

TEST(string_ops_test, make_sv_works)
{

}

TEST(string_ops_test, make_sv_works_for_nulls)
{

}

TEST(string_ops_test, make_sv_fails_for_invalid_range)
{
	//std::string hello = "hello";
	//EXPECT_EQ(make_sv(hello.end(), hello.begin()), "");
}
