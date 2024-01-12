/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "string_ops.h"
#include "cpp23.h"

namespace ghassanpl::string_ops
{
	/// \defgroup Unicode Unicode
	/// Functions and types that work on Unicode strings. 
	/// This code uses `char32_t` to represent single Unicode codepoints (as UTF-32 code units).
	/// \ingroup StringOps
	/// @{
	
	/// Returns whether `cp` is a codepoint that encodes the high part of a codepoint with a more-than-16-bit value
	[[nodiscard]] constexpr bool is_high_surrogate(char32_t cp) noexcept;
	
	/// Returns whether `cp` is a codepoint that encodes the low part of a codepoint with a more-than-16-bit value
	[[nodiscard]] constexpr bool is_low_surrogate(char32_t cp) noexcept;
	
	/// Returns whether `cp` is a codepoint that encodes any part of a codepoint with a more-than-16-bit value
	[[nodiscard]] constexpr bool is_surrogate(char32_t cp) noexcept;

	/// Returns whether `cp` has a value that is a valid Unicode codepoint (ie. between 0 and 0x10FFFF).
	[[nodiscard]] constexpr bool is_unicode(char32_t cp) noexcept;

	/// Returns whether `cp` has a value that is a valid Unicode character (ie. a value that encodes a (part of a) character).
	/// Specifically, byte order marks are not characters, but surrogates technically are part of a character.
	[[nodiscard]] constexpr bool is_unicode_character(char32_t cp) noexcept;
	
	/// Returns the codepoint encoded by two surrogates
	[[nodiscard]] constexpr char32_t surrogate_pair_to_codepoint(char32_t high, char32_t low) noexcept;

	enum class unicode_plane;
	[[nodiscard]] constexpr auto get_unicode_plane(char32_t cp) noexcept -> unicode_plane;

	/// Specifies a base text-encoding, ignoring endianness for multi-byte encodings
	enum class base_text_encoding
	{
		unknown, utf8, utf16, utf32,
		utf7, utf1, utf_ebcdic,
		scsu, bocu1, gb18030,
	};

	/// Type that represents a specific text encoding - a combination of \c ghassanpl::string_ops::base_text_encoding and endianness
	struct text_encoding
	{
		base_text_encoding base_encoding;
		std::endian endianness;

		[[nodiscard]] constexpr auto operator<=>(text_encoding const& other) const noexcept = default;
	};

	/// \name Encodings
	/// Values representing UTF encodings
	/// @{
	constexpr inline text_encoding utf8_encoding = { base_text_encoding::utf8, std::endian::native };
	constexpr inline text_encoding utf16_le_encoding = { base_text_encoding::utf16, std::endian::little };
	constexpr inline text_encoding utf16_be_encoding = { base_text_encoding::utf16, std::endian::big };
	constexpr inline text_encoding utf32_le_encoding = { base_text_encoding::utf32, std::endian::little };
	constexpr inline text_encoding utf32_be_encoding = { base_text_encoding::utf32, std::endian::big };
	/// Represents an unknown text encoding (e.g. when an encoding could not be determined)
	constexpr inline text_encoding unknown_text_encoding = { base_text_encoding::unknown, std::endian::native };
	/// @}

	/// \internal https://bjoern.hoehrmann.de/utf-8/decoder/dfa/

	/// Returns the number of UTF-8 octets necessarity to encode the given codepoint
	[[nodiscard]] constexpr size_t codepoint_utf8_count(char32_t cp) noexcept;

	struct text_decode_result;

	/// Attempts to decode the first codepoint in 8-bit range `str`, assuming it is encoded in `encoding`.
	[[nodiscard]] text_decode_result decode_codepoint(stringable8 auto str, text_encoding encoding);

	/// Consumes (see \c consume()) a [byte order mark](https://en.wikipedia.org/wiki/Byte_order_mark) from the beginning of sv, 
	/// and returns the encoding that the BOM represents (or \c unknown_text_encoding) if no BOM.
	text_encoding consume_bom(stringable8 auto& sv);
	
	/// Consumes (see \c consume()) a [byte order mark](https://en.wikipedia.org/wiki/Byte_order_mark) from the beginning of sv, 
	/// and returns the UTF-16 encoding that the BOM represents (or \c unknown_text_encoding) if no BOM.
	text_encoding consume_bom(stringable16 auto& sv);
	
	/// Consumes (see \c consume()) a [byte order mark](https://en.wikipedia.org/wiki/Byte_order_mark) from the beginning of sv, 
	/// and returns the UTF-32 encoding that the BOM represents (or \c unknown_text_encoding) if no BOM.
	text_encoding consume_bom(stringable32 auto& sv);
	
	/// Attempts to detect the encoding of a given 8-bit range.
	/// \note If no BOM is present, only detects UTF encodings.
	[[nodiscard]] text_encoding detect_encoding(stringable8 auto str);

	/// Consumes (see \c consume()) a UTF-8 codepoint from `str`.
	constexpr char32_t consume_utf8(string_view8 auto& str);

