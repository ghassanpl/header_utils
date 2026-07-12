/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/string_ops.h"
#include "../include/ghassanpl/unicode.h"
#include "../include/ghassanpl/ranges.h"
#include "../include/ghassanpl/stringification.h"
#include "../include/ghassanpl/rec2.h"

#include <array>
#include <gtest/gtest.h>
#undef isascii

using namespace ghassanpl::string_ops;

using std::string;
using std::string_view;
using namespace std::string_view_literals;

template <typename F1, typename F2>
void same(F1&& f1, F2&& f2, const char* name)
{
	for (int cp = -1; cp < 256; ++cp) /// same range as C functions support
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

	/// Check for isident00
	for (int cp = -1; cp < 256; ++cp) /// same range as C functions support
	{
		auto a = ascii::isident(cp);
		auto b = (bool)::isalnum(cp);
		if (cp == '_')
			EXPECT_TRUE(ascii::isident(cp));
		else
			EXPECT_EQ((decltype(b))a, b) << "function isident for codepoint " << cp << "\n";
	}

	same(::toupper, (char32_t(*)(char32_t))ascii::toupper, "toupper");
	same(::tolower, (char32_t(*)(char32_t))ascii::tolower, "tolower");

	const char digits[] = "0123456789ABCDEF";
	for (int i = 0; i < 16; i++)
	{
		EXPECT_EQ(digits[i], (uint32_t)ascii::number_to_xdigit(i));
		EXPECT_EQ(ascii::xdigit_to_number(ascii::number_to_xdigit(i)), i);
		if (i < 10)
		{
			EXPECT_EQ(digits[i], (uint32_t)ascii::number_to_digit(i));
			EXPECT_EQ(ascii::digit_to_number(ascii::number_to_digit(i)), i);
		}
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

	/// TODO: string_starts_with_ignore_case, string_find_ignore_case, string_find_last_ignore_case, string_contains_ignore_case

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

template <typename T>
class StringableTestFixture : public ::testing::Test
{
public:
	T null_value = {};
	T empty_string_value{ ""};
	T complex_value{ "ZCoo(01_;" };
	T embedded_zeroes_value{ "asdf\0ZXCV" };
};

using StringableTypes = ::testing::Types<char const*, std::string_view, const char[10], std::string>;
TYPED_TEST_SUITE(StringableTestFixture, StringableTypes);

TYPED_TEST(StringableTestFixture, ascii_works_with_all_stringable_types)
{
	EXPECT_EQ(ascii::tolower(this->null_value), std::string_view{});
	EXPECT_EQ(ascii::tolower(this->empty_string_value), std::string_view{});
	EXPECT_EQ(ascii::tolower(this->complex_value), "zcoo(01_;");
	EXPECT_EQ(ascii::tolower(this->embedded_zeroes_value), "asdf\0ZXCV");

	EXPECT_EQ(ascii::toupper(this->null_value), std::string_view{});
	EXPECT_EQ(ascii::toupper(this->empty_string_value), std::string_view{});
	EXPECT_EQ(ascii::toupper(this->complex_value), "ZCOO(01_;");
	EXPECT_EQ(ascii::toupper(this->embedded_zeroes_value), "ASDF\0ZXCV");
}

/*
TEST(string_ops, ascii_works_with_nonconst_char_arrays)
{
	char empty_string_value[] = "";
	char complex_value[] = "ZCoo(01_;" ;
	char embedded_zeroes_value[] = "asdf\0ZXCV";
	EXPECT_EQ(ascii::tolower(empty_string_value), std::string_view{ "\0" });
	EXPECT_EQ(ascii::tolower(complex_value), "zcoo(01_;\0");
	EXPECT_EQ(ascii::tolower(embedded_zeroes_value), "asdf\0ZXCV\0");

	EXPECT_EQ(ascii::toupper(empty_string_value), "\0");
	EXPECT_EQ(ascii::toupper(complex_value), "ZCOO(01_;");
	EXPECT_EQ(ascii::toupper(embedded_zeroes_value), "ASDF\0ZXCV");
}
*/

template <typename T, int VALUE>
concept narrowable = requires (T t) { { t = T{ VALUE } }; };

template <typename T, int VALUE>
using narrow_type = std::conditional_t<narrowable<T, VALUE>, T, int>;

template <typename T>
class CharTestFixture : public ::testing::Test
{
public:
	T null_value = {};
	T zero_value = '\0';
	T a_value = 'a';
	narrow_type<T, 'long'> long_value = 'long';
	narrow_type<T, U'ą'> utf_value = U'ą';
};

using CharTypes = ::testing::Types<char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t>;
TYPED_TEST_SUITE(CharTestFixture, CharTypes);

#undef FU

#define FU(x) \
	EXPECT_EQ(ascii::isalpha(char32_t(x)), ((x) < 256) ? (bool)::isalpha(x) : false); \
	EXPECT_EQ(ascii::isdigit(char32_t(x)), ((x) < 256) ? (bool)::isdigit(x) : false); \
	EXPECT_EQ(ascii::isxdigit(char32_t(x)),((x) < 256) ? (bool)::isxdigit(x) : false); \
	EXPECT_EQ(ascii::isalnum(char32_t(x)), ((x) < 256) ? (bool)::isalnum(x) : false); \
	EXPECT_EQ(ascii::isspace(char32_t(x)), ((x) < 256) ? (bool)::isspace(x) : false); \
	EXPECT_EQ(ascii::ispunct(char32_t(x)), ((x) < 256) ? (bool)::ispunct(x) : false); \
	EXPECT_EQ(ascii::islower(char32_t(x)), ((x) < 256) ? (bool)::islower(x) : false); \
	EXPECT_EQ(ascii::isupper(char32_t(x)), ((x) < 256) ? (bool)::isupper(x) : false); \
	EXPECT_EQ(ascii::iscntrl(char32_t(x)), ((x) < 256) ? (bool)::iscntrl(x) : false); \
	EXPECT_EQ(ascii::isblank(char32_t(x)), ((x) < 256) ? (bool)::isblank(x) : false); \
	EXPECT_EQ(ascii::isgraph(char32_t(x)), ((x) < 256) ? (bool)::isgraph(x) : false); \
	EXPECT_EQ(ascii::isprint(char32_t(x)), ((x) < 256) ? (bool)::isprint(x) : false)

TYPED_TEST(CharTestFixture, ascii_works_with_all_char_types)
{
	FU(this->null_value);
	FU(this->zero_value);
	FU(this->a_value);
	FU(this->long_value);
	FU(this->utf_value);

	static_assert((sizeof(TypeParam) < 4) == std::is_same_v<decltype(this->long_value), int>);
	static_assert((sizeof(TypeParam) < 2) == std::is_same_v<decltype(this->utf_value), int>);

	EXPECT_TRUE(isascii(char32_t(this->null_value)));
	EXPECT_TRUE(isascii(char32_t(this->zero_value)));
	EXPECT_TRUE(isascii(char32_t(this->a_value)));
	EXPECT_FALSE(isascii(this->long_value)) << (int)this->long_value;
	EXPECT_FALSE(isascii(this->utf_value));
}

TEST(string_ops_test, contains_works)
{
	EXPECT_TRUE(string_contains("hello", 'e'));
	EXPECT_FALSE(string_contains("hello", 'w'));
	EXPECT_FALSE(string_contains("", 'w'));
	EXPECT_FALSE(string_contains("", '\0'));
	EXPECT_FALSE(string_contains("DAYUM", '\0'));
}

TEST(string_ops_test, make_sv_works)
{
	auto sv = make_sv(nullptr, nullptr);
	EXPECT_EQ(sv, string_view{});

	string_view svo = "hello world";
	EXPECT_EQ(make_sv(svo.data(), svo.data() + svo.size()), svo);
	EXPECT_EQ(make_sv(svo.begin(), svo.end()), svo);
}

TEST(string_ops_test, make_string_works)
{
	auto s = make_string(nullptr, nullptr);
	EXPECT_EQ(s, string_view{});

	string so = "hello world";
	EXPECT_EQ(make_string(so.data(), so.data() + so.size()), so);
	EXPECT_EQ(make_string(so.begin(), so.end()), so);
}

TEST(string_ops_test, to_string_works)
{
	{
		static_assert(std::same_as<decltype(to_string("hello")), std::string>);
		std::string s;
		static_assert(std::same_as<decltype(to_string(s)), std::string const&>);
		static_assert(std::same_as<decltype(to_string(std::move(s))), std::string>);
	}

	constexpr auto sv = std::string_view{};
	auto s = to_string(sv);
	static_assert(std::same_as<decltype(s), std::string>);
	EXPECT_EQ(s, sv);


	string const so = "hello world";
	EXPECT_EQ(to_string(so), so);
	EXPECT_EQ(to_string("hello world"), so);
}

TEST(string_ops_test, trims_work)
{
	constexpr auto base_test = "  \t\n\r\n\r\r\r \n\n\n\va0\n\n \n\tasd\n\b\v \v\t"sv;
	constexpr auto only_ws = "  \t\n\r\n\r\r\r \n\n\n\v"sv;
	EXPECT_EQ(trimmed_whitespace_left(base_test), "a0\n\n \n\tasd\n\b\v \v\t");
	EXPECT_EQ(trimmed_whitespace_right(base_test), "  \t\n\r\n\r\r\r \n\n\n\va0\n\n \n\tasd\n\b");
	EXPECT_EQ(trimmed_whitespace(base_test), "a0\n\n \n\tasd\n\b");
	EXPECT_EQ(trimmed_until(base_test, '\b'), "\b\v \v\t");

	EXPECT_EQ(trimmed_whitespace_left(only_ws), "");
	EXPECT_EQ(trimmed_whitespace_right(only_ws), "");
	EXPECT_EQ(trimmed_whitespace(only_ws), "");
	EXPECT_EQ(trimmed_until(only_ws, '\b'), "");

	EXPECT_EQ(trimmed_whitespace_left(std::string_view{}), "");
	EXPECT_EQ(trimmed_whitespace_right(std::string_view{}), "");
	EXPECT_EQ(trimmed_whitespace(std::string_view{}), "");
	EXPECT_EQ(trimmed_until(std::string_view{}, '\b'), "");

	EXPECT_EQ(trimmed_whitespace_left(std::string{}), "");
	EXPECT_EQ(trimmed_whitespace_right(std::string{}), "");
	EXPECT_EQ(trimmed_whitespace(std::string{}), "");
	EXPECT_EQ(trimmed_until(std::string{}, '\b'), "");

	auto bt_left = base_test, bt_right = base_test, bt_both = base_test;
	trim_whitespace_left(bt_left);
	trim_whitespace_right(bt_right);
	trim_whitespace(bt_both);
	EXPECT_EQ(bt_left, trimmed_whitespace_left(base_test));
	EXPECT_EQ(bt_right, trimmed_whitespace_right(base_test));
	EXPECT_EQ(bt_both, trimmed_whitespace(base_test));
	
	EXPECT_EQ(trimmed_while(base_test, [](auto cp) { return cp != U'\b'; }), "\b\v \v\t");
}

TEST(string_ops_test, utf8_to_16_converting_works)
{
	std::u8string utf8 = u8"zażółć gęślą jaźń";
	std::u16string utf16 = u"zażółć gęślą jaźń";

	EXPECT_TRUE(to_utf8<std::u8string>(utf16) == utf8);
	EXPECT_TRUE(to_utf16<std::u16string>(utf8) == utf16);
}


TEST(string_ops_test, split_functions_are_correct)
{
	EXPECT_EQ(split("hello world ", ' '), (std::vector<std::string_view>{"hello"sv, "world"sv, ""sv}));
	EXPECT_EQ(split("hello world ", "ll"), (std::vector<std::string_view>{"he"sv, "o world "sv}));
	EXPECT_EQ(split("", ' '), (std::vector<std::string_view>{""}));
	EXPECT_EQ(split("asd", ' '), (std::vector<std::string_view>{"asd"}));
	//EXPECT_EQ(ghassanpl::to<std::vector<std::string_view>>(std::ranges::split_view(std::string_view{ "hello world " }, ' ')), (std::vector<std::string_view>{"hello"sv, "world"sv, ""sv}));

	EXPECT_EQ(split_on_any("hello world ", "od"), (std::vector<std::string_view>{"hell"sv, " w"sv, "rl"sv, " "sv}));
	EXPECT_EQ(split_on_any("hello world ", ""), (std::vector<std::string_view>{}));
	EXPECT_EQ(split_on_any("", " "), (std::vector<std::string_view>{""}));
	EXPECT_EQ(split_on_any("asd", " "), (std::vector<std::string_view>{"asd"}));

	/// TODO: split_on, natural_split
}

TEST(split_on, works)
{
	std::vector<string> results;
	split_on("alpha,beta;gamma", [](auto sv) { return sv.find_first_of(",;"); }, [&](auto sv, bool) { results.push_back(string{ sv }); });
	ASSERT_EQ(results.size(), 3);
	EXPECT_EQ(results[0], "alpha");
	EXPECT_EQ(results[1], "beta");
	EXPECT_EQ(results[2], "gamma");
}

TEST(string_ops_test, join_functions_are_correct)
{
	EXPECT_EQ("hello world", join(std::vector<std::string_view>{"hello"sv, "world"sv}, " "));
}

TEST(string_ops_test, transcode)
{
	static const std::array<char32_t, 128> win1250 = {
		U'€',	U' ', 	U'‚', 	U' ', 	U'„', 	U'…', 	U'†', 	U'‡', 	U' ', 	U'‰', 	U'Š', 	U'‹', 	U'Ś', 	U'Ť', 	U'Ž', 	U'Ź',
		U' ',	U'‘', 	U'’', 	U'“', 	U'”', 	U'•', 	U'–', 	U'—', 	U' ', 	U'™', 	U'š', 	U'›', 	U'ś', 	U'ť', 	U'ž', 	U'ź',
		0xA0,	U'ˇ', 	U'˘', 	U'Ł', 	U'¤', 	U'Ą', 	U'¦', 	U'§', 	U'¨', 	U'©', 	U'Ş', 	U'«', 	U'¬', 	U'-', 	U'®', 	U'Ż',
		U'°', 	U'±', 	U'˛', 	U'ł', 	U'´', 	U'µ', 	U'¶', 	U'·', 	U'¸', 	U'ą', 	U'ş', 	U'»', 	U'Ľ', 	U'˝', 	U'ľ', 	U'ż',
		U'Ŕ', 	U'Á', 	U'Â', 	U'Ă', 	U'Ä', 	U'Ĺ', 	U'Ć', 	U'Ç', 	U'Č', 	U'É', 	U'Ę', 	U'Ë', 	U'Ě', 	U'Í', 	U'Î', 	U'Ď',
		U'Đ', 	U'Ń', 	U'Ň', 	U'Ó', 	U'Ô', 	U'Ő', 	U'Ö', 	U'×', 	U'Ř', 	U'Ů', 	U'Ú', 	U'Ű', 	U'Ü', 	U'Ý', 	U'Ţ', 	U'ß',
		U'ŕ', 	U'á', 	U'â', 	U'ă', 	U'ä', 	U'ĺ', 	U'ć', 	U'ç', 	U'č', 	U'é', 	U'ę', 	U'ë', 	U'ě', 	U'í', 	U'î', 	U'ď',
		U'đ', 	U'ń', 	U'ň', 	U'ó', 	U'ô', 	U'ő', 	U'ö', 	U'÷', 	U'ř', 	U'ů', 	U'ú', 	U'ű', 	U'ü', 	U'ý', 	U'ţ', 	U'˙',
	};

	using namespace std::string_literals;

	const std::string str { (char)0x5A, (char)0x41, (char)0xAF, (char)0xD3, (char)0xA3, (char)0xC6, (char)0x20, (char)0x47, (char)0xCA, (char)0x8C, 
		(char)0x4C, (char)0xA5, (char)0x20, (char)0x4A, (char)0x41, (char)0x8F, (char)0xD1 };

	EXPECT_EQ(transcode_codepage_to_utf8<std::string>(str, win1250), to_string(u8"ZAŻÓŁĆ GĘŚLĄ JAŹŃ"));
}


TEST(string_ops_test, consume_bom_and_detect_encoding)
{
	{
		auto carray = std::array{ char(0xEF), char(0xBB), char(0xBF), 'h', 'e', 'l', 'l', 'o', };
		auto cspan = std::span{ carray.data(), carray.size() }; /// We need it to be a dynamic span
		auto [encoding, endianness] = consume_bom(cspan);
		EXPECT_EQ(encoding, text_encoding_type::utf8);
		EXPECT_EQ(endianness, std::endian::native);
		EXPECT_EQ(std::string_view{ cspan }, "hello"sv);
	}

	{
		std::string_view str = "hello";
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, text_encoding_type::unknown);
		EXPECT_EQ(endianness, std::endian::native);
		EXPECT_EQ(str, "hello"sv);
	}

	{
		std::string_view str = "\xEF\xBB\xBF" "hello";
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, text_encoding_type::utf8);
		EXPECT_EQ(endianness, std::endian::native);
		EXPECT_EQ(str, "hello"sv);
	}

	{
		static_assert(sizeof(char16_t) == 2);

		const char16_t bom = 0xFEFF;
		std::string hello;
		hello += std::string{ reinterpret_cast<char const*>(&bom), sizeof(bom) };
		hello += std::string{ reinterpret_cast<char const*>(u"hello"), sizeof(u"hello") - sizeof(u"")};

		std::string_view str = hello;
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, text_encoding_type::utf16);
		EXPECT_EQ(endianness, std::endian::native);

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.type, text_encoding_type::utf16);
		EXPECT_EQ(detected_encoding.endianness, std::endian::native);
	}

	{
		static_assert(sizeof(char32_t) == 4);

		const char32_t bom = 0xFEFF;
		std::string hello;
		hello += std::string{ reinterpret_cast<char const*>(&bom), sizeof(bom) };
		hello += std::string{ reinterpret_cast<char const*>(U"hello"), sizeof(U"hello") - sizeof(U"") };

		std::string_view str = hello;
		auto bom_encoding = consume_bom(str);
		EXPECT_EQ(bom_encoding.type, text_encoding_type::utf32);
		EXPECT_EQ(bom_encoding.endianness, std::endian::native);

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.type, text_encoding_type::utf32);
		EXPECT_EQ(detected_encoding.endianness, std::endian::native);
		//EXPECT_EQ(str, "hello"sv);
	}

	/// not native endianness
	{
		static_assert(sizeof(char16_t) == 2);

		constexpr char16_t bom = std::byteswap(char16_t(0xFEFF));
		std::string hello;
		hello += std::string{ reinterpret_cast<char const*>(&bom), sizeof(bom) };
		hello += '\0';
		hello += std::string{ reinterpret_cast<char const*>(u"hello"), sizeof(u"hello") - sizeof(u"") };
		hello.pop_back();

		std::string_view str = hello;
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, text_encoding_type::utf16);
		EXPECT_EQ(endianness, std::endian(!(int)std::endian::native));

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.type, text_encoding_type::utf16);
		EXPECT_EQ(detected_encoding.endianness, std::endian(!(int)std::endian::native));
	}

	{
		static_assert(sizeof(char32_t) == 4);

		constexpr char32_t bom = std::byteswap(char32_t(0xFEFF));
		std::string hello;
		hello += std::string{ reinterpret_cast<char const*>(&bom), sizeof(bom) };
		hello += '\0';
		hello += '\0';
		hello += '\0';
		hello += std::string{ reinterpret_cast<char const*>(U"hello"), sizeof(U"hello") - sizeof(U"") };
		hello.pop_back();
		hello.pop_back();
		hello.pop_back();

		std::string_view str = hello;
		auto bom_encoding = consume_bom(str);
		EXPECT_EQ(bom_encoding.type, text_encoding_type::utf32);
		EXPECT_EQ(bom_encoding.endianness, std::endian(!(int)std::endian::native));

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.type, text_encoding_type::utf32);
		EXPECT_EQ(detected_encoding.endianness, std::endian(!(int)std::endian::native));
		//EXPECT_EQ(str, "hello"sv);
	}

	{
		decltype(auto) str = "hello world";
		static_assert(ghassanpl::bytelike_range<decltype(str)>);
		auto [encoding, endianness] = detect_encoding("hello world");
		EXPECT_EQ(encoding, text_encoding_type::utf8);
		EXPECT_EQ(endianness, std::endian::native);
	}
}

