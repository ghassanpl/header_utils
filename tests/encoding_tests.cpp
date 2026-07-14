/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/encoding.h"

#include <algorithm>
#include <array>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

/// NOTE: Unlike the header itself, these tests do assume an ASCII-based execution charset - there is no way to
/// write readable test vectors otherwise.

using namespace ghassanpl;
using namespace std::string_view_literals;

namespace
{
	/// Whether `alphabet` can be used to encode a base-N number - that is, whether it consists of distinct ASCII characters.
	/// This used to live in `ghassanpl::detail`, but nothing in the header needs it at runtime - it only ever guarded the
	/// alphabets, and that is a thing for the tests to do.
	template <size_t N>
	[[nodiscard]] constexpr bool is_valid_alphabet(std::array<char, N> const& alphabet) noexcept
	{
		for (size_t i = 0; i < N; ++i)
		{
			if (static_cast<unsigned char>(alphabet[i]) > 127) /// not an ASCII character
				return false;
			for (size_t j = i + 1; j < N; ++j)
				if (alphabet[i] == alphabet[j]) /// a character cannot represent two different digits
					return false;
		}
		return true;
	}

	/// Turns decoded bytes back into a string, so that tests can compare them against string literals
	template <bytelike B>
	[[nodiscard]] std::string as_str(std::vector<B> const& bytes)
	{
		std::string result;
		for (auto const byte : bytes)
			result += to_char(byte);
		return result;
	}

	/// A fixed set of byte sequences to run the encodings over: every length up to 32 (so that every possible
	/// partial group is covered), the edge cases, and every byte value
	[[nodiscard]] std::vector<std::vector<uint8_t>> const& test_data()
	{
		static const std::vector<std::vector<uint8_t>> data = [] {
			std::vector<std::vector<uint8_t>> result = {
				{}, { 0 }, { 255 }, { 0, 0, 0, 0 }, { 255, 255, 255, 255 }, { 0, 0, 0, 0, 0 }, { 0, 1, 2 },
			};

			uint32_t seed = 0x12345678;
			for (size_t length = 0; length <= 32; ++length)
			{
				std::vector<uint8_t> bytes;
				for (size_t i = 0; i < length; ++i)
				{
					seed = seed * 1103515245u + 12345u;
					bytes.push_back(static_cast<uint8_t>(seed >> 16));
				}
				result.push_back(std::move(bytes));
			}

			std::vector<uint8_t> every_byte;
			for (int byte = 0; byte < 256; ++byte)
				every_byte.push_back(static_cast<uint8_t>(byte));
			result.push_back(std::move(every_byte));

			return result;
		}();
		return data;
	}

	/// \name Encoding Tags
	/// One per encoding in the header, so that the tests for the behavior they all share can be typed tests
	/// @{

	struct base64_encoding
	{
		static constexpr auto name = "base64"sv;
		static constexpr auto invalid = "Zm9vYmF!"sv; ///< digits enough for 5 bytes, then a character that is not one
		static std::string encode(std::span<uint8_t const> bytes) { return base64::encode(bytes); }
		static size_t encode(std::span<uint8_t const> bytes, std::string& output) { return base64::encode(bytes, output); }
		static std::vector<uint8_t> decode(std::string_view encoded) { return base64::decode<uint8_t>(encoded); }
		static size_t decode(std::string_view encoded, std::vector<uint8_t>& output) { return base64::decode(encoded, output); }
	};

	struct base64url_encoding
	{
		static constexpr auto name = "base64url"sv;
		static constexpr auto invalid = "Zm9vYmF="sv; ///< padding, which base64url does not allow
		static std::string encode(std::span<uint8_t const> bytes) { return base64url::encode(bytes); }
		static size_t encode(std::span<uint8_t const> bytes, std::string& output) { return base64url::encode(bytes, output); }
		static std::vector<uint8_t> decode(std::string_view encoded) { return base64url::decode<uint8_t>(encoded); }
		static size_t decode(std::string_view encoded, std::vector<uint8_t>& output) { return base64url::decode(encoded, output); }
	};