	[[nodiscard]] constexpr size_t count_utf8_codepoints(stringable8 auto str);

	/// Appends octets to `buffer` by encoding `cp` into UTF-8
	constexpr size_t append_utf8(string8 auto& buffer, char32_t cp);
	
	/// Returns `cp` encoded as a UTF-8 string
	/// \tparam RESULT the type of string to return (`std::string` by default)
	template <string8 RESULT = std::string>
	[[nodiscard]] constexpr RESULT to_utf8(char32_t cp);

	/// Returns `str` (a UTF-16-encoded string) encoded as a UTF-8 string
	/// \tparam RESULT the type of string to return (`std::string` by default)
	template <string8 RESULT = std::string, stringable16 STR>
	[[nodiscard]] constexpr RESULT to_utf8(STR&& str);

	/// Returns `str` (a UTF-32-encoded string) encoded as a UTF-8 string
	/// \tparam RESULT the type of string to return (`std::string` by default)
	template <string8 RESULT = std::string, stringable32 STR>
	[[nodiscard]] constexpr RESULT to_utf8(STR&& str);

	/// Returns `str` (a UTF-16-encoded string) encoded as a UTF-8 string
	[[nodiscard]] std::string to_string(std::wstring_view str);

	/// A simple view over an UTF8 string range with codepoint values
	template <std::ranges::view R>
	struct utf8_view;

	/// Consumes (see \c consume()) a UTF-16 codepoint from `str`.
	constexpr char32_t consume_utf16(string_view16 auto& str);

	/// Consumes (see \c consume()) a UTF-32 codepoint from `str`.
	constexpr char32_t consume_utf32(string_view32 auto& str);

	/// Appends 16-bit values to `buffer` by encoding `cp` into UTF-16
	/// \return the number of 16-bit codepoints appended
	constexpr size_t append_utf16(string16 auto& buffer, char32_t cp);

	/// Appends 32-bit values to `buffer` by encoding `cp` into UTF-16
	/// \return the number of 32-bit codepoints appended
	constexpr size_t append_utf32(string32 auto& buffer, char32_t cp);

	/// Returns `cp` encoded as a UTF-16 string
	/// \tparam RESULT the type of string to return (`std::wstring` by default)
	template <string16 RESULT = std::wstring>
	[[nodiscard]] constexpr RESULT to_utf16(char32_t cp);

	/// Returns `str` (a UTF-8-encoded string) encoded as a UTF-16 string
	/// \tparam RESULT the type of string to return (`std::wstring` by default)
	template <string16 RESULT, stringable8 STR>
	[[nodiscard]] constexpr RESULT to_utf16(STR str);

	/// Returns `str` (a UTF-8-encoded string) encoded as a UTF-16 string
	[[nodiscard]] std::wstring to_wstring(std::string_view str);

	/// Transcodes an [Extended ASCII](https://en.wikipedia.org/wiki/Extended_ASCII) string `source` into UTF-8 `dest`, according to `codepage_map`
	/// \param codepage_map A span of 128 Unicode codepoints that will be substituted for EASCII values 128-255
	constexpr void transcode_codepage_to_utf8(string8 auto& dest, stringable8 auto source, std::span<char32_t const, 128> codepage_map);

	/// Transcodes an [Extended ASCII](https://en.wikipedia.org/wiki/Extended_ASCII) string `source` into UTF-8, according to `codepage_map`
	/// \tparam RESULT the type of string to return (`std::string` by default)
	/// \param codepage_map A span of 128 Unicode codepoints that will be substituted for EASCII values 128-255
	/// \returns a UTF-8-encoded string of type T
	template <string8 RESULT = std::string>
	[[nodiscard]] constexpr auto transcode_codepage_to_utf8(stringable8 auto source, std::span<char32_t const, 128> codepage_map) -> RESULT;

	/// Consumes a codepoint from a UTF-encoded string and returns its
	template <typename T>
	constexpr char32_t consume_codepoint(T& str)
	{
		if (std::empty(str))
			return 0;
		using in_char_type = std::ranges::range_value_t<std::remove_cvref_t<T>>;
		if constexpr (stringable8<T>)
			return consume_utf8(str);
		else if constexpr (stringable16<T>)
			return consume_utf16(str);
		else if constexpr (stringable32<T>)
			return consume_utf32(str);
		else
			static_assert(stringable8<T>, "Unsupported character type");
	}

	template <typename T>
	constexpr void append_codepoint(T& str, char32_t cp)
	{
		if constexpr (string8<T>)
			append_utf8(str, cp);
		else if constexpr (string16<T>)
			append_utf16(str, cp);
		else if constexpr (string32<T>)
			append_utf32(str, cp);
		else
			static_assert(string8<T>, "Unsupported character type");
	}

	template <typename TO, typename FROM>
	constexpr void transcode_unicode(FROM const& from, TO& out)
	{
		auto from_sv = make_sv(from);
		while (!std::empty(from_sv))
			append_codepoint(out, consume_codepoint(from_sv));
	}