TEST(stringification_test, sanity_check)
{
	const ghassanpl::trec2<float> val{0,10,20,30};
	const auto stringified = ghassanpl::to_string(val);
	EXPECT_EQ(stringified, "[0,10,20,30]");
	
	ghassanpl::trec2<float> unstringified{};
	EXPECT_TRUE(ghassanpl::from_string<ghassanpl::trec2<float>>(stringified, unstringified));
	EXPECT_EQ(unstringified, val);
}

TEST(string_ops, any_versions)
{
	{
		auto sv = "hello"sv;
		auto ret = consume_any(sv, 'a', "he");
		EXPECT_EQ(ret, 'h');
		EXPECT_EQ(sv, "ello"sv);
	}
	{
		auto sv = "hello"sv;
		auto ret = consume_while_any(sv, "eh", 'l');
		EXPECT_EQ(ret, "hell"sv);
		EXPECT_EQ(sv, "o"sv);
	}

	EXPECT_FALSE(isany(char32_t(500), -1));
}

TEST(consume_until, char_delimiter)
{
	/// Delimiter in the middle: stops before it and leaves it at the front of the view.
	{
		auto sv = "abc,def"sv;
		auto ret = consume_until(sv, ',');
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, ",def"sv);
	}
	/// Delimiter absent: consumes the whole string.
	{
		auto sv = "abcdef"sv;
		auto ret = consume_until(sv, ',');
		EXPECT_EQ(ret, "abcdef"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Delimiter at the very start: consumes nothing.
	{
		auto sv = ",abc"sv;
		auto ret = consume_until(sv, ',');
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, ",abc"sv);
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_until(sv, ',');
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_until, string_delimiter)
{
	/// Needle in the middle: stops before it and leaves it at the front of the view.
	{
		auto sv = "abcXYdef"sv;
		auto ret = consume_until(sv, "XY"sv);
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, "XYdef"sv);
	}
	/// Needle absent: consumes the whole string.
	{
		auto sv = "abcdef"sv;
		auto ret = consume_until(sv, "XY"sv);
		EXPECT_EQ(ret, "abcdef"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Needle at the start: consumes nothing.
	{
		auto sv = "XYabc"sv;
		auto ret = consume_until(sv, "XY"sv);
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "XYabc"sv);
	}
	/// A partial match ("X") must not trigger; only the full needle does.
	{
		auto sv = "aXbXYc"sv;
		auto ret = consume_until(sv, "XY"sv);
		EXPECT_EQ(ret, "aXb"sv);
		EXPECT_EQ(sv, "XYc"sv);
	}
	/// Empty needle: consumes nothing.
	{
		auto sv = "abc"sv;
		auto ret = consume_until(sv, ""sv);
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "abc"sv);
	}
}

