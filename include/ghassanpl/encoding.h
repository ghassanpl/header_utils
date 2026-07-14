/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "bytes.h"
#include "string_ops.h"

namespace ghassanpl
{
	/// \defgroup Encoding Encoding
	/// Function templates that encode or decode data to/from common representations.
	///
	/// Every `baseN` namespace in this header gives you the same four functions:
	/// * `encode(bytes, output) -> size_t` - appends `bytes` encoded as text to `output`, returns the number of characters appended;
	/// * `encode(bytes) -> std::string` - returns `bytes` encoded as text;
	/// * `decode<B>(encoded, output) -> size_t` - appends the bytes that `encoded` represents to `output`, returns the number of bytes appended;
	/// * `decode<B>(encoded) -> std::vector<B>` - returns the bytes that `encoded` represents.
	///
	/// as well as an `alphabet` - the array of characters that represent the digits of that base, in order - and a
	/// `digit_values` table that maps a character back to the digit it represents (or to -1, if it represents none).
	///
	/// Every one of these encodings round-trips exactly, for every byte sequence (including the empty one), that is,
	/// `decode<B>(encode(bytes))` always gives back `bytes`.
	///
	/// \par Decoding Errors
	/// Decoding is strict - the entire `encoded` string has to be a valid encoding of *something*. If it is not, nothing
	/// at all is decoded: `decode` appends nothing to `output` and returns 0 (or returns an empty vector, for the overload
	/// that does not take an `output`). Since any non-empty valid input decodes to at least one byte,
	/// `!encoded.empty() && decode(encoded, output) == 0` means "`encoded` is invalid".
	///
	/// \internal
	/// The alphabets are given as character values (e.g. 65) instead of character literals (e.g. 'A'), because the
	/// encoding of this source file might not be ASCII-based (the same rationale as in \ref ghassanpl::string_ops::ascii)
	/// \endinternal
	///
	/// \todo `expected`-returning versions of the `decode` functions, so that the kind and position of a failure can be reported
	/// @{

	namespace detail
	{
		/// Creates a table that maps a character to the value of the digit it represents in `alphabet`, or to -1 if it represents none
		/// \pre `alphabet` consists of distinct ASCII characters (encoding_tests.cpp checks that the ones in this header do)
		template <size_t N>
		[[nodiscard]] constexpr auto make_digit_values(std::array<char, N> alphabet) -> std::array<int8_t, 256>
		{
			static_assert(N <= 128, "digit values must fit in an int8_t, and the alphabet must fit in the ASCII range");
			std::array<int8_t, 256> result{};
			result.fill(static_cast<int8_t>(-1));
			for (size_t i = 0; i < N; ++i)
				result[static_cast<uint8_t>(alphabet[i])] = static_cast<int8_t>(i);
			return result;
		}

		/// Encodes each group of 3 bytes as 4 digits of `ALPHABET`, 6 bits to a digit, most significant bits first.
		/// If `PADDING_CHAR` is not 0, the last group is padded with it up to 4 digits.
		/// \returns the number of characters appended to `output`
		template <auto const& ALPHABET, char PADDING_CHAR, bytelike_range RANGE>
		size_t encode_base64(RANGE&& bytes, std::string& output)
		{
			static_assert(ALPHABET.size() == 64, "a base64 alphabet must have exactly 64 characters");

			if constexpr (std::ranges::sized_range<RANGE>)
				output.reserve(output.size() + 4 * ((std::ranges::size(bytes) + 2) / 3)); /// over-reserves by at most 3 when unpadded

			size_t appended = 0;
			uint32_t buffer = 0;      /// only its lowest `buffered_bits` bits are meaningful, the rest are shifted out and ignored
			size_t buffered_bits = 0;

			for (auto&& byte : bytes)
			{
				buffer = (buffer << 8) | to_u8(byte);
				buffered_bits += 8;
				while (buffered_bits >= 6)
				{
					buffered_bits -= 6;
					output += ALPHABET[(buffer >> buffered_bits) & 0b111111];
					++appended;
				}
			}

			if (buffered_bits > 0) /// the leftover 2 or 4 bits become the most significant bits of one last digit
			{
				output += ALPHABET[(buffer << (6 - buffered_bits)) & 0b111111];
				++appended;
				if constexpr (PADDING_CHAR != 0)
				{
					while (appended % 4 != 0)
					{
						output += PADDING_CHAR;
						++appended;
					}
				}
			}

			return appended;
		}