	struct base85_encoding
	{
		static constexpr auto name = "base85"sv;
		static constexpr auto invalid = "AoDTs@<~"sv; ///< a full group, then digits and a character that is not one
		static std::string encode(std::span<uint8_t const> bytes) { return base85::encode(bytes); }
		static size_t encode(std::span<uint8_t const> bytes, std::string& output) { return base85::encode(bytes, output); }
		static std::vector<uint8_t> decode(std::string_view encoded) { return base85::decode<uint8_t>(encoded); }
		static size_t decode(std::string_view encoded, std::vector<uint8_t>& output) { return base85::decode(encoded, output); }
	};

	/// @}
}

template <typename ENCODING>
class encoding_test : public ::testing::Test {};

using encoding_types = ::testing::Types<base64_encoding, base64url_encoding, base85_encoding>;
TYPED_TEST_SUITE(encoding_test, encoding_types);

/// \name Behavior shared by every encoding
/// @{

TYPED_TEST(encoding_test, roundtrips_every_byte_sequence)
{
	for (auto const& data : test_data())
	{
		const auto encoded = TypeParam::encode(data);
		const auto decoded = TypeParam::decode(encoded);
		EXPECT_EQ(decoded, data) << TypeParam::name << " of " << data.size() << " bytes gave '" << encoded << "'";
	}
}

TYPED_TEST(encoding_test, roundtrips_the_empty_sequence)
{
	EXPECT_EQ(TypeParam::encode(std::span<uint8_t const>{}), "") << TypeParam::name;
	EXPECT_TRUE(TypeParam::decode(""sv).empty()) << TypeParam::name;
}

TYPED_TEST(encoding_test, appends_to_the_output_instead_of_replacing_it)
{
	for (auto const& data : test_data())
	{
		std::string text = "text:";
		const auto appended = TypeParam::encode(data, text);
		EXPECT_EQ(appended, text.size() - 5) << TypeParam::name;
		EXPECT_EQ(text, "text:" + TypeParam::encode(data)) << TypeParam::name;

		std::vector<uint8_t> bytes = { 42 };
		const auto decoded = TypeParam::decode(text.substr(5), bytes);
		EXPECT_EQ(decoded, bytes.size() - 1) << TypeParam::name;
		EXPECT_EQ(bytes.front(), 42) << TypeParam::name;
		EXPECT_EQ(std::vector<uint8_t>(bytes.begin() + 1, bytes.end()), data) << TypeParam::name;
	}
}

TYPED_TEST(encoding_test, both_overloads_of_encode_and_decode_agree)
{
	for (auto const& data : test_data())
	{
		std::string encoded;
		const auto encoded_count = TypeParam::encode(data, encoded);
		EXPECT_EQ(encoded_count, encoded.size()) << TypeParam::name;
		EXPECT_EQ(encoded, TypeParam::encode(data)) << TypeParam::name;

		std::vector<uint8_t> decoded;
		const auto decoded_count = TypeParam::decode(encoded, decoded);
		EXPECT_EQ(decoded_count, decoded.size()) << TypeParam::name;
		EXPECT_EQ(decoded, TypeParam::decode(encoded)) << TypeParam::name;
	}
}

TYPED_TEST(encoding_test, leaves_the_output_untouched_when_decoding_fails)
{
	/// the invalid character comes last, so that a decoder that did not clean up after itself would leave bytes behind
	std::vector<uint8_t> bytes = { 1, 2, 3 };
	EXPECT_EQ(TypeParam::decode(TypeParam::invalid, bytes), 0u) << TypeParam::name;
	EXPECT_EQ(bytes, (std::vector<uint8_t>{ 1, 2, 3 })) << TypeParam::name;

	EXPECT_TRUE(TypeParam::decode(TypeParam::invalid).empty()) << TypeParam::name;
}

/// @}

TEST(encoding_alphabets, are_valid)
{
	static_assert(is_valid_alphabet(base64::alphabet));
	static_assert(is_valid_alphabet(base64url::alphabet));
	static_assert(is_valid_alphabet(base85::alphabet));

	EXPECT_TRUE(is_valid_alphabet(base64::alphabet));
	EXPECT_TRUE(is_valid_alphabet(base64url::alphabet));
	EXPECT_TRUE(is_valid_alphabet(base85::alphabet));

	/// the characters that are not digits must not be mistaken for ones
	static_assert(base64::digit_values[static_cast<uint8_t>(base64::padding_char)] < 0);
	static_assert(base85::digit_values[static_cast<uint8_t>(base85::zero_group_char)] < 0);

	/// and the check catches what it is meant to catch
	static_assert(!is_valid_alphabet(std::array<char, 3>{ 65, 66, 65 }));      /// the same character twice
	static_assert(!is_valid_alphabet(std::array<char, 2>{ 65, char(-1) }));    /// not ASCII
}

