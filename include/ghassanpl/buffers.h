/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "bytes.h"
#include "expected.h"
#include "min-cpp-version/cpp20.h"

namespace ghassanpl
{
	namespace detail
	{
		template <typename BUFFER>
		consteval auto deduce_buffer_element_type() noexcept
		{
			if constexpr (requires { typename BUFFER::value_type; })
				return std::type_identity<typename BUFFER::value_type>{};
			else if constexpr (requires { typename BUFFER::char_type; })
				return std::type_identity<typename BUFFER::char_type>{};
		}
	}

	/// \defgroup Buffers Buffers
	/// Buffers and stuff
	
	/// \ingroup Buffers
	///@{

	/// Resolves to the destination element type of the buffer
	template <typename BUFFER>
	using buffer_element_type = typename decltype(detail::deduce_buffer_element_type<BUFFER>())::type;

	/// Primary function to append a value (preferably the buffer element type) to the buffer
	template <typename BUFFER, typename T>
	bool buffer_append(BUFFER&& buffer, T&& val)
	{
		if constexpr (requires { buffer.append(val); })
		{
			buffer.append(val);
			return true;
		}
		else if constexpr (requires { buffer.push_back(val); })
		{
			buffer.push_back(val);
			return true;
		}
		else if constexpr (requires { buffer.insert(buffer.end(), val); })
		{
			buffer.insert(buffer.end(), val);
			return true;
		}
		else if constexpr (requires { buffer.put(val); }) /// for ostreams
		{
			buffer.put(val);
			return true;
		}
		else
		{
			static_assert(!always_false<T>, "buffer cannot be appended with this value type - buffer_append might need to be specialized");
			return false;
		}
	}

	/// Checks if an element value is appendable to a buffer
	template <typename BUFFER, typename ELEMENT_VALUE>
	concept output_buffer = requires (BUFFER buffer, ELEMENT_VALUE val) {
		::ghassanpl::buffer_append(buffer, val);
	};

	/// Reserves **additional** elements in the buffer, if possible.
	template <typename BUFFER>
	bool buffer_reserve(BUFFER& buffer, size_t additional)
	{
		if constexpr (requires { buffer.reserve(std::ranges::size(buffer) + additional); })
		{
			buffer.reserve(std::ranges::size(buffer) + additional);
			return true;
		}
		else
			return false;
	}

	/// Appends elements in the `range` to the `buffer`
	template <typename BUFFER, typename RANGE>
	requires output_buffer<BUFFER, std::ranges::range_value_t<RANGE>>
	size_t buffer_append_range(BUFFER& buffer, RANGE&& range)
	{
		if constexpr (std::ranges::sized_range<RANGE>)
			buffer_reserve(buffer, std::ranges::size(range));

		if constexpr (requires { buffer.append(std::to_address(std::ranges::begin(range)), std::ranges::size(range)); } && std::ranges::sized_range<RANGE>) /// for strings etc
		{
			const auto size = std::ranges::size(range);
			buffer.append(std::to_address(std::ranges::begin(range)), size);
			return size;
		}
		else if constexpr (requires { buffer.write(std::to_address(std::ranges::begin(range)), std::ranges::size(range)); } && std::ranges::sized_range<RANGE>) /// for ostreams
		{
			const auto size = std::ranges::size(range);
			/// const auto pos = buffer.tellp();
			buffer.write(std::to_address(std::ranges::begin(range)), size);
			/// return static_cast<size_t>(buffer.tellp() - pos);
			/// return buffer.size;
			return size;
		}
		else
		{
			size_t count = 0;
			for (auto&& value : std::forward<RANGE>(range))
			{
				if (!buffer_append(buffer, value))
					break;
				++count;
			}
			return count;
		}
	}

