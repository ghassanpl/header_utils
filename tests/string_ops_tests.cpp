/// This Source Code Form is subject to the terms of the Mozilla Public
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
	T empty_string_value = "";
	T complex_value = "ZCoo(01_;";
	T embedded_zeroes_value = "asdf\0ZXCV";
};

using StringableTypes = ::testing::Types<char const*, std::string_view, const char(&)[10], std::string>;
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
	narrow_type<T, 'ą'> utf_value = 'ą';
};

using CharTypes = ::testing::Types<char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t>;
TYPED_TEST_SUITE(CharTestFixture, CharTypes);

#undef FU

#define FU(x) \
	EXPECT_EQ(ascii::isalpha(x), (x < 256) ? (bool)::isalpha(x) : false); \
	EXPECT_EQ(ascii::isdigit(x), (x < 256) ? (bool)::isdigit(x) : false); \
	EXPECT_EQ(ascii::isxdigit(x),(x < 256) ? (bool)::isxdigit(x) : false); \
	EXPECT_EQ(ascii::isalnum(x), (x < 256) ? (bool)::isalnum(x) : false); \
	EXPECT_EQ(ascii::isspace(x), (x < 256) ? (bool)::isspace(x) : false); \
	EXPECT_EQ(ascii::ispunct(x), (x < 256) ? (bool)::ispunct(x) : false); \
	EXPECT_EQ(ascii::islower(x), (x < 256) ? (bool)::islower(x) : false); \
	EXPECT_EQ(ascii::isupper(x), (x < 256) ? (bool)::isupper(x) : false); \
	EXPECT_EQ(ascii::iscntrl(x), (x < 256) ? (bool)::iscntrl(x) : false); \
	EXPECT_EQ(ascii::isblank(x), (x < 256) ? (bool)::isblank(x) : false); \
	EXPECT_EQ(ascii::isgraph(x), (x < 256) ? (bool)::isgraph(x) : false); \
	EXPECT_EQ(ascii::isprint(x), (x < 256) ? (bool)::isprint(x) : false); \

TYPED_TEST(CharTestFixture, ascii_works_with_all_char_types)
{
	FU(this->null_value);
	FU(this->zero_value);
	FU(this->a_value);
	FU(this->long_value);
	FU(this->utf_value);

	static_assert((sizeof(TypeParam) < 4) == std::is_same_v<decltype(this->long_value), int>);
	static_assert((sizeof(TypeParam) < 2) == std::is_same_v<decltype(this->utf_value), int>);

	EXPECT_TRUE(isascii(this->null_value));
	EXPECT_TRUE(isascii(this->zero_value));
	EXPECT_TRUE(isascii(this->a_value));
	EXPECT_FALSE(isascii(this->long_value)) << (int)this->long_value;
	EXPECT_FALSE(isascii(this->utf_value));
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
	auto sv = std::string_view{};
	auto s = to_string(sv);
	EXPECT_EQ(s, sv);

	string so = "hello world";
	EXPECT_EQ(to_string(so), so);
	EXPECT_EQ(to_string("hello world"), so);
}

TEST(string_ops_test, trims_work)
{
	const auto base_test = "  \t\n\r\n\r\r\r \n\n\n\va0\n\n \n\tasd\n\b\v \v\t"sv;
	const auto only_ws = "  \t\n\r\n\r\r\r \n\n\n\v"sv;
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

TEST(string_ops_test, join_functions_are_correct)
{
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
		std::string_view str = "hello";
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, base_text_encoding::unknown);
		EXPECT_EQ(endianness, std::endian::native);
		EXPECT_EQ(str, "hello"sv);
	}

	{
		std::string_view str = "\xEF\xBB\xBFhello";
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, base_text_encoding::utf8);
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
		EXPECT_EQ(encoding, base_text_encoding::utf16);
		EXPECT_EQ(endianness, std::endian::native);

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.base_encoding, base_text_encoding::utf16);
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
		EXPECT_EQ(bom_encoding.base_encoding, base_text_encoding::utf32);
		EXPECT_EQ(bom_encoding.endianness, std::endian::native);

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.base_encoding, base_text_encoding::utf32);
		EXPECT_EQ(detected_encoding.endianness, std::endian::native);
		//EXPECT_EQ(str, "hello"sv);
	}

	/// not native endianness
	{
		static_assert(sizeof(char16_t) == 2);

		const char16_t bom = std::byteswap(char16_t(0xFEFF));
		std::string hello;
		hello += std::string{ reinterpret_cast<char const*>(&bom), sizeof(bom) };
		hello += '\0';
		hello += std::string{ reinterpret_cast<char const*>(u"hello"), sizeof(u"hello") - sizeof(u"") };
		hello.pop_back();

		std::string_view str = hello;
		auto [encoding, endianness] = consume_bom(str);
		EXPECT_EQ(encoding, base_text_encoding::utf16);
		EXPECT_EQ(endianness, std::endian(!(int)std::endian::native));

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.base_encoding, base_text_encoding::utf16);
		EXPECT_EQ(detected_encoding.endianness, std::endian(!(int)std::endian::native));
	}

	{
		static_assert(sizeof(char32_t) == 4);

		const char32_t bom = std::byteswap(char32_t(0xFEFF));
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
		EXPECT_EQ(bom_encoding.base_encoding, base_text_encoding::utf32);
		EXPECT_EQ(bom_encoding.endianness, std::endian(!(int)std::endian::native));

		auto detected_encoding = detect_encoding(str);
		EXPECT_EQ(detected_encoding.base_encoding, base_text_encoding::utf32);
		EXPECT_EQ(detected_encoding.endianness, std::endian(!(int)std::endian::native));
		//EXPECT_EQ(str, "hello"sv);
	}

	{
		auto [encoding, endianness] = detect_encoding("hello world");
		EXPECT_EQ(encoding, base_text_encoding::utf8);
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