TEST(encoding_alphabets, are_the_ones_the_standards_ask_for)
{
	/// base64 (RFC 4648, section 4) and base64url (section 5) only differ in their last two digits
	EXPECT_TRUE(std::equal(base64::alphabet.begin(), base64::alphabet.begin() + 62, base64url::alphabet.begin()));
	EXPECT_EQ(std::string(base64::alphabet.begin(), base64::alphabet.end()),
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
	EXPECT_EQ(std::string(base64url::alphabet.begin(), base64url::alphabet.end()),
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_");
	EXPECT_EQ(base64::padding_char, '=');

	/// base85 (btoa/Ascii85) uses the 85 consecutive characters starting at 33 (`!` through `u`)
	for (size_t i = 0; i < base85::alphabet.size(); ++i)
		EXPECT_EQ(base85::alphabet[i], static_cast<char>(33 + i));
	EXPECT_EQ(base85::zero_group_char, 'z');
	EXPECT_EQ(base85::base, 85u);
}

/// \name base64
/// @{

TEST(base64, encodes_the_rfc4648_test_vectors) /// RFC 4648, section 10
{
	EXPECT_EQ(base64::encode(""sv), "");
	EXPECT_EQ(base64::encode("f"sv), "Zg==");
	EXPECT_EQ(base64::encode("fo"sv), "Zm8=");
	EXPECT_EQ(base64::encode("foo"sv), "Zm9v");
	EXPECT_EQ(base64::encode("foob"sv), "Zm9vYg==");
	EXPECT_EQ(base64::encode("fooba"sv), "Zm9vYmE=");
	EXPECT_EQ(base64::encode("foobar"sv), "Zm9vYmFy");
}

TEST(base64, decodes_the_rfc4648_test_vectors)
{
	EXPECT_EQ(as_str(base64::decode<char>(""sv)), "");
	EXPECT_EQ(as_str(base64::decode<char>("Zg=="sv)), "f");
	EXPECT_EQ(as_str(base64::decode<char>("Zm8="sv)), "fo");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9v"sv)), "foo");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9vYg=="sv)), "foob");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9vYmE="sv)), "fooba");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9vYmFy"sv)), "foobar");
}

TEST(base64, accepts_input_without_padding)
{
	EXPECT_EQ(as_str(base64::decode<char>("Zg"sv)), "f");
	EXPECT_EQ(as_str(base64::decode<char>("Zm8"sv)), "fo");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9vYg"sv)), "foob");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9vYmE"sv)), "fooba");
}

TEST(base64, ignores_whitespace_when_decoding)
{
	EXPECT_EQ(as_str(base64::decode<char>("Zm9v\r\nYmFy"sv)), "foobar");
	EXPECT_EQ(as_str(base64::decode<char>(" Z m 9 v Y m F y "sv)), "foobar");
	EXPECT_EQ(as_str(base64::decode<char>("Zg =="sv)), "f");
	EXPECT_TRUE(base64::decode<char>(" \t\r\n"sv).empty());
}

TEST(base64, rejects_invalid_input)
{
	EXPECT_TRUE(base64::decode<uint8_t>("A"sv).empty());        /// one digit carries 6 bits, not enough for a byte
	EXPECT_TRUE(base64::decode<uint8_t>("AAAAA"sv).empty());    /// ...even after a whole group
	EXPECT_TRUE(base64::decode<uint8_t>("A==="sv).empty());     /// more padding than a group can take
	EXPECT_TRUE(base64::decode<uint8_t>("=="sv).empty());
	EXPECT_TRUE(base64::decode<uint8_t>("AA="sv).empty());      /// padding that does not fill the group
	EXPECT_TRUE(base64::decode<uint8_t>("AAAA="sv).empty());    /// padding of a group that is already full
	EXPECT_TRUE(base64::decode<uint8_t>("QQ==QQ=="sv).empty()); /// data after padding
	EXPECT_TRUE(base64::decode<uint8_t>("Zm9-"sv).empty());     /// not a base64 digit
	EXPECT_TRUE(base64::decode<uint8_t>("Zm9\x80"sv).empty());  /// not even ASCII
}

/// @}