		/// The counterpart of \c encode_base64, with `DIGIT_VALUES` being the \c make_digit_values table of `ALPHABET`.
		/// If `IGNORE_WHITESPACE` is true, ASCII whitespace in `encoded` is ignored; if `PADDING_CHAR` is not 0, that
		/// character is accepted as padding of the last group (and has to pad it exactly).
		/// \returns the number of bytes appended to `output`, or 0 (leaving `output` untouched) if `encoded` is invalid
		template <auto const& ALPHABET, auto const& DIGIT_VALUES, bool IGNORE_WHITESPACE, char PADDING_CHAR, bytelike B>
		size_t decode_base64(std::string_view encoded, std::vector<B>& output)
		{
			static_assert(ALPHABET.size() == 64, "a base64 alphabet must have exactly 64 characters");
			static_assert(PADDING_CHAR == 0 || DIGIT_VALUES[static_cast<uint8_t>(PADDING_CHAR)] < 0, "the padding character must not be a digit");

			const auto start_size = output.size();
			output.reserve(start_size + (encoded.size() / 4) * 3 + 3);

			const auto fail = [&output, start_size] { output.resize(start_size); return size_t(0); };

			uint32_t buffer = 0;
			size_t buffered_bits = 0;
			size_t digits = 0;
			[[maybe_unused]] size_t padding = 0;

			for (const char c : encoded)
			{
				const auto ch = static_cast<uint8_t>(c);

				if constexpr (IGNORE_WHITESPACE)
				{
					if (string_ops::ascii::isspace(ch))
						continue;
				}

				if constexpr (PADDING_CHAR != 0)
				{
					if (ch == static_cast<uint8_t>(PADDING_CHAR))
					{
						if (++padding > 2) /// at most 2 of the 4 digits of the last group can be padding
							return fail();
						continue;
					}

					if (padding > 0) /// nothing but padding may follow padding
						return fail();
				}

				const auto digit = DIGIT_VALUES[ch];
				if (digit < 0)
					return fail();

				buffer = (buffer << 6) | static_cast<uint32_t>(digit);
				buffered_bits += 6;
				++digits;

				if (buffered_bits >= 8)
				{
					buffered_bits -= 8;
					output.push_back(std::bit_cast<B>(static_cast<uint8_t>((buffer >> buffered_bits) & 0xFF)));
				}
			}

			if (digits % 4 == 1) /// a single digit carries only 6 bits, not enough for even one byte
				return fail();
			if constexpr (PADDING_CHAR != 0)
			{
				if (padding > 0 && (digits + padding) % 4 != 0) /// padding has to complete the last group exactly
					return fail();
			}

			/// NOTE: The leftover 2 or 4 bits in `buffer` are ignored, even if they are not zero, as is customary
			return output.size() - start_size;
		}
	}

	/// \defgroup Base64 Base64
	/// The standard base64 encoding (RFC 4648, §4): every group of 3 bytes is represented by 4 characters of the
	/// `A`-`Z`, `a`-`z`, `0`-`9`, `+`, `/` alphabet, with the last group padded with `=` up to 4 characters.
	/// Encoded data is 4/3rds the size of the input.
	///
	/// \note When decoding, ASCII whitespace is ignored (line-wrapped base64 is common), the padding is optional but,
	/// if present, has to be correct, and the (at most 4) leftover bits of the last group are ignored even if they are not zero.
	/// \see \ref Base64Url, for the flavour to use in URLs and filenames
	/// \ingroup Encoding
	/// @{
	namespace base64
	{
		/// The 64 characters that represent the digits of a base64 number, in order
		constexpr inline std::array<char, 64> alphabet = {
			65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, /// ABCDEFGHIJKLM
			78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, /// NOPQRSTUVWXYZ
			97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, /// abcdefghijklm
			110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, /// nopqrstuvwxyz
			48, 49, 50, 51, 52, 53, 54, 55, 56, 57, /// 0123456789
			43, 47, /// + /
		};

		/// The character that the last group is padded with, up to 4 characters
		constexpr inline char padding_char = 61; ///< =

		/// Maps a character to the base64 digit it represents, or to -1 if it represents none
		constexpr inline auto digit_values = detail::make_digit_values(alphabet);

		/// Appends `bytes`, encoded as base64, to `output`
		/// \returns the number of characters appended to `output`
		size_t encode(bytelike_range auto bytes, std::string& output)
		{
			return detail::encode_base64<alphabet, padding_char>(bytes, output);
		}

		/// \returns `bytes` encoded as base64
		[[nodiscard]] std::string encode(bytelike_range auto bytes)
		{
			std::string result;
			encode(bytes, result);
			return result;
		}