	/// Appends characters in the `cstr` to the `buffer`
	template <typename BUFFER, typename CHAR_TYPE>
	requires output_buffer<BUFFER, CHAR_TYPE>
	size_t buffer_append_cstring_ptr(BUFFER& buffer, const CHAR_TYPE* cstr)
	{
		return buffer_append_range(buffer, cstr, cstr + std::char_traits<CHAR_TYPE>::length(cstr));
	}

	/// Appends at most `max_len` characters in the `cstr` to the `buffer`
	template <typename BUFFER, typename CHAR_TYPE>
	requires output_buffer<BUFFER, CHAR_TYPE>
	size_t buffer_append_cstring_ptr(BUFFER& buffer, const CHAR_TYPE* cstr, size_t max_len)
	{
		return buffer_append_range(buffer, cstr, cstr + std::min(max_len, std::char_traits<CHAR_TYPE>::length(cstr)));
	}

	/// Appends characters in the `cstr` literal array to the `buffer`
	template <typename BUFFER, size_t N, typename CHAR_TYPE>
	requires output_buffer<BUFFER, CHAR_TYPE>
	size_t buffer_append_cstring(BUFFER& buffer, const CHAR_TYPE(&cstr)[N])
	{
		return buffer_append_range(buffer, std::span{ cstr, cstr + (N - 1) });
	}

	/// Appends UTF-8 codeunits that represent the Unicode code-point `cp` to the `buffer`. Assumes the codepoint is a valid Unicode codepoint that represents a character.
	template <typename BUFFER, typename ELEMENT_TYPE = buffer_element_type<BUFFER>>
	requires output_buffer<BUFFER, ELEMENT_TYPE>
	size_t buffer_append_varint(BUFFER& buffer, std::integral auto oval)
	{
		auto val = std::bit_cast<std::make_unsigned_t<decltype(oval)>>(oval);
		if constexpr (std::is_signed_v<decltype(val)>)
			val = (oval < 0) ? ((std::bit_cast<std::make_unsigned_t<decltype(oval)>>(-oval) << 1) | 1) : (val << 1);
		size_t result = 0;
		while (val >= 128)
		{
			result += buffer_append(buffer, uint8_t(0x80 | (val & 0x7f)));
			val >>= 7;
		}
		result += buffer_append(buffer, uint8_t(val));
		return result;
	}