/// \name base64url
/// @{

TEST(base64url, encodes_the_rfc7515_example) /// RFC 7515, appendix A.1
{
	const std::string jose_header = "{\"typ\":\"JWT\",\r\n \"alg\":\"HS256\"}";
	const auto encoded = "eyJ0eXAiOiJKV1QiLA0KICJhbGciOiJIUzI1NiJ9"sv;

	EXPECT_EQ(base64url::encode(jose_header), encoded);
	EXPECT_EQ(as_str(base64url::decode<char>(encoded)), jose_header);
}

TEST(base64url, omits_the_padding)
{
	EXPECT_EQ(base64url::encode("f"sv), "Zg");   /// base64 gives "Zg=="
	EXPECT_EQ(base64url::encode("fo"sv), "Zm8"); /// base64 gives "Zm8="
	EXPECT_EQ(base64url::encode("foo"sv), "Zm9v");
	EXPECT_EQ(base64url::encode("foobar"sv), "Zm9vYmFy");

	EXPECT_EQ(as_str(base64url::decode<char>("Zg"sv)), "f");
	EXPECT_EQ(as_str(base64url::decode<char>("Zm8"sv)), "fo");
}

TEST(base64url, uses_the_url_safe_alphabet)
{
	const std::array<uint8_t, 3> bytes = { 0xFB, 0xFF, 0xBF }; /// the three bytes that use the last two digits
	EXPECT_EQ(base64::encode(bytes), "+/+/");
	EXPECT_EQ(base64url::encode(bytes), "-_-_");

	/// nothing that base64url ever emits needs escaping in a URL or a filename
	for (auto const& data : test_data())
	{
		const auto encoded = base64url::encode(std::span{ data });
		EXPECT_EQ(encoded.find_first_of("+/=\r\n\t "), std::string::npos) << "in '" << encoded << "'";
	}
}

TEST(base64url, rejects_everything_that_is_not_a_digit)
{
	/// RFC 7515 2 allows no padding, no line breaks, no whitespace and no other extra characters...
	EXPECT_TRUE(base64url::decode<uint8_t>("Zg=="sv).empty());
	EXPECT_TRUE(base64url::decode<uint8_t>("Zm9v "sv).empty());
	EXPECT_TRUE(base64url::decode<uint8_t>("Zm9\nv"sv).empty());
	EXPECT_TRUE(base64url::decode<uint8_t>("+/+/"sv).empty());
	EXPECT_TRUE(base64url::decode<uint8_t>("A"sv).empty());

	/// ...while base64 takes all of those
	EXPECT_EQ(as_str(base64::decode<char>("Zg=="sv)), "f");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9v "sv)), "foo");
	EXPECT_EQ(as_str(base64::decode<char>("Zm9\nv"sv)), "foo");
	EXPECT_EQ(base64::decode<uint8_t>("+/+/"sv).size(), 3u);

	/// and the two url-safe digits are just as invalid in base64
	EXPECT_TRUE(base64::decode<uint8_t>("-_-_"sv).empty());
	EXPECT_EQ(base64url::decode<uint8_t>("-_-_"sv).size(), 3u);
}

/// @}

/// \name base85
/// @{

TEST(base85, encodes_known_values)
{
	EXPECT_EQ(base85::encode(""sv), "");
	EXPECT_EQ(base85::encode("f"sv), "Ac");
	EXPECT_EQ(base85::encode("fo"sv), "Ao@");
	EXPECT_EQ(base85::encode("foo"sv), "AoDS");
	EXPECT_EQ(base85::encode("foob"sv), "AoDTs");
	EXPECT_EQ(base85::encode("fooba"sv), "AoDTs@/");
	EXPECT_EQ(base85::encode("foobar"sv), "AoDTs@<)");
	EXPECT_EQ(base85::encode("sure."sv), "F*2M7/c");

	EXPECT_EQ(as_str(base85::decode<char>("AoDTs@<)"sv)), "foobar");
	EXPECT_EQ(as_str(base85::decode<char>("F*2M7/c"sv)), "sure.");
}

TEST(base85, writes_one_more_digit_than_the_group_has_bytes)
{
	for (auto const& data : test_data())
	{
		const auto encoded = base85::encode(std::span{ data });
		const auto expected = (data.size() / 4) * 5 + (data.size() % 4 ? data.size() % 4 + 1 : 0);
		EXPECT_EQ(encoded.size(), expected) << "for " << data.size() << " bytes";
	}
}