TEST(consume_until, predicate)
{
	const auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

	/// Predicate matches in the middle: stops before the first matching char.
	{
		auto sv = "abc123"sv;
		auto ret = consume_until(sv, is_digit);
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, "123"sv);
	}
	/// No char matches: consumes the whole string.
	{
		auto sv = "abc"sv;
		auto ret = consume_until(sv, is_digit);
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// First char already matches: consumes nothing.
	{
		auto sv = "123"sv;
		auto ret = consume_until(sv, is_digit);
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "123"sv);
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_until(sv, is_digit);
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_while, char_run)
{
	/// Run at the start: consumed up to the first non-matching char.
	{
		auto sv = "aaabbb"sv;
		auto ret = consume_while(sv, 'a');
		EXPECT_EQ(ret, "aaa"sv);
		EXPECT_EQ(sv, "bbb"sv);
	}
	/// First char doesn't match: consumes nothing.
	{
		auto sv = "bbb"sv;
		auto ret = consume_while(sv, 'a');
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "bbb"sv);
	}
	/// All chars match: consumes the whole string.
	{
		auto sv = "aaa"sv;
		auto ret = consume_while(sv, 'a');
		EXPECT_EQ(ret, "aaa"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_while(sv, 'a');
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_while, predicate)
{
	const auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

	{
		auto sv = "123abc"sv;
		auto ret = consume_while(sv, is_digit);
		EXPECT_EQ(ret, "123"sv);
		EXPECT_EQ(sv, "abc"sv);
	}
	{
		auto sv = "abc"sv;
		auto ret = consume_while(sv, is_digit);
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "abc"sv);
	}
	{
		auto sv = "123"sv;
		auto ret = consume_while(sv, is_digit);
		EXPECT_EQ(ret, "123"sv);
		EXPECT_TRUE(sv.empty());
	}
	{
		auto sv = ""sv;
		auto ret = consume_while(sv, is_digit);
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_while_any, char_fast_path)
{
	/// All-`char` arguments take the find_first_not_of fast path.
	{
		auto sv = "abba c"sv;
		auto ret = consume_while_any(sv, 'a', 'b');
		EXPECT_EQ(ret, "abba"sv);
		EXPECT_EQ(sv, " c"sv);
	}
	{
		auto sv = "xyz"sv;
		auto ret = consume_while_any(sv, 'a', 'b');
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "xyz"sv);
	}
	{
		auto sv = "abab"sv;
		auto ret = consume_while_any(sv, 'a', 'b');
		EXPECT_EQ(ret, "abab"sv);
		EXPECT_TRUE(sv.empty());
	}
	{
		auto sv = ""sv;
		auto ret = consume_while_any(sv, 'a', 'b');
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_while_any, general_path)
{
	/// A string-like argument takes the isany-based general path.
	{
		auto sv = "abba c"sv;
		auto ret = consume_while_any(sv, "ab"sv);
		EXPECT_EQ(ret, "abba"sv);
		EXPECT_EQ(sv, " c"sv);
	}
	/// Mixed string and char arguments.
	{
		auto sv = "hello"sv;
		auto ret = consume_while_any(sv, "eh", 'l');
		EXPECT_EQ(ret, "hell"sv);
		EXPECT_EQ(sv, "o"sv);
	}
}

TEST(consume_until_any, char_fast_path)
{
	/// All-`char` arguments take the find_first_of fast path.
	{
		auto sv = "abc;def"sv;
		auto ret = consume_until_any(sv, ',', ';');
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, ";def"sv);
	}
	/// No delimiter found: consumes the whole string.
	{
		auto sv = "abc"sv;
		auto ret = consume_until_any(sv, ',', ';');
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Delimiter at the start: consumes nothing.
	{
		auto sv = ";abc"sv;
		auto ret = consume_until_any(sv, ',', ';');
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, ";abc"sv);
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_until_any(sv, ',', ';');
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_until_any, general_path)
{
	/// A string-like argument takes the isany-based general path.
	{
		auto sv = "abc;def"sv;
		auto ret = consume_until_any(sv, ",;"sv);
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, ";def"sv);
	}
	/// Mixed string and char arguments.
	{
		auto sv = "abc,def"sv;
		auto ret = consume_until_any(sv, ";"sv, ',');
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, ",def"sv);
	}
}