	template <typename TO, typename FROM>
	[[nodiscard]] constexpr TO transcode_unicode(FROM const& from)
	{
		TO result{};
		transcode_unicode(from, result);
		return result;
	}

	/// @}

	/// \internal
	/// 
	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///
	/// Implementation
	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///
	/// 
	/// TODO: upper-, lower-, titlecasing, case folding, collation, transliteration
	/// Though it would need ICU access...
	
	inline text_encoding consume_bom(stringable8 auto & sv)
	{
		static constexpr std::string_view bom_for_gb18030{ "\x84\x31\x95\x33", 4 };
		static constexpr std::string_view bom_for_utf32_be{ "\x00\x00\xFE\xFF", 4 };
		static constexpr std::string_view bom_for_utf32_le{ "\xFF\xFE\x00\x00", 4 };
		static constexpr std::string_view bom_for_utf_ebcdic{ "\xDD\x73\x66\x73", 4 };
		static constexpr std::string_view bom_for_utf8{ "\xEF\xBB\xBF", 3 };
		static constexpr std::string_view bom_for_utf7{ "\x2B\x2F\x76", 3 };
		static constexpr std::string_view bom_for_utf1{ "\xF7\x64\x4C", 3 };
		static constexpr std::string_view bom_for_scsu{ "\x0E\xFE\xFF", 3 };
		static constexpr std::string_view bom_for_bocu1{ "\xFB\xEE\x28", 3 };
		static constexpr std::string_view bom_for_utf16_be{ "\xFE\xFF", 2 };
		static constexpr std::string_view bom_for_utf16_le{ "\xFF\xFE", 2 };

		if (!sv.empty())
		{
			using enum std::endian;
			using enum ghassanpl::string_ops::base_text_encoding;
			if (sv.starts_with(bom_for_gb18030)) { sv.remove_prefix(bom_for_gb18030.size()); return { gb18030, native }; }
			if (sv.starts_with(bom_for_utf32_be)) { sv.remove_prefix(bom_for_utf32_be.size()); return { utf32, big }; }
			if (sv.starts_with(bom_for_utf32_le)) { sv.remove_prefix(bom_for_utf32_le.size()); return { utf32, little }; }
			if (sv.starts_with(bom_for_utf_ebcdic)) { sv.remove_prefix(bom_for_utf_ebcdic.size()); return { utf_ebcdic, native }; }
			if (sv.starts_with(bom_for_utf8)) { sv.remove_prefix(bom_for_utf8.size()); return { utf8, native }; }
			if (sv.starts_with(bom_for_utf7)) { sv.remove_prefix(bom_for_utf7.size()); return { utf7, native }; }
			if (sv.starts_with(bom_for_utf1)) { sv.remove_prefix(bom_for_utf1.size()); return { utf1, native }; }
			if (sv.starts_with(bom_for_scsu)) { sv.remove_prefix(bom_for_scsu.size()); return { scsu, native }; }
			if (sv.starts_with(bom_for_bocu1)) { sv.remove_prefix(bom_for_bocu1.size()); return { bocu1, native }; }
			if (sv.starts_with(bom_for_utf16_be)) { sv.remove_prefix(bom_for_utf16_be.size()); return { utf16, big }; }
			if (sv.starts_with(bom_for_utf16_le)) { sv.remove_prefix(bom_for_utf16_le.size()); return { utf16, little }; }
		}
		return unknown_text_encoding;
	}

	inline text_encoding consume_bom(stringable16 auto& sv)
	{
		static constexpr std::string_view bom_for_utf16_be{ "\xFE\xFF", 2 };
		static constexpr std::string_view bom_for_utf16_le{ "\xFF\xFE", 2 };

		if (!sv.empty())
		{
			const auto as_bytes = std::string_view{ (char*)sv.data(), sv.size() * sizeof(char16_t) };

			if (as_bytes.starts_with(bom_for_utf16_be)) { sv.remove_prefix(1); return { base_text_encoding::utf16, std::endian::big }; }
			if (as_bytes.starts_with(bom_for_utf16_le)) { sv.remove_prefix(1); return { base_text_encoding::utf16, std::endian::little }; }
		}
		return unknown_text_encoding;
	}

	inline text_encoding consume_bom(stringable32 auto& sv)
	{
		static constexpr std::string_view bom_for_utf32_be{ "\xFF\xFE\x00\x00", 4 };
		static constexpr std::string_view bom_for_utf32_le{ "\x00\x00\xFE\xFF", 4 };

		if (!sv.empty())
		{
			const auto as_bytes = std::string_view{ (char*)sv.data(), sv.size() * sizeof(char32_t) };

			if (as_bytes.starts_with(bom_for_utf32_be)) { sv.remove_prefix(1); return { base_text_encoding::utf32, std::endian::big }; }
			if (as_bytes.starts_with(bom_for_utf32_le)) { sv.remove_prefix(1); return { base_text_encoding::utf32, std::endian::little }; }
		}
		return unknown_text_encoding;
	}

