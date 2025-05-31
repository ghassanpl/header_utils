/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp20.h"
#include <random>
#include <numeric>
#include "span.h"

namespace ghassanpl::random
{
	/// TODO: Tests
	/// check out https://github.com/effolkronium/random/
	
	/// Shamelessly stolen from 'Numeric Recipes: The Art of Scientific Computing' Third Edition, by  William H. Press et. al, Cambridge University Press, 2007 (ISBN  978-0521880688),
	struct good_random_engine
	{
		uint64_t u, v, w;
		good_random_engine(uint64_t j) noexcept {
			seed(j);
		}

		void seed(uint64_t j) noexcept
		{
			v = 4101842887655102017LL;
			w = 1;
			u = j ^ v; (void)this->operator()();
			v = u; (void)this->operator()();
			w = v; (void)this->operator()();
		}

		[[nodiscard]] static constexpr uint64_t max() noexcept { return UINT64_MAX; }
		[[nodiscard]] static constexpr uint64_t min() noexcept { return 0; }

		[[nodiscard]] uint64_t operator()() noexcept
		{
			u = u * 2862933555777941757LL + 7046029254386353087LL;
			v ^= v >> 17; v ^= v << 31; v ^= v >> 8;
			w = 4294957665U * (w & 0xffffffff) + (w >> 32);
			uint64_t x = u ^ (u << 21); x ^= x >> 35; x ^= x << 4;
			return (x + v) ^ w;
		}
	};

#if GHPL_CPP20
	static_assert(std::uniform_random_bit_generator<good_random_engine>);
#endif
	
	thread_local inline std::default_random_engine default_random_engine;
	
	template <typename INTEGER = uint64_t, typename RANDOM = std::default_random_engine>
	[[nodiscard]] constexpr INTEGER integer(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		std::uniform_int_distribution<INTEGER> dist;
		return dist(rng);
	}

	template <typename REAL = double, typename RANDOM = std::default_random_engine>
	[[nodiscard]] REAL percentage(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<REAL> dist;
		return dist(rng);
	}
	
	/*
	template <typename REAL = double, typename T, typename RANDOM = std::default_random_engine>
	[[nodiscard]] auto between(T const& a, T const& b, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::lerp;
		if constexpr (std::is_integral_v<T>)
			return lerp(a, b, ::ghassanpl::random::percentage<REAL>(rng));
		else
			return lerp(a, b, ::ghassanpl::random::percentage<REAL>(rng));
	}
	*/

