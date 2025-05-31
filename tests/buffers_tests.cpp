/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/buffers.h"
#include "../include/ghassanpl/expected.h"
#include "../include/ghassanpl/json_helpers.h"
#include "../include/ghassanpl/mmap.h"

#include <gtest/gtest.h>

using namespace ghassanpl;


GTEST_API_ void testing::internal::PrintU8StringTo(const ::std::u8string& s, ::std::ostream* os)
{
	(*os) << std::string_view{ (const char*)s.data(), s.size() };
}

TEST(buffers, basics)
{
	const char arr[] = "yo ";

	std::u8string dest;

	EXPECT_EQ(buffer_append_range(dest, arr), 4);
	EXPECT_EQ(buffer_append_range(dest, std::string_view{ arr }), 3);
	const char8_t expected[] = u8"yo \0yo ";
	EXPECT_EQ(dest, (std::u8string_view{ expected, sizeof(expected) - 1 }));

	dest = {};
	buffer_append_utf8(dest, U'Z'); buffer_append_utf8(dest, U'a'); buffer_append_utf8(dest, U'ż'); buffer_append_utf8(dest, U'ó'); buffer_append_utf8(dest, U'ł'); buffer_append_utf8(dest, U'ć');
	buffer_append_utf8(dest, std::u32string_view{ U" gęślą" });
	EXPECT_EQ(dest, u8"Zażółć gęślą");

	dest = {};
	uint32_t bleh = 'damn';
	buffer_append_range(dest, as_chars(bleh));
	EXPECT_EQ(dest, u8"nmad");
	buffer_append_pod(dest, bleh);
	EXPECT_EQ(dest, u8"nmadnmad");
}

#if 0

/// https://hugi.scene.org/online/coding/hugi%2012%20-%20colzp.htm
/// https://github.com/lmcilroy/lzp/blob/master/lzp.c
/// https://cs.stackexchange.com/questions/134277/what-is-the-simplest-algorithm-to-compress-a-string/152057#152057?newreg=5c5487df3eb541609f29964a48017bd1

struct lzp_compressor
{
	struct compression_options
	{
		size_t max_internal_output_buffer_size = 1024;
	};

	lzp_compressor() { init({}); }
	lzp_compressor(compression_options opts) { init(opts); }

	expected<void, buffer_compression_error> init(compression_options opts)
	{
		opts.max_internal_output_buffer_size = std::max<size_t>(opts.max_internal_output_buffer_size, 64);
		if (opts.max_internal_output_buffer_size > 0x100000)
			return unexpected(buffer_compression_error{ buffer_compression_error_type::invalid_options, "max internal output buffer size is too large (must be <= 1MB)"});

		m_options = opts;
		m_table.resize(65536U);
		m_output_buffer.reserve(m_options.max_internal_output_buffer_size);
		
		restart();
		return {};
	}

	expected<void, buffer_compression_error> restart()
	{
		std::fill(m_table.begin(), m_table.end(), 0);
		m_bytes_in_encode_buffer = 0;
		m_encode_buffer_mask = 0;
		m_current_hash = 0;
		m_input_bytes_swallowed = 0;
		m_output_buffer.clear();
		return {};
	}

	template <typename BUFFER>
	expected<void, buffer_compression_error> compress_fragment(bytelike_range auto&& _input, BUFFER& output)
	{
		auto input = std::span{ _input };

		const auto encode_buffer = m_encode_buffer.data();
		const auto table = m_table.data();
		const auto max_internal_output_buffer_size = m_options.max_internal_output_buffer_size;
		
		auto& output_buffer = m_output_buffer;
		
		auto bytes_in_encode_buffer = m_bytes_in_encode_buffer;
		auto encode_buffer_mask = m_encode_buffer_mask;
		auto current_hash = m_current_hash;
		auto input_bytes_swallowed = m_input_bytes_swallowed;
		
		while (!input.empty())
		{
			const int max_input_to_swallow = (int)std::min(size_t(8), input.size());
			auto input_data = input.data();
			for (; input_bytes_swallowed < max_input_to_swallow; input_bytes_swallowed++)
			{
				const auto c = std::bit_cast<uint8_t>(*input_data++); /// TODO: also add to checksum
				if (c == table[current_hash])
				{
					encode_buffer_mask |= 1 << input_bytes_swallowed;
				}
				else
				{
					table[current_hash] = c;
					encode_buffer[bytes_in_encode_buffer++] = c;
				}

				current_hash = (current_hash << 4) ^ std::bit_cast<uint8_t>(c);
			}
			input = input.subspan(input_bytes_swallowed);

			if (input_bytes_swallowed == 8)
			{
				if (output_buffer.size() + bytes_in_encode_buffer + 1 > max_internal_output_buffer_size)
				{
					buffer_append_range(output, output_buffer);
					output_buffer.clear();
				}

				output_buffer.push_back(encode_buffer_mask);
				output_buffer.insert(output_buffer.end(), encode_buffer, encode_buffer + bytes_in_encode_buffer);
				bytes_in_encode_buffer = 0;
				encode_buffer_mask = 0;
				input_bytes_swallowed = 0;
			}
		}
		
		m_bytes_in_encode_buffer = bytes_in_encode_buffer;
		m_encode_buffer_mask = encode_buffer_mask;
		m_current_hash = current_hash;
		m_input_bytes_swallowed = input_bytes_swallowed;
		
		return {};
	}

