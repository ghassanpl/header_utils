/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "string_ops.h"

namespace ghassanpl::string_ops
{
	/// fwd

	constexpr bool is_high_surrogate(char32_t cp) noexcept;
	constexpr bool is_low_surrogate(char32_t cp) noexcept;
	constexpr bool is_surrogate(char32_t cp) noexcept;
	constexpr bool is_unicode(char32_t cp) noexcept;
	constexpr bool is_unicode_character(char32_t cp) noexcept;
	
	constexpr char32_t surrogate_pair_to_codepoint(char32_t high, char32_t low) noexcept;

	enum class unicode_plane;
	constexpr auto get_unicode_plane(char32_t cp) noexcept -> unicode_plane;

	struct text_encoding;
	/// constexpr inline text_encoding utf8_encoding;
	/// constexpr inline text_encoding utf16_le_encoding;
	/// constexpr inline text_encoding utf16_be_encoding;
	/// constexpr inline text_encoding utf32_le_encoding;
	/// constexpr inline text_encoding utf32_be_encoding;
	/// constexpr inline text_encoding unknown_text_encoding;

	/// https://bjoern.hoehrmann.de/utf-8/decoder/dfa/

	constexpr size_t codepoint_utf8_count(char32_t cp) noexcept;

	struct text_decode_result;
	text_decode_result decode_codepoint(stringable8 auto _str, text_encoding encoding);

	text_encoding consume_bom(stringable8 auto& sv);
	text_encoding consume_bom(stringable16 auto& sv);
	text_encoding consume_bom(stringable32 auto& sv);
	
	text_encoding detect_encoding(stringable8 auto str);

	constexpr char32_t consume_utf8(string_view8 auto& str);
	size_t append_utf8(string8 auto& buffer, char32_t cp);
	template <string8 RESULT>
	constexpr RESULT to_utf8(char32_t cp);
	template <string8 RESULT, stringable16 STR>
	RESULT to_utf8(STR str);
	std::string to_string(std::wstring_view str);

	template <std::ranges::view R>
	struct utf8_view;

	constexpr char32_t consume_utf16(string_view16 auto& str);
	size_t append_utf16(string16 auto& buffer, char32_t cp);
	template <string16 RESULT>
	RESULT to_utf16(char32_t cp);
	template <string16 RESULT, stringable8 STR>
	RESULT to_utf16(STR str);
	std::wstring to_wstring(std::string_view str);

	void transcode_codepage_to_utf8(string8 auto& dest, stringable8 auto source, std::span<char32_t const, 128> codepage_map);
	template <string8 T = std::string>
	auto transcode_codepage_to_utf8(stringable8 auto source, std::span<char32_t const, 128> codepage_map) -> T;


	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///
	/// Implementation
	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ///

	enum class base_text_encoding
	{
		unknown,
		utf8,
		utf16,
		utf32,
		utf7,
		utf1,
		utf_ebcdic,
		scsu,
		bocu1,
		gb18030,
	};

	struct text_encoding
	{
		base_text_encoding base_encoding;
		std::endian endianness;

		[[nodiscard]] constexpr bool operator==(text_encoding const& other) const noexcept = default;
		[[nodiscard]] constexpr auto operator<=>(text_encoding const& other) const noexcept = default;
	};

	constexpr inline text_encoding utf8_encoding = { base_text_encoding::utf8, std::endian::native };
	constexpr inline text_encoding utf16_le_encoding = { base_text_encoding::utf16, std::endian::little };
	constexpr inline text_encoding utf16_be_encoding = { base_text_encoding::utf16, std::endian::big };
	constexpr inline text_encoding utf32_le_encoding = { base_text_encoding::utf32, std::endian::little };
	constexpr inline text_encoding utf32_be_encoding = { base_text_encoding::utf32, std::endian::big };
	constexpr inline text_encoding unknown_text_encoding = { base_text_encoding::unknown, std::endian::native };

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
			if (sv.starts_with(bom_for_gb18030)) { sv.remove_prefix(bom_for_gb18030.size()); return { base_text_encoding::gb18030, std::endian::native }; }
			if (sv.starts_with(bom_for_utf32_be)) { sv.remove_prefix(bom_for_utf32_be.size()); return { base_text_encoding::utf32, std::endian::big }; }
			if (sv.starts_with(bom_for_utf32_le)) { sv.remove_prefix(bom_for_utf32_le.size()); return { base_text_encoding::utf32, std::endian::little }; }
			if (sv.starts_with(bom_for_utf_ebcdic)) { sv.remove_prefix(bom_for_utf_ebcdic.size()); return { base_text_encoding::utf_ebcdic, std::endian::native }; }
			if (sv.starts_with(bom_for_utf8)) { sv.remove_prefix(bom_for_utf8.size()); return { base_text_encoding::utf8, std::endian::native }; }
			if (sv.starts_with(bom_for_utf7)) { sv.remove_prefix(bom_for_utf7.size()); return { base_text_encoding::utf7, std::endian::native }; }
			if (sv.starts_with(bom_for_utf1)) { sv.remove_prefix(bom_for_utf1.size()); return { base_text_encoding::utf1, std::endian::native }; }
			if (sv.starts_with(bom_for_scsu)) { sv.remove_prefix(bom_for_scsu.size()); return { base_text_encoding::scsu, std::endian::native }; }
			if (sv.starts_with(bom_for_bocu1)) { sv.remove_prefix(bom_for_bocu1.size()); return { base_text_encoding::bocu1, std::endian::native }; }
			if (sv.starts_with(bom_for_utf16_be)) { sv.remove_prefix(bom_for_utf16_be.size()); return { base_text_encoding::utf16, std::endian::big }; }
			if (sv.starts_with(bom_for_utf16_le)) { sv.remove_prefix(bom_for_utf16_le.size()); return { base_text_encoding::utf16, std::endian::little }; }
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

