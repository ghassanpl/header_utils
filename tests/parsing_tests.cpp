/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/parsing.h"
#include "../include/ghassanpl/wilson.h"
#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_view_literals;

TEST(parsing_functions, basics)
{
}

constexpr auto wilson_decade = R"([ChangeImageOf [this] ToTile chest_open]
[Play chest_open At [here] Waiting false]
[AddLog [Phrase OpenChest FromGroup logs]]
Once)"sv;

TEST(parsing_functions, wilson_parsing_basic_decade)
{
	auto parsed = formats::wilson::parse_array(wilson_decade).value();
	//std::cout << parsed.dump(1) << "\n";
	EXPECT_EQ(parsed.size(), 4);
}

TEST(parsing_functions, consume_c_string_basics)
{
	/// Simple string with the default single-quote delimiter.
	{
		auto sv = "'hello'"sv;
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "hello");   /// .second is the decoded content
		EXPECT_EQ(r.first, "'hello'");  /// .first is the raw source span, delimiters included
		EXPECT_TRUE(sv.empty());        /// the input view is advanced past the literal
	}

	/// Empty string literal.
	{
		auto sv = "''"sv;
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "");
		EXPECT_EQ(r.first, "''");
		EXPECT_TRUE(sv.empty());
	}

	/// Only the literal is consumed; trailing text is left in place.
	{
		auto sv = "'ab' tail"sv;
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "ab");
		EXPECT_EQ(sv, " tail");
	}
}

TEST(parsing_functions, consume_c_string_escapes)
{
	/// All the simple single-character escapes.
	{
		auto sv = R"('a\nb\tc\\d\'e\"f\r\b\f')"sv;
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "a\nb\tc\\d'e\"f\r\b\f");
		EXPECT_TRUE(sv.empty());
	}

	/// \0 produces an embedded NUL rather than terminating the result.
	{
		auto sv = R"('\0')"sv;
		const auto r = parsing::consume_c_string(sv);
		ASSERT_EQ(r.second.size(), 1u);
		EXPECT_EQ(r.second[0], '\0');
	}

	/// Hex escapes.
	{
		auto sv = R"('\x41\x42')"sv;
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "AB");
	}

	/// Octal escape (\oNNN); 0101 octal == 65 == 'A'.
	{
		auto sv = R"('\o101')"sv;
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "A");
	}

	/// Unicode escapes are re-encoded as UTF-8. (Non-raw literals with a doubled
	/// backslash, so the bytes handed to the parser are really '\','u',... - inside a
	/// raw string the compiler could fold \u/\U into a universal character name.)
	{
		auto sv = "'\\u0041'"sv;             // U+0041 'A'
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "A");
	}
	{
		auto sv = "'\\u00e9'"sv;             // U+00E9 'é'
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "\xC3\xA9");
	}
	{
		auto sv = "'\\U0001F600'"sv;         // U+1F600 grinning face
		const auto r = parsing::consume_c_string(sv);
		EXPECT_EQ(r.second, "\xF0\x9F\x98\x80");
	}
}

TEST(parsing_functions, consume_c_string_custom_delimiter)
{
	/// A double-quote delimiter, with the delimiter itself escaped inside.
	auto sv = R"("he said \"hi\"")"sv;
	const auto r = parsing::consume_c_string<'"'>(sv);
	EXPECT_EQ(r.second, "he said \"hi\"");
	EXPECT_TRUE(sv.empty());
}

TEST(parsing_functions, consume_c_string_errors)
{
	/// Missing opening delimiter.
	{
		auto sv = "hello"sv;
		EXPECT_THROW((void)parsing::consume_c_string(sv), std::runtime_error);
	}
	/// Unterminated (no closing delimiter).
	{
		auto sv = "'hello"sv;
		EXPECT_THROW((void)parsing::consume_c_string(sv), std::runtime_error);
	}
	/// Unknown escape character.
	{
		auto sv = R"('\q')"sv;
		EXPECT_THROW((void)parsing::consume_c_string(sv), std::runtime_error);
	}
}