	/// Shamelessly stolen from https://github.com/arc80/plywood/
	struct text_decode_result
	{
		enum status : uint8_t {
			unsupported_encoding,
			truncated,
			invalid,
			valid,
		};

		/// The result of the decode
		status status = unsupported_encoding;
		/// The decoded codepoint (or -1 if failed)
		char32_t point = static_cast<char32_t>(-1);
		/// The number of bytes this codepoint takes up in the input string
		uint8_t byte_count = 0;
	};

	/// Represents the [Unicode plane](https://en.wikipedia.org/wiki/Plane_(Unicode)). Value equals the actual number of the unicode plane
	/// \ingroup Unicode
	/// \showinitializer
	enum class unicode_plane
	{
		/// Represents an invalid plane number
		invalid = -1,
		basic_multilingual_plane,
		supplementary_multilingual_plane,
		supplementary_ideographic_plane,
		tertiary_ideographic_plane,
		supplementary_special_purpose_plane = 14,
		private_use_plane_a,
		private_use_plane_b,

		bmp = basic_multilingual_plane,
		smp = supplementary_multilingual_plane,
		sip = supplementary_ideographic_plane,
		tip = tertiary_ideographic_plane,
		ssp = supplementary_special_purpose_plane,
		spua_a = private_use_plane_a, pup_a = spua_a,
		spua_b = private_use_plane_b, pup_b = spua_b,
	};

	constexpr inline char32_t last_unicode_code_point = 0x10FFFF;
	constexpr inline char32_t first_unicode_high_surrogate = 0xD800;
	constexpr inline char32_t last_unicode_high_surrogate = 0xDBFF;
	constexpr inline char32_t first_unicode_low_surrogate = 0xDC00;
	constexpr inline char32_t last_unicode_low_surrogate = 0xDFFF;

	constexpr bool is_high_surrogate(char32_t cp) noexcept { return cp >= first_unicode_high_surrogate && cp <= last_unicode_high_surrogate; }
	constexpr bool is_low_surrogate(char32_t cp) noexcept { return cp >= first_unicode_low_surrogate && cp <= last_unicode_low_surrogate; }
	constexpr bool is_surrogate(char32_t cp) noexcept { return cp >= first_unicode_high_surrogate && cp <= last_unicode_low_surrogate; }
	constexpr bool is_unicode(char32_t cp) noexcept { return cp <= last_unicode_code_point; }
	constexpr auto get_unicode_plane(char32_t cp) noexcept -> unicode_plane { return is_unicode(cp) ? unicode_plane(cp >> 16) : unicode_plane::invalid; }
	constexpr bool is_unicode_character(char32_t cp) noexcept { return is_unicode(cp) && ((cp & 0xFFFE) != 0xFFFE) && !(cp >= 0xFDD0 && cp <= 0xFDEF); }

	constexpr char32_t surrogate_pair_to_codepoint(char32_t high, char32_t low) noexcept
	{
		return 0x10000 + ((high - first_unicode_high_surrogate) << 10) + (low - first_unicode_low_surrogate);
	}

	constexpr std::pair<char32_t, char32_t> codepoint_to_surrogate_pair(char32_t cp) noexcept
	{
		return { ((cp - 0x10000) >> 10) + first_unicode_high_surrogate, ((cp - 0x10000) & 0x3FF) + first_unicode_low_surrogate };
	}

	namespace detail
	{
		template <std::integral T>
		T get_val(const void* source, std::endian source_endianness)
		{
			T result{};
			std::memcpy(&result, source, sizeof(T));
			if (std::endian::native != source_endianness)
				return byteswap(result);
			return result;
		}
	}

	[[nodiscard]] constexpr size_t codepoint_utf8_count(char32_t cp) noexcept
	{
		constexpr size_t lut[33] = { 7,6,6,6,6,6,5,5,5,5,5,4,4,4,4,4,3,3,3,3,3,2,2,2,2,1,1,1,1,1,1,1,1 };
		return lut[std::countl_zero(std::bit_cast<uint32_t>(cp))];
	}