	template <typename BUFFER>
	expected<void, buffer_compression_error> finalize(BUFFER& output)
	{
		if (m_output_buffer.size() > 0)
		{
			buffer_append_range(output, m_output_buffer);
			m_output_buffer.clear();
		}
		if (m_input_bytes_swallowed > 0)
		{
			m_output_buffer.push_back(m_encode_buffer_mask);
			m_output_buffer.insert(m_output_buffer.end(), m_encode_buffer.begin(), m_encode_buffer.begin() + m_bytes_in_encode_buffer);
			/// TODO: pad to 8 bytes?
			buffer_append_range(output, m_output_buffer);
		}
		return {};
	}

protected:

	compression_options m_options{};

	std::array<uint8_t, 8> m_encode_buffer;
	std::vector<uint8_t> m_table;
	int m_bytes_in_encode_buffer = 0;
	uint8_t m_encode_buffer_mask = 0;
	uint16_t m_current_hash = 0;
	int m_input_bytes_swallowed = 0;

	std::vector<uint8_t> m_output_buffer;

};

auto compressor_init(lzp_compressor& compressor, typename lzp_compressor::compression_options opts)
{
	return compressor.init(opts);
}
auto compressor_restart(lzp_compressor& compressor)
{
	return compressor.restart();
}
template <typename BUFFER>
auto compressor_compress_fragment(lzp_compressor& compressor, bytelike_range auto&& input_range, BUFFER& buffer)
{
	return compressor.compress_fragment(input_range, buffer);
}
template <typename BUFFER>
auto compressor_finalize(lzp_compressor& compressor, BUFFER& buffer)
{
	return compressor.finalize(buffer);
}

struct lzp_decompressor
{

	template <typename BUFFER>
	void decode(bytelike_range auto&& _input, BUFFER& output) {

		auto input = std::span{ _input };
		for (;;) {
			m_input_bytes_swallowed = 0;
			if (input.size() < 9) break;

			auto data = input.data();
			const auto data_end = input.data() + input.size();

			m_bytes_in_decode_buffer = 0;
			m_decode_buffer_mask = *data++;
			m_input_bytes_swallowed++;
			for (int i = 0; i < 8; i++)
			{
				uint8_t c;
				if ((m_decode_buffer_mask & (1 << i)) != 0)
				{
					c = m_table[m_current_hash];
				}
				else
				{
					if (data == data_end) break;
					c = *data++;
					m_input_bytes_swallowed++;
					m_table[m_current_hash] = c;
				}
				m_decode_buffer[m_bytes_in_decode_buffer++] = c;
				m_current_hash = (m_current_hash << 4) ^ std::bit_cast<uint8_t>(c);
			}
			input = input.subspan(m_input_bytes_swallowed);

			if (m_input_bytes_swallowed > 0)
			{
				m_output_buffer.insert(m_output_buffer.end(), m_decode_buffer.begin(), m_decode_buffer.begin() + m_bytes_in_decode_buffer);
			}
		}

		buffer_append_range(output, m_output_buffer);
	}

	struct decompression_options
	{
		size_t max_internal_output_buffer_size = 1024;
	};

	lzp_decompressor() { init({}); }
	lzp_decompressor(decompression_options opts) { init(opts); }

	expected<void, buffer_compression_error> init(decompression_options opts)
	{
		opts.max_internal_output_buffer_size = std::max<size_t>(opts.max_internal_output_buffer_size, 64);
		if (opts.max_internal_output_buffer_size > 0x100000)
			return unexpected(buffer_compression_error{ buffer_compression_error_type::invalid_options, "max internal output buffer size is too large (must be <= 1MB)" });

		m_options = opts;
		m_table.resize(65536U);
		m_output_buffer.reserve(m_options.max_internal_output_buffer_size);

		restart();
		return {};
	}

	expected<void, buffer_compression_error> restart()
	{
		std::fill(m_table.begin(), m_table.end(), 0);
		m_bytes_in_decode_buffer = 0;
		m_decode_buffer_mask = 0;
		m_current_hash = 0;
		m_input_bytes_swallowed = 0;
		m_output_buffer.clear();
		return {};
	}

	decompression_options m_options{};
	std::vector<uint8_t> m_table;
	std::array<uint8_t, 8> m_decode_buffer;
	int m_bytes_in_decode_buffer = 0;
	uint8_t m_decode_buffer_mask = 0;
	uint16_t m_current_hash = 0;
	int m_input_bytes_swallowed = 0;

	std::vector<uint8_t> m_output_buffer;
};

TEST(buffer_compression, works)
{
	auto input_data = make_mmap_source<char>("gmock.dll");
	std::vector<char8_t> compressed_data;
	lzp_compressor compressor{};
	buffer_append_compressed(compressed_data, input_data, compressor);
	EXPECT_EQ(compressed_data.size(), 18);

	lzp_decompressor decompressor{};
	std::vector<char8_t> decompressed_data;
	decompressor.decode(compressed_data, decompressed_data);
	EXPECT_EQ(decompressed_data.size(), input_data.size());
}

#endif