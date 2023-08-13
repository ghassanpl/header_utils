#pragma once

#include "random.h"
#include "hashes.h"
#include "noise.h"

namespace ghassanpl::noise
{
	using ghassanpl::integer::splitmix;

	/// Value Noise

	uint64_t SquirrelNoise3(uint64_t seed, uint64_t position)
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

	constexpr uint32_t Get2dNoiseUint(int32_t indexX, int32_t indexY, uint32_t seed)
	{
		constexpr int PRIME_NUMBER = 198491317; // Large prime number with non-boring bits
		return SquirrelNoise5(indexX + (PRIME_NUMBER * indexY), seed);
	}

	constexpr double Get1dNoiseZeroToOne(int32_t index, uint32_t seed)
	{
		constexpr double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
		return ONE_OVER_MAX_UINT * (double)SquirrelNoise5(index, seed);
	}

	constexpr double Get2dNoiseZeroToOne(int32_t indexX, int32_t indexY, uint32_t seed)
	{
		constexpr double ONE_OVER_MAX_UINT = (1.0 / (double)0xFFFFFFFF);
		return ONE_OVER_MAX_UINT * (double)Get2dNoiseUint(indexX, indexY, seed);
	}
}