	inline text_decode_result decode_codepoint(stringable8 auto _str, text_encoding encoding)
	{
		auto sv = make_sv(_str);
		if (sv.empty())
			return { text_decode_result::truncated };

		switch (encoding.base_encoding)
		{
		case base_text_encoding::utf8:
		{
			const auto first = std::bit_cast<uint8_t>(sv[0]);
			if (first < 0x80)
				return { text_decode_result::valid, first, 1 };
			
			char32_t value = 0;
			uint8_t length = 0;
			if ((first >> 5) == 0x6) { length = 2; value = first & 0x1F; }
			else if ((first >> 4) == 0xe) { length = 3; value = first & 0xF; }
			else if ((first >> 3) == 0x1e) { length = 4; value = first & 0x7; }

			if (length == 0 || sv.size() < length)
				return { text_decode_result::truncated, first, 1 };

			switch (length)
			{
			case 4:
				if ((sv[3] & 0xC0) != 0x80)
					return { text_decode_result::invalid, first, 1 };
				value = (value << 6) | (sv[3] & 0x3f);
			case 3:
				if ((sv[2] & 0xC0) != 0x80)
					return { text_decode_result::invalid, first, 1 };
				value = (value << 6) | (sv[2] & 0x3f);
			case 2:
				if ((sv[1] & 0xC0) != 0x80)
					return { text_decode_result::invalid, first, 1 };
				value = (value << 6) | (sv[1] & 0x3f);
				return { text_decode_result::valid, value, length };
			}
		}
		case base_text_encoding::utf16:
		{
			if (sv.size() < 2)
				return { text_decode_result::truncated };

			const uint16_t first = detail::get_val<uint16_t>(sv.data(), encoding.endianness);
			if (is_surrogate(first))
			{
				if (is_high_surrogate(first))
				{
					if (sv.size() < 4)
						return { text_decode_result::truncated, first, 2 };

					const uint16_t second = detail::get_val<uint16_t>(sv.data() + 2, encoding.endianness);
					if (is_low_surrogate(second))
						return { text_decode_result::valid, surrogate_pair_to_codepoint(first, second), 4 };
				}

				return { text_decode_result::invalid, first, 2 };
			}

			return { text_decode_result::valid, first, 2 };
		}
		case base_text_encoding::utf32:
		{
			if (sv.size() < 4)
				return { text_decode_result::truncated };

			const uint32_t cp = detail::get_val<uint32_t>(sv.data(), encoding.endianness);
			if (is_unicode(cp))
				return { text_decode_result::valid, cp, 4 };
			return { text_decode_result::invalid, cp, 4 };
		}
		default:
			return {};
		}
	}

	inline text_encoding detect_encoding(stringable8 auto str)
	{
		struct TextFileStats {
			size_t points = 0;
			size_t valid_points = 0;
			size_t control_points = 0; /// non-whitespace points < 32, including nulls
			size_t plain_ascii = 0; /// includes whitespace, excludes control characters < 32
			size_t whitespace = 0;
			size_t extended_codepoints = 0;
			float one_over_points = 0.f;
			size_t invalid_points() const { return points - valid_points; }
			float score() const { return (2.5f * whitespace + plain_ascii - 100.f * invalid_points() - 50.f * control_points + 5.f * extended_codepoints) * one_over_points; }
		};

		static const auto calculate_stats = [](TextFileStats& stats, stringable8 auto sv, text_encoding encoding) {
			size_t numBytes = 0;
			while (!sv.empty())
			{
				text_decode_result decoded = decode_codepoint(sv, encoding);
				if (decoded.status == text_decode_result::truncated)
					break;
				sv.remove_prefix(decoded.byte_count);
				numBytes += decoded.byte_count;
				stats.points++;
				if (decoded.status == text_decode_result::valid)
				{
					stats.valid_points++;
					if (decoded.point < 32)
					{
						if (decoded.point == '\n' || decoded.point == '\t')
						{
							stats.plain_ascii++;
							stats.whitespace++;
						}
						else if (decoded.point == '\r')
							stats.plain_ascii++;
						else
							stats.control_points++;
					}
					else if (decoded.point < 127)
					{
						stats.plain_ascii++;
						if (decoded.point == ' ')
							stats.whitespace++;
					}
					else if (decoded.point >= 65536)
						stats.extended_codepoints++;
				}
			}
			if (stats.points > 0)
				stats.one_over_points = 1.f / stats.points;
			return numBytes;
		};

		auto sv = make_sv(str);

		if (sv.empty())
			return unknown_text_encoding;

		if (text_encoding encoding = consume_bom(sv); encoding != unknown_text_encoding)
			return encoding;

		sv = sv.substr(0, 4096);

		TextFileStats stats8;

		/// Try UTF8 first:
		size_t numBytesRead = calculate_stats(stats8, sv, utf8_encoding);
		if (numBytesRead == 0)
			return utf8_encoding;

		/// No UTF-8 encoding errors, and no weird control characters/nulls. Pick UTF-8.
		if (stats8.invalid_points() == 0 && stats8.control_points == 0)
			return utf8_encoding;

		/// If more than 20% of the high bytes in UTF-8 are encoding errors, reinterpret UTF-8 as just
		/// bytes.
		text_encoding encoding8 = utf8_encoding;
		if (const auto numHighBytes = numBytesRead - stats8.plain_ascii - stats8.control_points; 
			stats8.invalid_points() >= numHighBytes * 0.2f)
		{
			/// Too many UTF-8 errors. Consider it bytes.
			encoding8 = unknown_text_encoding;
			stats8.points = numBytesRead;
			stats8.valid_points = numBytesRead;
		}

		/// Examine both UTF16 endianness:
		TextFileStats stats16_le;
		calculate_stats(stats16_le, sv, utf16_le_encoding);

		TextFileStats stats16_be;
		calculate_stats(stats16_be, sv, utf16_be_encoding);

		/// Choose the better UTF16 candidate:
		TextFileStats* stats16 = &stats16_le;
		text_encoding encoding16 = utf16_le_encoding;
		if (stats16_be.score() > stats16_le.score())
		{
			stats16 = &stats16_be;
			encoding16 = utf16_be_encoding;
		}

		/// Examine both UTF32 endianness:
		TextFileStats stats32_le;
		calculate_stats(stats32_le, sv, utf32_le_encoding);

		TextFileStats stats32_be;
		calculate_stats(stats32_be, sv, utf32_be_encoding);

		/// Choose the better UTF32 candidate:
		TextFileStats* stats32 = &stats32_le;
		text_encoding encoding32 = utf32_le_encoding;
		if (stats32_be.score() > stats32_le.score())
		{
			stats32 = &stats32_be;
			encoding32 = utf32_be_encoding;
		}
		
		/// Choose best scoring encoding
		const auto score8 = stats8.score();
		const auto score16 = stats16->score();
		const auto score32 = stats32->score();
		if (score8 >= score32)
		{
			if (score16 >= score8)
				return encoding16;
			return encoding8;
		}
		
		if (score32 >= score16)
			return encoding32;
		return encoding16;
	}


