/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.
/// Based on xxhash64.h, Copyright (c) 2016 Stephan Brumme, under the zlib license. (https://create.stephan-brumme.com/xxhash/)
/// Based on xxh64.hpp, Copyright (c) 2015 Daniel Kirchner (https://github.com/ekpyron/xxhashct/)

#pragma once

#include <cstdint>
#include <span>
#include <array>
#include <bit>

namespace ghassanpl
{

	struct xxhash64_t
	{
		explicit constexpr xxhash64_t(uint64_t seed = 0) 
			: state{ seed + Prime1 + Prime2, seed + Prime2, seed, seed - Prime1, }
		{
		}

		[[nodiscard]] constexpr static uint64_t hash(std::span<uint8_t const> input, uint64_t seed = 0)
		{
			xxhash64_t hasher(seed);
			hasher.add_bytes(input);
			return hasher.hash();
		}

		template <typename T>
		requires 
			(sizeof(T) <= sizeof(uint64_t)) 
			&& ((sizeof(T) & (sizeof(T) - 1)) == 0) /// Power of 2
			&& std::is_trivially_copyable_v<T> 
			&& !std::is_pointer_v<T>
		constexpr void add(T val) noexcept
		{
			if consteval
			{
				if constexpr (std::floating_point<T>)
				{
					/// TODO: Why was this here...?
					//if (val == T{ 0 }) val = T{ 0 }; /// -0 == 0
				}
				const auto as_int = std::bit_cast<uintN_t<bit_count<T>>>(val);
				const auto as_array = to_u8_array(as_int);
				add_bytes(as_array);
			}
			else
			{
				add_bytes(as_bytelikes<uint8_t>(val));
			}
		}

		template <typename RANGE>
		requires std::ranges::range<RANGE>
		constexpr void add(RANGE const& range) noexcept
		{
			if constexpr (std::same_as<std::ranges::range_value_t<RANGE>, uint8_t>)
			{
				add_bytes(std::span<uint8_t const>{ std::ranges::data(range), std::ranges::size(range) });
			}
			else
			{
				for (auto const& val : range)
					add(val);
			}
		}

		template <typename TUPLE_TYPE, uint64_t... Is>
		constexpr void add_tuple_elements(TUPLE_TYPE const& t, std::index_sequence<Is...>) noexcept
		{
			(add(std::get<Is>(t)), ...);
		}

		template <typename... T>
		constexpr void add(std::tuple<T...> const& tupl) noexcept
		{
			add_tuple_elements(tupl, std::index_sequence_for<T...>{});
		}


		template <typename... T>
		[[nodiscard]] constexpr static uint64_t hash(T&&... values)
		{
			xxhash64_t hasher;
			(hasher.add(values), ...);
			return hasher.hash();
		}

		constexpr bool add_bytes(std::span<uint8_t const> input)
		{
			if (input.empty())
				return false;

			totalLength += input.size();
			
			const auto* data = input.data();
			auto length = input.size();

			if (bufferSize + length < MaxBufferSize)
			{
				// just add new data
				while (length-- > 0)
					buffer[bufferSize++] = *data++;
				return true;
			}

			const auto* stop = data + length;
			const auto* stopBlock = stop - MaxBufferSize;

			if (bufferSize > 0)
			{
				while (bufferSize < MaxBufferSize)
					buffer[bufferSize++] = *data++;

				process(buffer, state[0], state[1], state[2], state[3]);
			}

			uint64_t s0 = state[0], s1 = state[1], s2 = state[2], s3 = state[3];
			while (data <= stopBlock)
			{
				process(data, s0, s1, s2, s3);
				data += 32;
			}
			state[0] = s0; state[1] = s1; state[2] = s2; state[3] = s3;

			bufferSize = stop - data;
			for (uint64_t i = 0; i < bufferSize; i++)
				buffer[i] = data[i];

			return true;
		}