TEST(consume_until_delim, works)
{
	/// The delimiter is consumed AND included in the result.
	{
		auto sv = "abc,def"sv;
		auto ret = consume_until_delim(sv, ',');
		EXPECT_EQ(ret, "abc,"sv);
		EXPECT_EQ(sv, "def"sv);
	}
	/// No delimiter: consumes and returns the whole string.
	{
		auto sv = "abcdef"sv;
		auto ret = consume_until_delim(sv, ',');
		EXPECT_EQ(ret, "abcdef"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Delimiter at the start: result is just the delimiter.
	{
		auto sv = ",abc"sv;
		auto ret = consume_until_delim(sv, ',');
		EXPECT_EQ(ret, ","sv);
		EXPECT_EQ(sv, "abc"sv);
	}
	/// Delimiter at the end.
	{
		auto sv = "abc,"sv;
		auto ret = consume_until_delim(sv, ',');
		EXPECT_EQ(ret, "abc,"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_until_delim(sv, ',');
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_until_delim_ex, works)
{
	/// The delimiter is consumed but NOT included in the result.
	{
		auto sv = "abc,def"sv;
		auto ret = consume_until_delim_ex(sv, ',');
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, "def"sv);
	}
	/// No delimiter: consumes and returns the whole string.
	{
		auto sv = "abcdef"sv;
		auto ret = consume_until_delim_ex(sv, ',');
		EXPECT_EQ(ret, "abcdef"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Delimiter at the start: empty result, delimiter skipped.
	{
		auto sv = ",abc"sv;
		auto ret = consume_until_delim_ex(sv, ',');
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "abc"sv);
	}
	/// Delimiter at the end.
	{
		auto sv = "abc,"sv;
		auto ret = consume_until_delim_ex(sv, ',');
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_until_delim_ex(sv, ',');
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
	/// The result aliases the source buffer (it's a subview, not a copy).
	{
		auto src = "abc,def"sv;
		auto sv = src;
		auto ret = consume_until_delim_ex(sv, ',');
		EXPECT_EQ(ret.data(), src.data());
		EXPECT_EQ(sv.data(), src.data() + 4);
	}
}

TEST(consume_n, unconditional)
{
	{
		auto sv = "abcdef"sv;
		auto ret = consume_n(sv, 3);
		EXPECT_EQ(ret, "abc"sv);
		EXPECT_EQ(sv, "def"sv);
	}
	/// n greater than size: consumes everything available.
	{
		auto sv = "ab"sv;
		auto ret = consume_n(sv, 5);
		EXPECT_EQ(ret, "ab"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// n == 0: consumes nothing.
	{
		auto sv = "abc"sv;
		auto ret = consume_n(sv, 0);
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "abc"sv);
	}
	{
		auto sv = ""sv;
		auto ret = consume_n(sv, 3);
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(consume_n, with_predicate)
{
	const auto is_digit = [](char c) { return c >= '0' && c <= '9'; };

	/// Stops at n even though more chars match.
	{
		auto sv = "12345abc"sv;
		auto ret = consume_n(sv, 3, is_digit);
		EXPECT_EQ(ret, "123"sv);
		EXPECT_EQ(sv, "45abc"sv);
	}
	/// Predicate stops before n is exhausted.
	{
		auto sv = "12ab"sv;
		auto ret = consume_n(sv, 3, is_digit);
		EXPECT_EQ(ret, "12"sv);
		EXPECT_EQ(sv, "ab"sv);
	}
	/// n greater than size, all matching.
	{
		auto sv = "12"sv;
		auto ret = consume_n(sv, 5, is_digit);
		EXPECT_EQ(ret, "12"sv);
		EXPECT_TRUE(sv.empty());
	}
	/// First char doesn't match: consumes nothing.
	{
		auto sv = "ab"sv;
		auto ret = consume_n(sv, 3, is_digit);
		EXPECT_EQ(ret, ""sv);
		EXPECT_EQ(sv, "ab"sv);
	}
	/// Empty input.
	{
		auto sv = ""sv;
		auto ret = consume_n(sv, 3, is_digit);
		EXPECT_EQ(ret, ""sv);
		EXPECT_TRUE(sv.empty());
	}
}

TEST(string_ops, substr_functions_work)
{
	auto sv = "0123456789"sv;

	EXPECT_EQ(prefix(sv, 4), "0123"sv);
	EXPECT_EQ(suffix(sv, 6), "456789"sv);
	EXPECT_EQ(without_prefix(sv, 4), "456789"sv);
	EXPECT_EQ(without_suffix(sv, 6), "0123"sv);

	EXPECT_EQ(prefix(sv, 0), ""sv);
	EXPECT_EQ(suffix(sv, 0), ""sv);
	EXPECT_EQ(without_prefix(sv, 0), sv);
	EXPECT_EQ(without_suffix(sv, 0), sv);

	EXPECT_EQ(prefix(sv, 1000), sv);
	EXPECT_EQ(suffix(sv, 1000), sv);
	EXPECT_EQ(without_prefix(sv, 1000), ""sv);
	EXPECT_EQ(without_suffix(sv, 1000), ""sv);

	{
		std::string s = "0123456789";
		erase_outside_n(s, 4, 3);
		EXPECT_EQ(s, "456");
	}
	{
		std::string s = "0123456789";
		erase_outside_from_to(s, 4, 7);
		EXPECT_EQ(s, "456");
	}
	{
		std::string s = "0123456789";
		erase_outside_from_to(s, 7, 4);
		EXPECT_EQ(s, "456");
	}
}

TEST(string_ops, split_range_works)
{
	std::vector<std::string_view> split;
	for (auto word : split_range{ "hello world, this is a very long string                  many", " " })
		split.push_back(word);

	ASSERT_EQ(split.size(), 9);
	EXPECT_EQ(split[0], "hello");
	EXPECT_EQ(split[1], "world,");
	EXPECT_EQ(split[2], "this");
	EXPECT_EQ(split[3], "is");
	EXPECT_EQ(split[4], "a");
	EXPECT_EQ(split[5], "very");
	EXPECT_EQ(split[6], "long");
	EXPECT_EQ(split[7], "string");
	EXPECT_EQ(split[8], "many");
}


TEST(string_ops, word_wrap_works)
{
	const auto split = word_wrap("hello\n      world my dear? dear ? ", 100ULL, [](std::string_view getter) { return std::size(getter) * 7; });

	ASSERT_EQ(split.size(), 3);
	EXPECT_EQ(split[0], "hello");
	EXPECT_EQ(split[1], "      world my ");
	EXPECT_EQ(split[2], "dear? dear ? ");
}

TEST(join_and, works_for_mutating_views)
{
	string_view sv = "hello!";
	EXPECT_EQ(join_and(sv | std::views::filter(ascii::islower), ", ", ", and "), "h, e, l, l, and o");
}

TEST(to_utf16, works_for_surrogates)
{
	EXPECT_EQ(to_utf16(char32_t(0x1F603)), (std::wstring{ 0xD83D, 0xDE03 }));
}

TEST(is_inside, works)
{
	const string_view sv = "hello world";
	const string_view sv1 = sv.substr(5);
	const string_view sv2 = sv.substr(0, 5);
	const string_view sv3 = "asdf";
	const string_view sv4 = sv.substr(3, 6);
	EXPECT_TRUE(is_inside(sv, sv1));
	EXPECT_TRUE(is_inside(sv, sv2));
	EXPECT_FALSE(is_inside(sv, sv3));
	EXPECT_TRUE(is_inside(sv, sv4));
}