	template <typename REAL = double, typename RANDOM = std::default_random_engine>
	[[nodiscard]] REAL normal(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::normal_distribution<REAL> dist;
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine>
	[[nodiscard]] uint64_t dice(uint64_t n_sided, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (n_sided < 2) return 0;
		std::uniform_int_distribution<uint64_t> dist{ 0, n_sided - 1 };
		return dist(rng) + 1;
	}

	template <typename RANDOM = std::default_random_engine>
	[[nodiscard]] uint64_t dice(uint64_t n_dice, uint64_t n_sided, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (n_sided < 2) return 0;
		std::uniform_int_distribution<uint64_t> dist{ 0, n_sided - 1 };
		uint64_t sum = 0;
		for (uint64_t i = 0; i < n_dice; ++i)
			sum += dist(rng) + 1;
		return sum;
	}
	
	template <uint64_t N_SIDED, typename RANDOM = std::default_random_engine>
	[[nodiscard]] uint64_t dice(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static_assert(N_SIDED >= 2, "you cannot roll a 0 or 1-sided die");
		static std::uniform_int_distribution<uint64_t> dist{ 0, N_SIDED - 1 };
		return dist(rng) + 1;
	}

	template <uint64_t N_DICE, uint64_t N_SIDED, typename RANDOM = std::default_random_engine>
	[[nodiscard]] uint64_t dice(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static_assert(N_DICE >= 1 , "you cannot roll less than one die");
		static_assert(N_SIDED >= 2, "you cannot roll a 0 or 1-sided die");
		static std::uniform_int_distribution<uint64_t> dist{ 0, N_SIDED - 1 };
		uint64_t sum = 0;
		for (uint64_t i = 0; i < N_DICE; ++i)
			sum += dist(rng) + 1;
		return sum;
	}

	template <typename RANDOM = std::default_random_engine>
	[[nodiscard]] bool coin(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		std::uniform_int_distribution<uint64_t> dist{ 0, 1 };
		return dist(rng) == 0;
	}
	 
	namespace operators
	{
		[[nodiscard]] inline int operator""_d2(unsigned long long int n) { return (int)dice(n, 2, default_random_engine); }
		[[nodiscard]] inline int operator""_d4(unsigned long long int n) { return (int)dice(n, 4, default_random_engine); }
		[[nodiscard]] inline int operator""_d6(unsigned long long int n) { return (int)dice(n, 6, default_random_engine); }
		[[nodiscard]] inline int operator""_d8(unsigned long long int n) { return (int)dice(n, 8, default_random_engine); }
		[[nodiscard]] inline int operator""_d10(unsigned long long int n) { return (int)dice(n, 10, default_random_engine); }
		[[nodiscard]] inline int operator""_d12(unsigned long long int n) { return (int)dice(n, 12, default_random_engine); }
		[[nodiscard]] inline int operator""_d20(unsigned long long int n) { return (int)dice(n, 20, default_random_engine); }
		[[nodiscard]] inline int operator""_d100(unsigned long long int n) { return (int)dice(n, 100, default_random_engine); }
	}

	template <typename RANDOM = std::default_random_engine, std::integral T>
	[[nodiscard]] T in_integer_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static_assert(is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>, "in_integer_range only works on real integer types");
		if (from >= to) return T{};
		std::uniform_int_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine, std::integral T>
	[[nodiscard]] T in_integer_range(T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return in_integer_range(T{}, to, rng);
	}

	template <typename RANDOM = std::default_random_engine, std::floating_point T>
	[[nodiscard]] T in_real_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static_assert(std::is_floating_point_v<T>, "in_real_range only works on floating point types");
		if (from >= to) return T{};
		std::uniform_real_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine, std::floating_point T>
	[[nodiscard]] T in_real_range(T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return in_real_range(T{}, to, rng);
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	[[nodiscard]] auto in_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if constexpr (std::is_enum_v<T>)
		{
			if (from >= to) return T{};

			std::uniform_int_distribution<std::underlying_type_t<T>> dist(from, to);
			return (T)dist(rng);
		}
		else if constexpr (std::is_floating_point_v<T>)
		{
			if (from >= to) return T{};
			return in_real_range(from, to, rng);
		}
		else if constexpr (is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>)
		{
			if (from >= to) return T{};
			return in_integer_range(from, to, rng);
		}
		else if constexpr (
			is_any_of_v<T, unsigned char, signed char, char, wchar_t, char16_t, char32_t>
#ifdef __cpp_char8_t
			|| std::is_same_v<T, char8_t>
#endif
			)
		{
			if (from >= to) return T{};
			/// Unfortunately `uniform_int_distribution` and, as a result, `integer_range` is defined only on the non-char integral types,
			/// which means we have to cast
			using signed_as_T = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>; /// 64bits because char32_t can TECHNICALLY be 64-bit
			return static_cast<T>(in_integer_range(static_cast<signed_as_T>(from), static_cast<signed_as_T>(to), rng));
		}
		else if constexpr (requires{ lerp(from, to, ::ghassanpl::random::percentage<float>(rng)); })
		{
			return lerp(from, to, ::ghassanpl::random::percentage<float>(rng));
		}
		else if constexpr (requires{ lerp(from, to, ::ghassanpl::random::percentage<double>(rng)); })
		{
			return lerp(from, to, ::ghassanpl::random::percentage<double>(rng));
		}
		else if constexpr (requires{ mix(from, to, ::ghassanpl::random::percentage<float>(rng)); })
		{
			return mix(from, to, ::ghassanpl::random::percentage<float>(rng));
		}
		else if constexpr (requires{ mix(from, to, ::ghassanpl::random::percentage<double>(rng)); })
		{
			return mix(from, to, ::ghassanpl::random::percentage<double>(rng));
		}
	}

	/// TODO: Should we move the below to random_seq?
	
	template <typename T = float>
	[[nodiscard]] constexpr T halton_sequence(size_t index, size_t base = 2)
	{
		static_assert(std::is_floating_point_v<T>, "halton_sequence only works on floating point types");

		auto result = T(0);
		auto fraction = T(1);
		while (index > 0)
		{
			fraction /= base;
			result += fraction * (index % base);
			index /= base;
		}
		return result;
	}

	template <typename RANDOM = std::default_random_engine>
	[[nodiscard]] bool with_probability(double probability, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return percentage(rng) < std::clamp(probability, 0.0, 1.0);
	}

	template <typename RANDOM = std::default_random_engine>
	[[nodiscard]] bool with_probability(double probability, double& result, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		auto res = result = percentage(rng);
		return res < std::clamp(probability, 0.0, 1.0);
	}

	template <typename RANDOM = std::default_random_engine>
	[[nodiscard]] bool one_in(size_t n, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (n == 0) return false;
		return with_probability(1.0 / double(n), rng);
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	void shuffle(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::begin;
		using std::end;
		std::shuffle(begin(cont), end(cont), rng);
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	GHPL_REQUIRES(std::ranges::sized_range<T>)
		[[nodiscard]] auto iterator(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::size;
		using std::begin;
		return begin(cont) + in_integer_range(0LL, (int64_t)size(cont) - 1, rng);
	}

	template <typename RANDOM = std::default_random_engine, typename T, typename PRED>
	GHPL_REQUIRES(std::ranges::sized_range<T>)
	[[nodiscard]] auto iterator_if(T& cont, PRED&& pred, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::size;
		using std::begin;
		using std::end;
		auto begin_it = begin(cont);
		const auto end_it = end(cont);
		const auto valid_count = std::count_if(begin_it, end_it, pred);
		auto item_position = in_integer_range(0LL, (int64_t)valid_count - 1, rng);
		for (; begin_it != end_it; ++begin_it)
		{
			if (pred(*begin_it) && (item_position--) == 0)
			{
				return begin_it;
			}
		}
		return end_it;
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	[[nodiscard]] auto index(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return std::distance(begin(cont), iterator(cont, rng));
	}

	template <typename RANDOM = std::default_random_engine, typename T, typename PRED>
	[[nodiscard]] auto index_if(T& cont, PRED&& pred, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return std::distance(begin(cont), iterator_if(cont, std::forward<PRED>(pred), rng));
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	[[nodiscard]] auto* element(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::end;
		auto result = iterator(cont, rng);
		return (result != end(cont)) ? std::addressof(*result) : nullptr;
	}
	
	template <typename RANDOM = std::default_random_engine, typename T, typename PRED>
	[[nodiscard]] auto* element_if(T& cont, PRED&& predicate, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::end;
		auto result = iterator_if(cont, std::forward<PRED>(predicate), rng);
		return (result != end(cont)) ? std::addressof(*result) : nullptr;
	}

	template <typename... T>
	[[nodiscard]] auto one_of(T&&... values)
	{
		/// TODO: Forward except last, if last matches rng concept, use last as rng, else use default_random_engine
		static_assert(sizeof...(values) > 0, "at least one value must be provided");
		auto v = std::array{ std::forward<T>(values)... };
		return std::move(*element(v, ::ghassanpl::random::default_random_engine));
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	[[nodiscard]] auto one_of(std::initializer_list<T> values, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (values.size() == 0) throw std::invalid_argument("values");
		return *element(values, rng);
	}

	/// TODO: random_range()
	/// 
	/// TODO: template <typename T> T enum_value(enum_flags<T> set);
	

	template <typename RANDOM = std::default_random_engine, typename T>
	[[nodiscard]] auto make_bag_randomizer(T& container, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using Iterator = decltype(std::end(container));
		struct Randomizer
		{
			Randomizer(RANDOM& rng, T& container)
				: mRNG(rng)
			{
				for (auto it = std::begin(container); it != std::end(container); ++it)
					mIterators.push_back(it);
				mCurrent = mIterators.end();
			}
			auto Next() { if (mCurrent == mIterators.end()) { Shuffle(); } return *mCurrent++; }
			void Shuffle() { std::shuffle(mIterators.begin(), mIterators.end(), mRNG); mCurrent = mIterators.begin(); }
		private:
			RANDOM& mRNG;
			std::vector<Iterator> mIterators;
			typename std::vector<Iterator>::iterator mCurrent;
		};

		return Randomizer{ rng, container };
	}

	/// When probability calculations are known ahead of time or expensive
	/// \complexity O(N) space, O(N+logN) time
	/// TODO: Check if works with known-sized spans
	template <typename T, typename RANDOM>
	[[nodiscard]] size_t option_with_probability(span<T const> option_probabilities, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static_assert(std::is_convertible_v<T, double>, "option probabilities must be convertible to double");
		/// Well, <random> is pretty dope it seems
		std::discrete_distribution<size_t> dist(option_probabilities.begin(), option_probabilities.end());
		return dist(rng);
	}

	/// For cheap probability functions
	/// \pre prob_func will never return < 0.0
	/// \complexity O(1) space, O(2N) time
	template <typename RANGE, typename FUNC, typename RANDOM>
	[[nodiscard]] auto iterator_with_probability(RANGE&& range, FUNC&& prob_func, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
#ifdef __cpp_lib_ranges
		static_assert(std::ranges::forward_range<RANGE>, "range must be forward range");
		static_assert(std::is_invocable_v<FUNC, std::ranges::range_reference_t<RANGE>>);
		static_assert(std::convertible_to<std::invoke_result_t<FUNC, std::ranges::range_reference_t<RANGE>>, double>);
#endif

		const auto start = std::begin(range);
		const auto end = std::end(range);

		const auto sum = std::accumulate(start, end, 0.0, [&](auto acc, auto&& item) { return acc + (double)prob_func(item); });
		auto target = in_real_range(0.0, sum, rng);

		for (auto it = start; it != end; ++it)
		{
			const auto weight = (double)prob_func(*it);
			if (target < weight)
				return it;
			target = target - weight;
		}

		return end;
	}

}