		/// Appends the bytes that the base64 string `encoded` represents to `output`
		/// \returns the number of bytes appended to `output`, or 0 (leaving `output` untouched) if `encoded` is not a valid base64 string
		template <bytelike B>
		size_t decode(std::string_view encoded, std::vector<B>& output)
		{
			return detail::decode_base64<alphabet, digit_values, true, padding_char>(encoded, output);
		}

		/// \returns the bytes that the base64 string `encoded` represents, or an empty vector if it does not represent any
		template <bytelike B>
		[[nodiscard]] std::vector<B> decode(std::string_view encoded)
		{
			std::vector<B> result;
			decode(encoded, result);
			return result;
		}
	}
	/// @}

	/// \defgroup Base64Url Base64url
	/// The base64url encoding as JWS defines it (RFC 7515, §2): base64 with the URL- and filename-safe alphabet of
	/// RFC 4648 §5 - `-` and `_` in place of `+` and `/` - with all trailing padding omitted, and without any line
	/// breaks, whitespace or other additional characters. Encoded data is 4/3rds the size of the input.
	///
	/// This is the encoding used for the parts of JWSs, JWTs and JWKs, and generally the one to use wherever the
	/// result ends up in a URL, a filename, or anything else that gives `+`, `/` or `=` a meaning of their own.
	///
	/// \note This is the strict JWS flavour, so unlike \ref Base64, decoding rejects everything that is not a digit -
	/// including both `=` padding and whitespace. The (at most 4) leftover bits of the last group are ignored, though,
	/// even if they are not zero.
	/// \see \ref Base64
	/// \ingroup Encoding
	/// @{
	namespace base64url
	{
		/// The 64 characters that represent the digits of a base64url number, in order
		constexpr inline std::array<char, 64> alphabet = {
			65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, /// ABCDEFGHIJKLM
			78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, /// NOPQRSTUVWXYZ
			97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, /// abcdefghijklm
			110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, /// nopqrstuvwxyz
			48, 49, 50, 51, 52, 53, 54, 55, 56, 57, /// 0123456789
			45, 95, /// - _
		};

		/// Maps a character to the base64url digit it represents, or to -1 if it represents none
		constexpr inline auto digit_values = detail::make_digit_values(alphabet);

		/// Appends `bytes`, encoded as base64url, to `output`
		/// \returns the number of characters appended to `output`
		size_t encode(bytelike_range auto bytes, std::string& output)
		{
			return detail::encode_base64<alphabet, 0>(bytes, output);
		}

		/// \returns `bytes` encoded as base64url
		[[nodiscard]] std::string encode(bytelike_range auto bytes)
		{
			std::string result;
			encode(bytes, result);
			return result;
		}

		/// Appends the bytes that the base64url string `encoded` represents to `output`
		/// \returns the number of bytes appended to `output`, or 0 (leaving `output` untouched) if `encoded` is not a valid base64url string
		template <bytelike B>
		size_t decode(std::string_view encoded, std::vector<B>& output)
		{
			return detail::decode_base64<alphabet, digit_values, false, 0>(encoded, output);
		}

		/// \returns the bytes that the base64url string `encoded` represents, or an empty vector if it does not represent any
		template <bytelike B>
		[[nodiscard]] std::vector<B> decode(std::string_view encoded)
		{
			std::vector<B> result;
			decode(encoded, result);
			return result;
		}
	}
	/// @}

	/// \defgroup Base85 Base85
	/// The original (btoa/Ascii85) base85 encoding: every group of 4 bytes is treated as a big-endian 32-bit number and
	/// written out as 5 base-85 digits, using the 85 characters between 33 (`!`) and 117 (`u`) as its alphabet.
	/// A trailing group of `n` bytes is written as `n + 1` digits. Encoded data is 5/4ths the size of the input,
	/// which makes this the densest of the encodings here that stay within printable, mostly-portable ASCII.
	///
	/// \note We never emit the `z` shorthand for a group of 4 zero bytes (so that the size of the output only ever depends
	/// on the size of the input), but we do accept it when decoding. The `<~ ~>` delimiters of the Adobe variant are neither
	/// emitted nor accepted.
	/// \note When decoding, ASCII whitespace is ignored, as the original encoding allows for it.
	/// \ingroup Encoding
	/// @{
	namespace base85
	{
		/// The 85 characters that represent the digits of a base85 number, in order
		constexpr inline std::array<char, 85> alphabet = {
			33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, /// 33 to 49
			50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, /// 50 to 66
			67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, /// 67 to 83
			84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, /// 84 to 100
			101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, /// 101 to 117
		};

