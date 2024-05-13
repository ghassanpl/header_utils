/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "random.h"
//#include "hashes.h"
#include "noise.h"

namespace ghassanpl::noise
{
	//using ghassanpl::integer::splitmix64;

	/// Value Noise

	[[nodiscard]] constexpr uint64_t SquirrelNoise3(uint64_t seed, uint64_t position)
	{
		const uint64_t BIT_NOISE1 = 0xB5297A4DB5297A4D;
		const uint64_t BIT_NOISE2 = 0x68E31DA468E31DA4;
		const uint64_t BIT_NOISE3 = 0x1B56C4E91B56C4E9;

		uint64_t mangled = position;
		mangled *= BIT_NOISE1;
		mangled += seed;
		mangled ^= (mangled >> 8);
		mangled += BIT_NOISE2;
		mangled ^= (mangled << 8);
		mangled *= BIT_NOISE3;
		mangled ^= (mangled >> 8);
		return mangled;
	}

	[[nodiscard]] constexpr uint32_t SquirrelNoise5(uint32_t seed, int32_t position)
	{
		constexpr uint32_t SQ5_BIT_NOISE1 = 0xd2a80a3f;	// 11010010101010000000101000111111
		constexpr uint32_t SQ5_BIT_NOISE2 = 0xa884f197;	// 10101000100001001111000110010111
		constexpr uint32_t SQ5_BIT_NOISE3 = 0x6C736F4B; // 01101100011100110110111101001011
		constexpr uint32_t SQ5_BIT_NOISE4 = 0xB79F3ABB;	// 10110111100111110011101010111011
		constexpr uint32_t SQ5_BIT_NOISE5 = 0x1b56c4f5;	// 00011011010101101100010011110101

		auto mangledBits = (uint32_t)position;
		mangledBits *= SQ5_BIT_NOISE1;
		mangledBits += seed;
		mangledBits ^= (mangledBits >> 9);
		mangledBits += SQ5_BIT_NOISE2;
		mangledBits ^= (mangledBits >> 11);
		mangledBits *= SQ5_BIT_NOISE3;
		mangledBits ^= (mangledBits >> 13);
		mangledBits += SQ5_BIT_NOISE4;
		mangledBits ^= (mangledBits >> 15);
		mangledBits *= SQ5_BIT_NOISE5;
		mangledBits ^= (mangledBits >> 17);
		return mangledBits;
	}

	[[nodiscard]] constexpr uint32_t Get2dNoiseUint(int32_t indexX, int32_t indexY, uint32_t seed)
	{
		constexpr int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
		return SquirrelNoise5(indexX + (PRIME_NUMBER * indexY), seed);
	}

	[[nodiscard]] constexpr double Get1dNoiseZeroToOne(int32_t index, uint32_t seed)
	{
		constexpr double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
		return ONE_OVER_MAX_UINT * (double)SquirrelNoise5(index, seed);
	}

	[[nodiscard]] constexpr double Get2dNoiseZeroToOne(int32_t indexX, int32_t indexY, uint32_t seed)
	{
		constexpr double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
		return ONE_OVER_MAX_UINT * (double)Get2dNoiseUint(indexX, indexY, seed);
	}
}

namespace ghassanpl::random
{

	inline uint64_t xorshift64(uint64_t& state) noexcept
	{
		uint64_t x = state;
		x ^= x >> 12;
		x ^= x << 25;
		x ^= x >> 27;
		state = x;
		return x * 0x2545F4914F6CDD1D;
	}

	inline uint64_t numrep_hash(uint64_t index)
	{
		uint64_t v = index * 3935559000370003845LL + 2691343689449507681LL;
		v ^= v >> 21; v ^= v << 37; v ^= v >> 4;
		v *= 4768777513237032717LL;
		v ^= v << 20; v ^= v >> 41; v ^= v << 5;
		return v;
	}

	/*
	inline std::pair<uint32_t, uint32_t> philox2x32_R(unsigned int R, std::pair<uint32_t, uint32_t> sequence_index, uint32_t sequence_key)
	{
		static constexpr auto _philox2x32round = [](std::pair<uint32_t, uint32_t> sequence_index, uint32_t sequence_key) -> std::pair<uint32_t, uint32_t> {
			uint64_t product = (((uint64_t)0xd256d193)) * sequence_index.first;
			uint32_t hi = (uint32_t)(product >> 32);
			uint32_t lo = (uint32_t)product;
			return { hi ^ sequence_key ^ sequence_index.second, lo };
		};
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		sequence_index = _philox2x32round(sequence_index, sequence_key);
		return sequence_index;
	}
	*/

	inline uint64_t philox64(uint64_t sequence_index, uint32_t sequence_key)
	{
		static constexpr auto _philox2x32round = [](std::pair<uint32_t, uint32_t> ctr, uint32_t key) -> std::pair<uint32_t, uint32_t> {
			uint64_t product = (((uint64_t)0xd256d193)) * ctr.first;
			uint32_t hi = (uint32_t)(product >> 32);
			uint32_t lo = (uint32_t)product;
			return { hi ^ key ^ ctr.second, lo };
		};
		auto ctrPair = std::pair<uint32_t, uint32_t>{ (uint32_t)sequence_index, (uint32_t)(sequence_index >> 32) };
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key); sequence_key += ((uint32_t)0x9E3779B9);
		ctrPair = _philox2x32round(ctrPair, sequence_key);
		return uint64_t(ctrPair.first) | (uint64_t(ctrPair.second) << 32ULL);
	}

	struct philox64_engine
	{
		using result_type = uint64_t;

		result_type operator()() noexcept { return philox64(m_index + n++, m_key); }
		philox64_engine(uint64_t index = 0, uint32_t key = 0) noexcept : m_index(index), m_key(key), n(0) { }

		static constexpr result_type min() { return 0; }
		static constexpr result_type max() { return ~((result_type)0); }

		uint64_t index() const noexcept { return m_index; }
		uint32_t key() const noexcept { return m_key; }
		void reset(uint64_t index, uint32_t key) noexcept
		{
			m_index = index;
			m_key = key;
			n = 0;
		}
	private:
		uint64_t m_index;
		uint32_t m_key;
		uint64_t n;
	};
}