	/// \pre `str` must be valid UTF-8
#ifndef __clang__
	[[gsl::suppress(type.1, es.79)]]
#else
	[[gsl::suppress("type.1", "es.79")]]
#endif
	[[nodiscard]] constexpr char32_t consume_utf8(string_view8 auto& str)
	{
		using char_type = typename std::remove_cvref_t<decltype(str)>::value_type;
		using unsigned_char_type = std::make_unsigned_t<char_type>;

		if (str.empty()) return 0;
		auto it = std::to_address(str.begin());
		char32_t cp = static_cast<unsigned_char_type>(*it);

		int length = 0;
		if (cp < 0x80) length = 1;
		else if ((cp >> 5) == 0x6)  length = 2;
		else if ((cp >> 4) == 0xe)  length = 3;
		else if ((cp >> 3) == 0x1e) length = 4;
		else return 0;

		switch (length) {
		case 2:
			++it; cp = ((cp << 6) & 0x7ff) + (static_cast<unsigned_char_type>(*it) & 0x3f);
			break;
		case 3:
			++it; cp = ((cp << 12) & 0xffff) + ((static_cast<unsigned_char_type>(*it) << 6) & 0xfff);
			++it; cp += static_cast<unsigned_char_type>(*it) & 0x3f;
			break;
		case 4:
			++it; cp = ((cp << 18) & 0x1fffff) + ((static_cast<unsigned_char_type>(*it) << 12) & 0x3ffff);
			++it; cp += (static_cast<unsigned_char_type>(*it) << 6) & 0xfff;
			++it; cp += static_cast<unsigned_char_type>(*it) & 0x3f;
			break;
		}
		str.remove_prefix(length);
		return cp;
	}

	constexpr size_t count_utf8_codepoints(stringable8 auto str)
	{
		using char_type = typename std::remove_cvref_t<decltype(str)>::value_type;
		using unsigned_char_type = std::make_unsigned_t<char_type>;

		auto it = std::to_address(str.begin());
		const auto end = std::to_address(str.end());

		size_t result = 0;
		while (it < end)
		{
			char32_t cp = static_cast<unsigned_char_type>(*it);

			int length = 1;
			if ((cp >> 5) == 0x6)  length = 2;
			else if ((cp >> 4) == 0xe)  length = 3;
			else if ((cp >> 3) == 0x1e) length = 4;

			it += length;
			result++;
		}
		return result;
	}


	/// \pre `str` must be valid UTF-8
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	constexpr size_t append_utf8(string8 auto& buffer, char32_t cp)
	{
		using char_type = typename std::remove_cvref_t<decltype(buffer)>::value_type;
#if 1
		const size_t cp_bytes = codepoint_utf8_count(cp);
		std::decay_t<decltype(buffer)> bytes(cp_bytes, 0);
		switch (cp_bytes) 
		{
		case 7: bytes[cp_bytes - 6] = static_cast<char_type>(0x80 | ((cp >> 30) & 0x3F)); [[fallthrough]];
		case 6: bytes[cp_bytes - 5] = static_cast<char_type>(0x80 | ((cp >> 24) & 0x3F)); [[fallthrough]];
		case 5: bytes[cp_bytes - 4] = static_cast<char_type>(0x80 | ((cp >> 18) & 0x3F)); [[fallthrough]];
		case 4: bytes[cp_bytes - 3] = static_cast<char_type>(0x80 | ((cp >> 12) & 0x3F)); [[fallthrough]];
		case 3: bytes[cp_bytes - 2] = static_cast<char_type>(0x80 | ((cp >> 6) & 0x3F)); [[fallthrough]];
		case 2: bytes[cp_bytes - 1] = static_cast<char_type>(0x80 | ((cp >> 0) & 0x3F)); bytes[0] = static_cast<char_type>((std::uint_least16_t(0xFF00uL) >> cp_bytes) | (uint64_t(cp) >> (6 * cp_bytes - 6))); break;
		case 1: bytes[0] = static_cast<char_type>(cp); break;
		}
		buffer += bytes;
		return cp_bytes;
#else
		if (cp < 0x80)
		{
			buffer += static_cast<char_type>(cp);
			return 1;
		}
		else if (cp < 0x800)
		{
			buffer += static_cast<char_type>((cp >> 6) | 0xc0);
			buffer += static_cast<char_type>((cp & 0x3f) | 0x80);
			return 2;
		}
		else if (cp < 0x10000)
		{
			buffer += static_cast<char_type>((cp >> 12) | 0xe0);
			buffer += static_cast<char_type>(((cp >> 6) & 0x3f) | 0x80);
			buffer += static_cast<char_type>((cp & 0x3f) | 0x80);
			return 3;
		}
		else
		{
			buffer += static_cast<char_type>((cp >> 18) | 0xf0);
			buffer += static_cast<char_type>(((cp >> 12) & 0x3f) | 0x80);
			buffer += static_cast<char_type>(((cp >> 6) & 0x3f) | 0x80);
			buffer += static_cast<char_type>((cp & 0x3f) | 0x80);
			return 4;
		}
#endif
	}