		/// The base of the encoding - the number of digits in its \c alphabet
		constexpr inline uint32_t base = static_cast<uint32_t>(alphabet.size());
		static_assert(uint64_t(base) * base * base * base * base > 0xFFFFFFFFULL, "5 digits must be able to represent any group of 4 bytes");

		/// The character that a whole group of 4 zero bytes may be shortened to
		constexpr inline char zero_group_char = 122; ///< z

		/// Maps a character to the base85 digit it represents, or to -1 if it represents none
		constexpr inline auto digit_values = detail::make_digit_values(alphabet);
		static_assert(digit_values[static_cast<uint8_t>(zero_group_char)] < 0, "the zero-group character must not be a digit");

		/// Appends `bytes`, encoded as base85, to `output`
		/// \returns the number of characters appended to `output`
		size_t encode(bytelike_range auto bytes, std::string& output)
		{
			if constexpr (std::ranges::sized_range<decltype(bytes)>)
			{
				const auto size = std::ranges::size(bytes);
				output.reserve(output.size() + (size / 4) * 5 + (size % 4 ? size % 4 + 1 : 0));
			}

			size_t appended = 0;
			uint32_t group = 0;
			size_t group_size = 0;

			const auto flush_group = [&] {
				std::array<char, 5> digits{};
				auto value = group << (8 * (4 - group_size)); /// a partial group is padded with zero bytes
				for (size_t i = digits.size(); i-- > 0; )
				{
					digits[i] = alphabet[value % base];
					value /= base;
				}
				output.append(digits.data(), group_size + 1); /// 4 bytes give 5 digits, n bytes give n + 1
				appended += group_size + 1;
				group = 0;
				group_size = 0;
			};

			for (auto&& byte : bytes)
			{
				group = (group << 8) | to_u8(byte);
				if (++group_size == 4)
					flush_group();
			}
			if (group_size > 0)
				flush_group();

			return appended;
		}

		/// \returns `bytes` encoded as base85
		[[nodiscard]] std::string encode(bytelike_range auto bytes)
		{
			std::string result;
			encode(bytes, result);
			return result;
		}

		/// Appends the bytes that the base85 string `encoded` represents to `output`
		/// \returns the number of bytes appended to `output`, or 0 (leaving `output` untouched) if `encoded` is not a valid base85 string
		template <bytelike B>
		size_t decode(std::string_view encoded, std::vector<B>& output)
		{
			const auto start_size = output.size();
			output.reserve(start_size + (encoded.size() / 5) * 4 + 4);

			const auto fail = [&output, start_size] { output.resize(start_size); return size_t(0); };

			uint64_t group = 0;
			size_t group_size = 0;

			const auto flush_group = [&]() -> bool {
				/// The digits missing from a partial group are filled in with the largest digit, so that the bytes we
				/// do have round up to exactly the values they had when they were encoded
				for (size_t i = group_size; i < 5; ++i)
					group = group * base + (base - 1);

				if (group > 0xFFFFFFFFULL) /// 5 digits can represent more than 4 bytes, and this group does
					return false;

				const auto byte_count = group_size - 1; /// 5 digits give 4 bytes, n digits give n - 1
				for (size_t i = 0; i < byte_count; ++i)
					output.push_back(std::bit_cast<B>(static_cast<uint8_t>((group >> (8 * (3 - i))) & 0xFF)));

				group = 0;
				group_size = 0;
				return true;
			};

			for (const char c : encoded)
			{
				const auto ch = static_cast<uint8_t>(c);

				if (string_ops::ascii::isspace(ch)) /// the original encoding allows for whitespace anywhere
					continue;

				if (ch == static_cast<uint8_t>(zero_group_char))
				{
					if (group_size != 0) /// it stands in for a whole group, so it may only appear where one starts
						return fail();
					output.insert(output.end(), size_t(4), B{});
					continue;
				}

				const auto digit = digit_values[ch];
				if (digit < 0)
					return fail();

				group = group * base + static_cast<uint32_t>(digit);
				if (++group_size == 5 && !flush_group())
					return fail();
			}

			if (group_size == 1) /// a single digit is not enough to recover even one byte
				return fail();
			if (group_size > 1 && !flush_group())
				return fail();

			return output.size() - start_size;
		}

		/// \returns the bytes that the base85 string `encoded` represents, or an empty vector if it does not represent any
		template <bytelike B>
		[[nodiscard]] std::vector<B> decode(std::string_view encoded)
		{
			std::vector<B> result;
			decode(encoded, result);
			return result;
		}
	}
	/// @}

	namespace varint
	{

	}

	/// @}
}
