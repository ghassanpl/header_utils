/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/unicode.h"

#include <string>
#include <string_view>
#include <span>
#include <cstdio>
#include <gtest/gtest.h>
#undef isascii

using namespace ghassanpl::string_ops;
using std::string;
using std::string_view;
using namespace std::string_view_literals;

namespace
{
	/// UTF-8 byte sequences written as adjacent `"\xNN"` string literals. Each hex escape
	/// closes at its own quote, so nothing can absorb a following hex digit, and the
	/// concatenation is a single compile-time constant - hence everything below is constexpr.
	/// Composite sequences are built by placing these macros next to each other.
	#define U8_A        "\x41"                       // U+0041  'A'      (1 byte)
	#define U8_E_ACUTE  "\xC3" "\xA9"                // U+00E9  'é'      (2 bytes)
	#define U8_EURO     "\xE2" "\x82" "\xAC"         // U+20AC  '€'      (3 bytes)
	#define U8_GRIN     "\xF0" "\x9F" "\x98" "\x80"  // U+1F600 '😀'     (4 bytes)

	constexpr string_view A       = U8_A;
	constexpr string_view e_acute = U8_E_ACUTE;
	constexpr string_view euro    = U8_EURO;
	constexpr string_view grin    = U8_GRIN;

	struct Case
	{
		string_view in;
		string_view expected;
	};

	string hex(string_view s)
	{
		return join(s, ' ', [](char c) { return std::format("{:02x}", uint8_t(c)); });
	}

	
	void run(std::span<const Case> cases)
	{
		for (auto const& c : cases)
		{
			const string_view result = trimmed_to_codepoint_boundaries(c.in);
			EXPECT_EQ(result, c.expected)
				<< "input   = " << hex(c.in) << "\n"
				<< "trimmed = " << hex(result) << "\n"
				<< "wanted  = " << hex(c.expected);
			

			/// The post-condition of trimming: the view is empty, or it starts on a codepoint
			/// boundary (an introducer byte) and its last codepoint is complete.
			if (!result.empty())
			{
				EXPECT_TRUE(is_introducer(result.front())) << "does not start on a boundary: " << hex(result);
				const size_t last = last_codepoint_size(result);
				const auto introducer = static_cast<unsigned char>(result[result.size() - last]);
				EXPECT_EQ(size_t(octet_count_for_introducer(introducer)), last)
					<< "last codepoint is incomplete: " << hex(result);
			}

			// Idempotence: trimming an already-trimmed view changes nothing.
			const string_view again = trimmed_to_codepoint_boundaries(result);
			EXPECT_EQ(again, result) << "not idempotent for input " << hex(c.in);
		}
	}
}

TEST(unicode_trim, empty_stays_empty)
{
	constexpr Case cases[] = { { ""sv, ""sv } };
	run(cases);
}

TEST(unicode_trim, complete_strings_are_unchanged)
{
	constexpr Case cases[] = {
		{ A, A },
		{ "Hello, world!"sv, "Hello, world!"sv },
		{ e_acute, e_acute },
		{ euro, euro },
		{ grin, grin },
		{ U8_A U8_E_ACUTE U8_EURO U8_GRIN, U8_A U8_E_ACUTE U8_EURO U8_GRIN },
		{ U8_GRIN U8_GRIN, U8_GRIN U8_GRIN },
	};
	run(cases);
}

TEST(unicode_trim, leading_partial_codepoint_is_removed)
{
	constexpr Case cases[] = {
		{ "\x82" "\xAC", "" },                 // trailing 2 bytes of €
		{ "\x82" "\xAC" U8_A, A },             // ...followed by a full codepoint
		{ "\x9F" "\x98" "\x80", "" },          // trailing 3 bytes of 😀
		{ "\x9F" "\x98" "\x80" U8_A, A },
		{ "\xA9" U8_A, A },                     // trailing byte of é
		{ "\x80" "\x80" "\x80", "" },          // nothing but continuation bytes
	};
	run(cases);
}

TEST(unicode_trim, trailing_partial_codepoint_is_removed)
{
	constexpr Case cases[] = {
		{ "\xE2" "\x82", "" },                 // first 2 bytes of €
		{ U8_A "\xE2" "\x82", A },
		{ U8_A "\xF0" "\x9F" "\x98", A },      // 'A' + first 3 bytes of 😀
		{ "\xF0", "" },                        // lone 4-byte introducer
		{ "\xE2", "" },                        // lone 3-byte introducer
		{ U8_A "\xC3", A },                    // 'A' + first byte of é
	};
	run(cases);
}

TEST(unicode_trim, both_ends_partial)
{
	constexpr Case cases[] = {
		// trailing-of-€ + 'A' + first-2-of-😀  ->  "A"
		{ "\x82" "\xAC" U8_A "\xF0" "\x9F", A },
		// junk continuations + complete € + a lone 3-byte introducer  ->  "€"
		{ "\x9F" "\x80" U8_EURO "\xE2", euro },
	};
	run(cases);
}

TEST(unicode_trim, in_place_overload_adjusts_view_in_buffer)
{
	constexpr string_view src = "\x82" "\xAC" U8_A "\xF0" "\x9F"sv; // «tail of €» + 'A' + «first 2 of 😀»
	string_view sv = src;
	trim_to_codepoint_boundaries(sv);

	EXPECT_EQ(sv, A);
	// The result must be a sub-view of the original storage, not a copy.
	EXPECT_GE(sv.data(), src.data());
	EXPECT_LE(sv.data() + sv.size(), src.data() + src.size());
}

TEST(unicode_trim, trimmed_overload_does_not_touch_its_argument)
{
	constexpr string_view src = U8_A "\xF0" "\x9F" "\x98"sv; // 'A' + truncated 😀
	string_view arg = src;

	const string_view result = trimmed_to_codepoint_boundaries(arg);

	EXPECT_EQ(arg, src) << "by-value overload must not modify the caller's view";
	EXPECT_EQ(result, A);
}

#undef U8_A
#undef U8_E_ACUTE
#undef U8_EURO
#undef U8_GRIN
