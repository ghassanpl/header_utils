#pragma once

#include <type_traits>
#include <iterator>
#if __cplusplus >= 202002L
#include <concepts>
#endif

namespace ghassanpl
{
	/// Combines an existing hash with a new hash value
	constexpr void fold_in_hash64(uint64_t& hash1, uint64_t hash2) noexcept
	{
		constexpr uint64_t kMul = 0x9ddfea08eb382d69ULL;
		const auto seed = hash1;
		uint64_t a = (hash2 ^ seed) * kMul;
		a ^= (a >> 47);
		uint64_t b = (seed ^ a) * kMul;
		b ^= (b >> 47);
		b *= kMul;
		hash1 = b;
	}

	/// Combines an existing hash value (`seed`) with the hash of value `v`
	template <typename T, typename HASHER = std::hash<std::remove_cv_t<std::remove_reference_t<T>>>>
	constexpr void hash64_combine_to(uint64_t& seed, T const& v, HASHER const& hasher = HASHER{})
	{
		auto result = hasher(v);
		static_assert(std::is_same_v<decltype(result), uint64_t>, "hasher() must return a uint64_t");
		fold_in_hash64(seed, result);
	}

	/// Performs a hash of multiple values
	template <template<typename> typename HASHER = std::hash, typename FIRST, typename... T>
	[[nodiscard]] constexpr uint64_t hash64(FIRST const& first, T&&... values)
	{
		using first_hasher_type = HASHER<std::remove_cv_t<std::remove_reference_t<FIRST>>>;
		first_hasher_type hasher = {};

		static_assert(
			std::is_same_v<decltype(hasher(first)), uint64_t> &&
			(std::is_same_v<decltype(HASHER<std::remove_cv_t<std::remove_reference_t<T>>>()(values)), uint64_t> && ... && true),
			"hasher() must return a uint64_t for each type");
		
		uint64_t result = hasher(first);
		(ghassanpl::hash64_combine_to(result, std::forward<T>(values), HASHER<std::remove_cv_t<std::remove_reference_t<T>>>()), ...);
		return result;
	}

	/// Combines an existing hash value (`seed`) with the hash of a range of values
	template<typename It, typename HASHER = std::hash<typename std::iterator_traits<It>::value_type>>
	constexpr void fold_in_hash64_range(uint64_t& seed, It first, It last, HASHER const& hasher = {})
	{
		static_assert(std::is_same_v<decltype(hasher(*first)), uint64_t>, "hasher() must return a uint64_t");
		for (; first != last; ++first)
			hash64_combine_to(seed, *first, hasher);
	}

	/// Hashes a range of values
	template<typename It, typename HASHER = std::hash<typename std::iterator_traits<It>::value_type>>
	[[nodiscard]] constexpr uint64_t hash64_range(It first, It last, HASHER&& hasher = {})
	{
		static_assert(std::is_same_v<decltype(hasher(*first)), uint64_t>, "hasher() must return a uint64_t");
		uint64_t seed = 0;
		fold_in_hash64_range(seed, first, last, std::forward<HASHER>(hasher));
		return seed;
	}

#if __cplusplus >= 202002L
	/// Hashes a range of values
	template <std::ranges::range T, typename HASHER = std::hash<std::ranges::range_value_t<T>>>
	[[nodiscard]] constexpr uint64_t hash64_range(T range, HASHER&& hasher = {})
	{
		static_assert(std::is_same_v<decltype(hasher(std::declval<std::ranges::range_value_t<T>>())), uint64_t>, "hasher() must return a uint64_t");
		uint64_t seed = 0;
		fold_in_hash64_range(seed, std::ranges::begin(range), std::ranges::end(range), std::forward<HASHER>(hasher));
		return seed;
	}
#endif
}