	template <typename BUFFER, typename ELEMENT_TYPE = buffer_element_type<BUFFER>>
	requires output_buffer<BUFFER, ELEMENT_TYPE>
	size_t buffer_append_utf8(BUFFER& buffer, char32_t cp)
	{
		size_t result = 0;
		if (cp < 0x80ULL)
		{
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>(cp));
		}
		else if (cp < 0x800ULL) {
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>((cp >> 6) | 0xc0));
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>((cp & 0x3f) | 0x80));
		}
		else if (cp < 0x10000ULL) {
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>((cp >> 12) | 0xe0));
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>(((cp >> 6) & 0x3f) | 0x80));
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>((cp & 0x3f) | 0x80));
		}
		else {
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>((cp >> 18) | 0xf0));
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>(((cp >> 12) & 0x3f) | 0x80));
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>(((cp >> 6) & 0x3f) | 0x80));
			result += buffer_append(buffer, static_cast<ELEMENT_TYPE>((cp & 0x3f) | 0x80));
		}

		return result;
	}

	/// Appends UTF-8 codeunits that represent the UTF-32 range `str` to the `buffer`. Assumes all codepoints in `str` are valid.
	/// \sa buffer_append_utf8(BUFFER& buffer, char32_t cp)
	template <typename BUFFER, typename STRING_TYPE, typename ELEMENT_TYPE = buffer_element_type<BUFFER>>
	requires output_buffer<BUFFER, ELEMENT_TYPE> && std::ranges::range<STRING_TYPE> && std::same_as<std::ranges::range_value_t<STRING_TYPE>, char32_t>
	size_t buffer_append_utf8(BUFFER& buffer, STRING_TYPE&& str)
	{
		size_t count = 0;
		for (auto cp : str)
			count += buffer_append_utf8(buffer, cp);
		return count;
	}

	/// Appends a POD values internal object representation to a buffer.
	/// \note
	/// If, for some reason, you want to append the pod as a certain type of bytelikes
	template <typename BUFFER, typename POD>
	requires std::is_trivially_copyable_v<POD> && bytelike<buffer_element_type<BUFFER>>
	size_t buffer_append_pod(BUFFER& buffer, POD const& pod)
	{
		return buffer_append_range(buffer, ghassanpl::as_bytelikes<buffer_element_type<BUFFER>>(pod));
	}

	/*
	/// TODO: Make this work - currently needs `string_ops`
	template <typename BUFFER, typename CALLBACK>
	requires std::invocable<CALLBACK, size_t, std::string_view, BUFFER&>
	void callback_format_to(BUFFER& buffer, std::string_view fmt, CALLBACK&& callback)
	{
		/// TODO: Make this respect `}}` as an escaped `}`
		size_t aid = 0;
		while (!fmt.empty())
		{
			auto text = consume_until(fmt, '{');
			buffer_append_range(buffer, text);
			if (fmt.empty())
				continue;
			std::ignore = consume(fmt, '{');

			if (consume(fmt, '{'))
			{
				buffer_append_range(buffer, '{');
				continue;
			}
			else
			{
				std::string fmt_clause_str = "{";
				auto fmt_clause = string_ops::consume_until_delim(fmt, '}');
				if (!string_ops::consume_at_end(fmt_clause, '}'))
					throw std::format_error("missing '}' in format string");
				if (!fmt_clause.empty())
				{
					auto num = string_ops::consume_until(fmt_clause, ':');
					if (!num.empty())
						aid = string_ops::stoull(num);
					fmt_clause_str += fmt_clause;
				}
				fmt_clause_str += '}';
				callback(aid, std::string_view{ fmt_clause_str }, buffer);
				++aid;
			}
		}
	}
	*/

	///@}
	
	enum class buffer_compression_error_type
	{
		unknown_error,
		invalid_options,
		//no_input,
		//no_more_space_in_output_buffer,
	};

	struct buffer_compression_error
	{
		buffer_compression_error_type type{};
		std::string description;
	};

	template <typename T>
	using buffer_compression_result = expected<T, buffer_compression_error>;
	
	template <typename COMPRESSOR>
	buffer_compression_result<void> compressor_restart(COMPRESSOR& comp) = delete;
	template <typename COMPRESSOR, typename BUFFER>
	buffer_compression_result<void> compressor_compress_fragment(COMPRESSOR& comp, bytelike_range auto&& input_range, BUFFER& output_buffer) = delete;
	template <typename COMPRESSOR, typename BUFFER>
	buffer_compression_result<void> compressor_finalize(COMPRESSOR& comp, BUFFER& output_buffer) = delete;

	/*
	template <typename T>
	concept compressor = requires (T & comp, std::span<char const> in_range, std::string out_range) {
		{ compressor_restart(comp) } -> std::convertible_to<buffer_compression_result<void>>;
		{ compressor_compress_fragment(comp, in_range, out_range) } -> std::convertible_to<buffer_compression_result<void>>;
		{ compressor_finalize(comp, out_range) }	-> std::convertible_to<buffer_compression_result<void>>;
	};
	*/
	
	template <typename BUFFER, bytelike_range RANGE, typename COMPRESSOR>
	buffer_compression_result<void> buffer_append_compressed(BUFFER& buffer, RANGE&& input_range, COMPRESSOR& comp)
	{
		//static_assert(compressor<COMPRESSOR>, "comp must be a compressor");
		return compressor_restart(comp)
			.and_then([&] { return compressor_compress_fragment(comp, input_range, buffer); })
			.and_then([&] { return compressor_finalize(comp, buffer); });
	}
}