TEST(base85, accepts_the_z_shorthand_but_never_emits_it)
{
	const std::array<uint8_t, 4> zeroes = {};
	EXPECT_EQ(base85::encode(zeroes), "!!!!!");

	EXPECT_EQ(as_str(base85::decode<char>("z"sv)), std::string(4, '\0'));
	EXPECT_EQ(as_str(base85::decode<char>("zz"sv)), std::string(8, '\0'));
	EXPECT_EQ(as_str(base85::decode<char>("!!!!!"sv)), std::string(4, '\0'));
	EXPECT_EQ(as_str(base85::decode<char>("zAc"sv)), std::string(4, '\0') + "f");

	/// it stands in for a whole group, so it may only appear where one starts
	EXPECT_TRUE(base85::decode<uint8_t>("!z"sv).empty());
	EXPECT_TRUE(base85::decode<uint8_t>("!!!!!!z"sv).empty());
}

TEST(base85, ignores_whitespace_when_decoding)
{
	EXPECT_EQ(as_str(base85::decode<char>("AoD Ts@\r\n<)"sv)), "foobar");
	EXPECT_TRUE(base85::decode<uint8_t>(" \t\r\n"sv).empty());
}

TEST(base85, rejects_invalid_input)
{
	EXPECT_TRUE(base85::decode<uint8_t>("!"sv).empty());      /// one digit is not enough for even one byte
	EXPECT_TRUE(base85::decode<uint8_t>("!!!!!!"sv).empty()); /// ...even after a whole group
	EXPECT_TRUE(base85::decode<uint8_t>("uuuuu"sv).empty());  /// a group that would not fit in 4 bytes
	EXPECT_TRUE(base85::decode<uint8_t>("v"sv).empty());      /// past the last digit
	EXPECT_TRUE(base85::decode<uint8_t>("~~~~~"sv).empty());
	EXPECT_TRUE(base85::decode<uint8_t>("<~AoDTs@<)~>"sv).empty()); /// the Adobe delimiters are not accepted

	/// the largest group that does fit
	EXPECT_EQ(as_str(base85::decode<char>("s8W-!"sv)), "\xff\xff\xff\xff");
	EXPECT_EQ(base85::encode("\xff\xff\xff\xff"sv), "s8W-!");
}

/// @}

TEST(encoding, works_with_every_bytelike_type)
{
	EXPECT_EQ(as_str(base64::decode<char>("Zm9vYmFy"sv)), "foobar");
	EXPECT_EQ(as_str(base64::decode<uint8_t>("Zm9vYmFy"sv)), "foobar");
	EXPECT_EQ(as_str(base64::decode<char8_t>("Zm9vYmFy"sv)), "foobar");
	EXPECT_EQ(as_str(base64::decode<std::byte>("Zm9vYmFy"sv)), "foobar");
	EXPECT_EQ(as_str(base64url::decode<std::byte>("Zm9vYmFy"sv)), "foobar");
	EXPECT_EQ(as_str(base85::decode<std::byte>("AoDTs@<)"sv)), "foobar");

	std::vector<std::byte> bytes;
	EXPECT_EQ(base64::decode("Zm9vYmFy"sv, bytes), 6u);
	EXPECT_EQ(as_str(bytes), "foobar");
}

TEST(encoding, works_with_common_range_types)
{
	const std::string text = "foobar";
	const std::vector<uint8_t> bytes = { 'f', 'o', 'o', 'b', 'a', 'r' };
	const std::array<char, 6> chars = { 'f', 'o', 'o', 'b', 'a', 'r' };
	const auto bytes_span = std::span{ bytes };

	EXPECT_EQ(base64::encode(text), "Zm9vYmFy");
	EXPECT_EQ(base64::encode(std::string_view{ text }), "Zm9vYmFy");
	EXPECT_EQ(base64::encode(bytes), "Zm9vYmFy");
	EXPECT_EQ(base64::encode(chars), "Zm9vYmFy");
	EXPECT_EQ(base64::encode(bytes_span), "Zm9vYmFy");
	EXPECT_EQ(base64::encode(std::as_bytes(bytes_span)), "Zm9vYmFy");
	EXPECT_EQ(base85::encode(std::as_bytes(bytes_span)), "AoDTs@<)");
}