	constexpr void transcode_codepage_to_utf8(string8 auto& dest, stringable8 auto source, std::span<char32_t const, 128> codepage_map)
	{
		using dest_char = typename std::decay_t<decltype(dest)>::value_type;
		for (uint8_t cp : source)
		{
			if (cp < 0x80)
				dest += static_cast<dest_char>(cp);
			else
				append_utf8(dest, codepage_map[cp - 0x80]);
		}
	}

	template <string8 T>
	constexpr auto transcode_codepage_to_utf8(stringable8 auto source, std::span<char32_t const, 128> codepage_map) -> T
	{
		T result{};
		transcode_codepage_to_utf8(result, source, codepage_map);
		return result;
	}

	/// \pre `str` must be valid UTF-16
#ifndef __clang__
	[[gsl::suppress(type.1, es.79)]]
#else
	[[gsl::suppress("type.1", "es.79")]]
#endif
	[[nodiscard]] constexpr char32_t consume_utf16(string_view16 auto& str)
	{
		using char_type = typename std::remove_cvref_t<decltype(str)>::value_type;
		using unsigned_char_type = std::make_unsigned_t<char_type const>;

		if (str.empty()) return 0;
		auto it = (unsigned_char_type*)std::to_address(str.begin());
		char32_t cp = *it;

		const int length = int(cp >= 0xD800 && cp <= 0xDBFF) + 1;

		if (length == 2)
		{
			++it;
			cp = ((cp - 0xD800) << 10) | (*it - 0xDC00);
		}
		str.remove_prefix(length);
		return cp;
	}

	[[nodiscard]] constexpr char32_t consume_utf32(string_view32 auto& str)
	{
		if (str.empty()) return 0;
		const auto result = str[0];
		str.remove_prefix(1);
		return result;
	}

	/// \pre `str` must be valid UTF-16
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	constexpr size_t append_utf16(string16 auto& buffer, char32_t cp)
	{
		using char_type = typename std::remove_cvref_t<decltype(buffer)>::value_type;
		if (cp <= 0xFFFF)
		{
			buffer += static_cast<char_type>(cp);
			return 1;
		}

		buffer += static_cast<char_type>((cp >> 10) + 0xD800);
		buffer += static_cast<char_type>((cp & 0x3FF) + 0xDC00);
		return 2;
	}

	constexpr size_t append_utf32(string32 auto& buffer, char32_t cp)
	{
		buffer += cp;
		return 1;
	}

	template <string8 T>
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	/// \pre `cp` must be a valid Unicode codepoint
	[[nodiscard]] constexpr T to_utf8(char32_t cp)
	{
		using char_type = typename T::value_type;
		if (cp < 0x80)
			return { static_cast<char_type>(cp) };
		else if (cp < 0x800)
			return { static_cast<char_type>((cp >> 6) | 0xc0), static_cast<char_type>((cp & 0x3f) | 0x80) };
		else if (cp < 0x10000)
			return { static_cast<char_type>((cp >> 12) | 0xe0), static_cast<char_type>(((cp >> 6) & 0x3f) | 0x80), static_cast<char_type>((cp & 0x3f) | 0x80) };
		else
			return { static_cast<char_type>((cp >> 18) | 0xf0), static_cast<char_type>(((cp >> 12) & 0x3f) | 0x80), static_cast<char_type>(((cp >> 6) & 0x3f) | 0x80), static_cast<char_type>((cp & 0x3f) | 0x80) };
	}

	/// \pre `str` must be valid UTF-8
	template <string8 RESULT, stringable8 STR>
	[[nodiscard]] constexpr RESULT to_utf8(STR&& str)
	{
		if constexpr (std::same_as<STR, RESULT>)
			return std::forward<STR>(str);
		else
		{
			using char_type = typename RESULT::value_type;
			return RESULT{ string_view_cast<char_type>(make_sv(std::forward<STR>(str))) };
		}
	}