		/// get current hash
		[[nodiscard]] constexpr uint64_t hash() const
		{
			uint64_t result;
			if (totalLength >= MaxBufferSize)
			{
				result = rotateLeft(state[0], 1) + rotateLeft(state[1], 7) + rotateLeft(state[2], 12) + rotateLeft(state[3], 18);
				result = (result ^ processSingle(0, state[0])) * Prime1 + Prime4;
				result = (result ^ processSingle(0, state[1])) * Prime1 + Prime4;
				result = (result ^ processSingle(0, state[2])) * Prime1 + Prime4;
				result = (result ^ processSingle(0, state[3])) * Prime1 + Prime4;
			}
			else
			{
				result = state[2] + Prime5;
			}

			result += totalLength;

			const auto* data = buffer;
			const auto* stop = data + bufferSize;

			for (; data + 8 <= stop; data += 8)
				result = rotateLeft(result ^ processSingle(0, endian64(data)), 27) * Prime1 + Prime4;

			if (data + 4 <= stop)
			{
				result = rotateLeft(result ^ (endian32(data)) * Prime1, 23) * Prime2 + Prime3;
				data += 4;
			}

			while (data != stop)
				result = rotateLeft(result ^ (*data++) * Prime5, 11) * Prime1;

			result ^= result >> 33;
			result *= Prime2;
			result ^= result >> 29;
			result *= Prime3;
			result ^= result >> 32;
			return result;
		}

	private:

		static constexpr uint32_t endian32(const auto* v)
		{
			if constexpr (std::endian::native == std::endian::little)
			{
				return uint32_t(uint8_t(v[0])) | (uint32_t(uint8_t(v[1])) << 8)
					| (uint32_t(uint8_t(v[2])) << 16) | (uint32_t(uint8_t(v[3])) << 24);
			}
			else
			{
				return uint32_t(uint8_t(v[3])) | (uint32_t(uint8_t(v[2])) << 8)
					| (uint32_t(uint8_t(v[1])) << 16) | (uint32_t(uint8_t(v[0])) << 24);
			}
		}
		
		static constexpr uint64_t endian64(const auto* v)
		{
			if constexpr (std::endian::native == std::endian::little)
			{
				return uint64_t(uint8_t(v[0])) | (uint64_t(uint8_t(v[1])) << 8)
					| (uint64_t(uint8_t(v[2])) << 16) | (uint64_t(uint8_t(v[3])) << 24)
					| (uint64_t(uint8_t(v[4])) << 32) | (uint64_t(uint8_t(v[5])) << 40)
					| (uint64_t(uint8_t(v[6])) << 48) | (uint64_t(uint8_t(v[7])) << 56);
			}
			else
			{
				return uint64_t(uint8_t(v[7])) | (uint64_t(uint8_t(v[6])) << 8)
					| (uint64_t(uint8_t(v[5])) << 16) | (uint64_t(uint8_t(v[4])) << 24)
					| (uint64_t(uint8_t(v[3])) << 32) | (uint64_t(uint8_t(v[2])) << 40)
					| (uint64_t(uint8_t(v[1])) << 48) | (uint64_t(uint8_t(v[0])) << 56);
			}
		}

		/// magic constants :-)
		static constexpr uint64_t Prime1 = 11400714785074694791ULL;
		static constexpr uint64_t Prime2 = 14029467366897019727ULL;
		static constexpr uint64_t Prime3 = 1609587929392839161ULL;
		static constexpr uint64_t Prime4 = 9650029242287828579ULL;
		static constexpr uint64_t Prime5 = 2870177450012600261ULL;

		/// temporarily store up to 31 bytes between multiple add() calls
		static constexpr uint64_t MaxBufferSize = 31 + 1;

		uint64_t      state[4] {};
		unsigned char buffer[MaxBufferSize] {};
		uint64_t      bufferSize = 0;
		uint64_t      totalLength = 0;

		/// rotate bits, should compile to a single CPU instruction (ROL)
		constexpr static inline uint64_t rotateLeft(uint64_t x, unsigned char bits)
		{
			return (x << bits) | (x >> (64 - bits));
		}

		/// process a single 64 bit value
		constexpr static inline uint64_t processSingle(uint64_t previous, uint64_t input)
		{
			return rotateLeft(previous + input * Prime2, 31) * Prime1;
		}

		/// process a block of 4x4 bytes, this is the main part of the XXHash32 algorithm
		constexpr static inline void process(const void* data, uint64_t& state0, uint64_t& state1, uint64_t& state2, uint64_t& state3)
		{
			auto const block = (const uint64_t*)data;
			state0 = processSingle(state0, block[0]);
			state1 = processSingle(state1, block[1]);
			state2 = processSingle(state2, block[2]);
			state3 = processSingle(state3, block[3]);
		}
	};

	[[nodiscard]] constexpr static uint64_t xxhash64(std::span<uint8_t const> input, uint64_t seed = 0)
	{
		xxhash64_t hasher(seed);
		hasher.add_bytes(input);
		return hasher.hash();
	}

}