		status status = unsupported_encoding;
		char32_t point = static_cast<char32_t>(-1);
		uint8_t byte_count = 0;
	};

	enum class unicode_plane
	{
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

	constexpr inline bool is_high_surrogate(char32_t cp) noexcept { return cp >= first_unicode_high_surrogate && cp <= last_unicode_high_surrogate; }
	constexpr inline bool is_low_surrogate(char32_t cp) noexcept { return cp >= first_unicode_low_surrogate && cp <= last_unicode_low_surrogate; }
	constexpr inline bool is_surrogate(char32_t cp) noexcept { return cp >= first_unicode_high_surrogate && cp <= last_unicode_low_surrogate; }
	constexpr inline bool is_unicode(char32_t cp) noexcept { return cp <= last_unicode_code_point; }
	constexpr inline auto get_unicode_plane(char32_t cp) noexcept -> unicode_plane { return is_unicode(cp) ? unicode_plane(cp >> 16) : unicode_plane::invalid; }
	constexpr inline bool is_unicode_character(char32_t cp) noexcept { return is_unicode(cp) && ((cp & 0xFFFE) != 0xFFFE) && !(cp >= 0xFDD0 && cp <= 0xFDEF); }

	constexpr inline char32_t surrogate_pair_to_codepoint(char32_t high, char32_t low) noexcept
	{
		return 0x10000 + ((high - first_unicode_high_surrogate) << 10) + (low - first_unicode_low_surrogate);
	}

	constexpr inline std::pair<char32_t, char32_t> codepoint_to_surrogate_pair(char32_t cp) noexcept
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
				return std::byteswap(result);
			return result;
		}
	}

	[[nodiscard]] constexpr inline size_t codepoint_utf8_count(char32_t cp) noexcept
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

		static const auto scanTextFile = [](TextFileStats& stats, stringable8 auto sv, text_encoding encoding) -> size_t {
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
		size_t numBytesRead = scanTextFile(stats8, sv, utf8_encoding);
		if (numBytesRead == 0)
			return utf8_encoding;

		/// No UTF-8 encoding errors, and no weird control characters/nulls. Pick UTF-8.
		if (stats8.invalid_points() == 0 && stats8.control_points == 0)
			return utf8_encoding;

		/// If more than 20% of the high bytes in UTF-8 are encoding errors, reinterpret UTF-8 as just
		/// bytes.
		text_encoding encoding8 = utf8_encoding;
		auto numHighBytes = numBytesRead - stats8.plain_ascii - stats8.control_points;
		if (stats8.invalid_points() >= numHighBytes * 0.2f)
		{
			/// Too many UTF-8 errors. Consider it bytes.
			encoding8 = unknown_text_encoding;
			stats8.points = numBytesRead;
			stats8.valid_points = numBytesRead;
		}

		/// Examine both UTF16 endianness:
		TextFileStats stats16_le;
		scanTextFile(stats16_le, sv, utf16_le_encoding);

		TextFileStats stats16_be;
		scanTextFile(stats16_be, sv, utf16_be_encoding);

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
		scanTextFile(stats32_le, sv, utf32_le_encoding);

		TextFileStats stats32_be;
		scanTextFile(stats32_be, sv, utf32_be_encoding);

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

	/// Assuming str is valid UTF-8

#ifndef __clang__
	[[gsl::suppress(type.1, es.79)]]
#else
	[[gsl::suppress("type.1", "es.79")]]
#endif
	[[nodiscard]] constexpr inline char32_t consume_utf8(string_view8 auto& str)
	{
		using char_type = std::remove_cvref_t<decltype(str)>::value_type;
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

	/*
	/// Assuming str is valid UTF-8
#ifndef __clang__
	[[gsl::suppress(type.1, es.79)]]
#else
	[[gsl::suppress("type.1", "es.79")]]
#endif
	[[nodiscard]] constexpr inline char32_t consume_utf8(string_view8 auto str)
	{
		return consume_utf8(str);
	}
	*/

	/// Assuming codepoint is valid
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	inline size_t append_utf8(string8 auto& buffer, char32_t cp)
	{
		using char_type = std::remove_cvref_t<decltype(buffer)>::value_type;
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
		case 2: bytes[cp_bytes - 1] = static_cast<char_type>(0x80 | ((cp >> 0) & 0x3F)); bytes[0] = static_cast<char_type>((std::uint_least16_t(0xFF00uL) >> cp_bytes) | (cp >> (6 * cp_bytes - 6))); break;
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

	inline void transcode_codepage_to_utf8(string8 auto& dest, stringable8 auto source, std::span<char32_t const, 128> codepage_map)
	{
		using dest_char = std::decay_t<decltype(dest)>::value_type;
		using source_char = std::decay_t<decltype(source)>::value_type;
		for (uint8_t cp : source)
		{
			if (cp < 0x80)
				dest += static_cast<dest_char>(cp);
			else
				append_utf8(dest, codepage_map[cp - 0x80]);
		}
	}

	template <string8 T>
	inline auto transcode_codepage_to_utf8(stringable8 auto source, std::span<char32_t const, 128> codepage_map) -> T
	{
		T result{};
		transcode_codepage_to_utf8(result, source, codepage_map);
		return result;
	}

	/// Assuming str is valid UTF-16
#ifndef __clang__
	[[gsl::suppress(type.1, es.79)]]
#else
	[[gsl::suppress("type.1", "es.79")]]
#endif
	[[nodiscard]] constexpr inline char32_t consume_utf16(string_view16 auto& str)
	{
		using char_type = std::remove_cvref_t<decltype(str)>::value_type;
		using unsigned_char_type = std::make_unsigned_t<char_type>;

		if (str.empty()) return 0;
		auto it = (unsigned_char_type*)std::to_address(str.begin());
		char32_t cp = *it;

		const int length = (cp >= 0xD800 && cp <= 0xDBFF) + 1;

		if (length == 2)
		{
			++it;
			cp = ((cp - 0xD800) << 10) | (*it - 0xDC00);
		}
		str.remove_prefix(length);
		return cp;
	}

	/// Assuming codepoint is valid
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	inline size_t append_utf16(string16 auto& buffer, char32_t cp)
	{
		using char_type = std::remove_cvref_t<decltype(buffer)>::value_type;
		if (cp <= 0xFFFF)
		{
			buffer += static_cast<char_type>(cp);
			return 1;
		}

		buffer += static_cast<char_type>((cp >> 10) + 0xD800);
		buffer += static_cast<char_type>((cp & 0x3FF) + 0xDC00);
		return 2;
	}

	template <string8 T>
#ifndef __clang__
	[[gsl::suppress(type.1)]]
#else
	[[gsl::suppress("type.1")]]
#endif
	/// Assumes codepoint is valid
	[[nodiscard]] constexpr inline T to_utf8(char32_t cp)
	{
		using char_type = T::value_type;
		if (cp < 0x80)
			return { static_cast<char_type>(cp) };
		else if (cp < 0x800)
			return { static_cast<char_type>((cp >> 6) | 0xc0), static_cast<char_type>((cp & 0x3f) | 0x80) };
		else if (cp < 0x10000)
			return { static_cast<char_type>((cp >> 12) | 0xe0), static_cast<char_type>(((cp >> 6) & 0x3f) | 0x80), static_cast<char_type>((cp & 0x3f) | 0x80) };
		else
			return { static_cast<char_type>((cp >> 18) | 0xf0), static_cast<char_type>(((cp >> 12) & 0x3f) | 0x80), static_cast<char_type>(((cp >> 6) & 0x3f) | 0x80), static_cast<char_type>((cp & 0x3f) | 0x80) };
	}

	/// Assumes codepoint is valid
	template <string8 RESULT, stringable16 STR>
	[[nodiscard]] inline RESULT to_utf8(STR str)
	{
		RESULT result{};
		auto sv = make_sv(str);
		while (!sv.empty())
			append_utf8(result, consume_utf16(sv));
		return result;
	}

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
	/// Assumes codepoint is valid
	[[nodiscard]] inline T to_utf16(char32_t cp)
	{
		using char_type = T::value_type;
		if (cp <= 0xFFFF)
			return { static_cast<char_type>(cp) };
		else
			return { static_cast<char_type>((cp >> 10) + 0xD800), static_cast<char_type>((cp & 0x3FF) + 0xDC00) };
	}

	/// Assumes codepoint is valid
	template <string16 T, stringable8 STR>
	[[nodiscard]] inline T to_utf16(STR str)
	{
		T result{};
		auto sv = make_sv(str);
		while (!sv.empty())
			append_utf16(result, consume_utf8(sv));
		return result;
	}

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
			constexpr bool operator==(utf8_iterator const&) const noexcept = default;
			constexpr bool operator!=(utf8_iterator const&) const noexcept = default;

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
		requires requires { std::string_view{ std::declval<ARGS>()... }; }
		constexpr utf8_view(ARGS&&... args)
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

	//template<class R> custom_take_view(R&& base, std::iter_difference_t<rg::iterator_t<R>>) ->custom_take_view<rg::views::all_t<R>>;

}