	/// \pre `str` must be valid UTF-16
	template <string8 RESULT, stringable16 STR>
	[[nodiscard]] constexpr RESULT to_utf8(STR&& str)
	{
		RESULT result{};
		auto sv = make_sv(str);
		while (!sv.empty())
			append_utf8(result, consume_utf16(sv));
		return result;
	}

	/// \pre `str` must be valid UTF-16
	template <string8 RESULT, stringable32 STR>
	[[nodiscard]] constexpr RESULT to_utf8(STR&& str)
	{
		RESULT result{};
		auto sv = make_sv(str);
		while (!sv.empty())
			append_utf8(result, consume_utf32(sv));
		return result;
	}

	/// \pre `str` must be valid UTF-16
	[[nodiscard]] inline std::string to_string(std::wstring_view str)
	{
		return to_utf8<std::string>(str);
	}

	template <string16 T>
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	/// \pre `cp` must be a valid Unicode codepoint
	[[nodiscard]] constexpr T to_utf16(char32_t cp)
	{
		using char_type = T::value_type;
		if (cp <= 0xFFFF)
			return { static_cast<char_type>(cp) };
		else
			return { static_cast<char_type>((cp >> 10) + 0xD800), static_cast<char_type>((cp & 0x3FF) + 0xDC00) };
	}

	/// \pre `str` must be valid UTF-8
	template <string16 T, stringable8 STR>
	[[nodiscard]] constexpr T to_utf16(STR str)
	{
		T result{};
		auto sv = make_sv(str);
		while (!sv.empty())
			append_utf16(result, consume_utf8(sv));
		return result;
	}

	/// \pre `str` must be valid UTF-8
	[[nodiscard]] inline std::wstring to_wstring(std::string_view str)
	{
		return to_utf16<std::wstring>(str);
	}

	template <std::ranges::view R>
	struct utf8_view : public std::ranges::view_interface<utf8_view<R>>
	{
		template <typename RANGE_ITER, typename SENTINEL>
		struct utf8_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = char32_t;
			using difference_type = ptrdiff_t;
			using reference = char32_t;

			constexpr utf8_iterator(RANGE_ITER current, SENTINEL end) : mCurrent(std::move(current)), mEnd(std::move(end)) {}

			[[nodiscard]] constexpr value_type operator*() const {
				const auto length = len();
				auto it = mCurrent;
				char32_t cp = std::bit_cast<uint8_t>(*it);

				switch (length) {
				case 2:
					++it; cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
					break;
				case 3:
					++it; cp = ((cp << 12) & 0xffff) + ((*it << 6) & 0xfff);
					++it; cp += (*it) & 0x3f;
					break;
				case 4:
					++it; cp = ((cp << 18) & 0x1fffff) + (((*it) << 12) & 0x3ffff);
					++it; cp += ((*it) << 6) & 0xfff;
					++it; cp += (*it) & 0x3f;
					break;
				}
				return cp;
			}

			constexpr utf8_iterator& operator++() {
				std::advance(mCurrent, len());
				return *this;
			}

			constexpr utf8_iterator operator++(int) {
				auto copy = *this;
				++* this;
				return copy;
			}

			constexpr auto operator<=>(utf8_iterator const&) const noexcept = default;

		private:

			size_t len() const
			{
				if (mCurrent >= mEnd)
					throw std::out_of_range("utf8 iterator out of range");

				const unsigned cp = std::bit_cast<uint8_t>(*mCurrent);
				size_t length = 0;
				if (cp < 0x80) length = 1;
				else if ((cp >> 5) == 0x6) length = 2;
				else if ((cp >> 4) == 0xe) length = 3;
				else if ((cp >> 3) == 0x1e) length = 4;
				else
					throw std::runtime_error("invalid utf-8 prefix");

				if (mCurrent + length > mEnd)
					throw std::runtime_error("utf-8 range contains codepoint with length beyond end of range");

				return length;
			}

			RANGE_ITER mCurrent;
			SENTINEL mEnd;
		};

		utf8_view() = default;

		constexpr utf8_view(R base)
			: mBase(base)
		{
		}

		template <typename... ARGS>
		requires std::constructible_from<std::string_view, ARGS...>
		explicit constexpr utf8_view(ARGS&&... args)
			: mBase(std::forward<ARGS>(args)...)
		{
		}

		constexpr R base() const&
		{
			return mBase;
		}
		constexpr R base()&&
		{
			return std::move(mBase);
		}

		constexpr auto begin() const
		{
			return utf8_iterator<decltype(std::begin(mBase)), decltype(std::end(mBase))>{std::begin(mBase), std::end(mBase)};
		}

		constexpr auto end() const
		{
			return utf8_iterator<decltype(std::end(mBase)), decltype(std::end(mBase))>{std::end(mBase), std::end(mBase)};
		}

	private:
		R mBase{};
	};

}