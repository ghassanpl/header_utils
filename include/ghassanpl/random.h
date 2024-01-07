/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <random>
#include <concepts>
#include <numeric>
#include <span>

namespace ghassanpl::random
{
	/// TODO: Tests
	/// check out https://github.com/effolkronium/random/
	
	thread_local inline std::default_random_engine default_random_engine;
	
	template <typename INTEGER = uint64_t, typename RANDOM = std::default_random_engine>
	constexpr INTEGER integer(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<INTEGER> dist;
		return dist(rng);
	}

	template <typename REAL = double, typename RANDOM = std::default_random_engine>
	REAL percentage(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<REAL> dist;
		return dist(rng);
	}

	template <typename REAL = double, typename RANDOM = std::default_random_engine>
	REAL normal(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::normal_distribution<REAL> dist;
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine>
	uint64_t dice(uint64_t n_sided, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (n_sided < 2) return 0;
		std::uniform_int_distribution<uint64_t> dist{ 0, n_sided - 1 };
		return dist(rng) + 1;
	}

	template <typename RANDOM = std::default_random_engine>
	uint64_t dice(uint64_t n_dice, uint64_t n_sided, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (n_sided < 2) return 0;
		std::uniform_int_distribution<uint64_t> dist{ 0, n_sided - 1 };
		uint64_t sum = 0;
		for (uint64_t i = 0; i < n_dice; ++i)
			sum += dist(rng) + 1;
		return sum;
	}
	
	template <uint64_t N_SIDED, typename RANDOM = std::default_random_engine>
	requires (N_SIDED >= 2)
	uint64_t dice(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<uint64_t> dist{ 0, N_SIDED - 1 };
		return dist(rng) + 1;
	}

	template <uint64_t N_DICE, uint64_t N_SIDED, typename RANDOM = std::default_random_engine>
	requires (N_DICE >= 1 && N_SIDED >= 2)
	uint64_t dice(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<uint64_t> dist{ 0, N_SIDED - 1 };
		uint64_t sum = 0;
		for (uint64_t i = 0; i < N_DICE; ++i)
			sum += dist(rng) + 1;
		return sum;
	}

	template <typename RANDOM = std::default_random_engine>
	bool coin(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		std::uniform_int_distribution<uint64_t> dist{ 0, 1 };
		return dist(rng) == 0;
	}
	 
	namespace operators
	{
		inline int operator""_d2(unsigned long long int n) { return (int)dice(n, 2, default_random_engine); }
		inline int operator""_d4(unsigned long long int n) { return (int)dice(n, 4, default_random_engine); }
		inline int operator""_d6(unsigned long long int n) { return (int)dice(n, 6, default_random_engine); }
		inline int operator""_d8(unsigned long long int n) { return (int)dice(n, 8, default_random_engine); }
		inline int operator""_d10(unsigned long long int n) { return (int)dice(n, 10, default_random_engine); }
		inline int operator""_d12(unsigned long long int n) { return (int)dice(n, 12, default_random_engine); }
		inline int operator""_d20(unsigned long long int n) { return (int)dice(n, 20, default_random_engine); }
		inline int operator""_d100(unsigned long long int n) { return (int)dice(n, 100, default_random_engine); }
	}

	template <class T, class... TYPES>
	constexpr inline bool is_any_of_v = std::disjunction_v<std::is_same<T, TYPES>...>;

	template <typename RANDOM = std::default_random_engine, typename T>
	requires is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>
	T in_integer_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (from >= to) return T{};
		std::uniform_int_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine, std::floating_point T>
	T in_real_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (from >= to) return T{};
		std::uniform_real_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	auto in_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (from >= to) return T{};

		if constexpr (std::is_enum_v<T>)
		{
			std::uniform_int_distribution<std::underlying_type_t<T>> dist(from, to);
			return (T)dist(rng);
		}
		else if constexpr (std::is_floating_point_v<T>)
			return in_real_range(from, to, rng);
		else if constexpr (is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>)
			return in_integer_range(from, to, rng);
		else if constexpr (is_any_of_v<T, unsigned char, signed char, char, wchar_t, char8_t, char16_t, char32_t>)
		{
			/// Unfortunately `uniform_int_distribution` and, as a result, `integer_range` is defined only on the non-char integral types,
			/// which means we have to cast
			using signed_as_T = std::conditional_t<std::is_signed_v<T>, int64_t, uint64_t>; /// 64bits because char32_t can TECHNICALLY be 64-bit
			return static_cast<T>(in_integer_range(static_cast<signed_as_T>(from), static_cast<signed_as_T>(to), rng));
		}
		else
		{
			const auto i = to - from;
			const auto r = ::ghassanpl::random::in_range(decltype(i){}, i);
			return from + r;
		}
	}

	/// TODO: Should we move the below to random_seq?
	
	template <std::floating_point T = float>
	constexpr T halton_sequence(size_t index, size_t base = 2)
	{
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
	bool with_probability(double probability, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return percentage(rng) < std::clamp(probability, 0.0, 1.0);
	}

	template <typename RANDOM = std::default_random_engine>
	bool with_probability(double probability, double& result, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		auto res = result = percentage(rng);
		return res < std::clamp(probability, 0.0, 1.0);
	}

	template <typename RANDOM = std::default_random_engine>
	bool one_in(size_t n, RANDOM& rng = ::ghassanpl::random::default_random_engine)
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
	requires std::ranges::sized_range<T>
	auto iterator(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::size;
		using std::begin;
		return begin(cont) + in_integer_range(0LL, (int64_t)size(cont) - 1, rng);
	}

	template <typename RANDOM = std::default_random_engine, typename T, typename PRED>
	requires std::ranges::sized_range<T>
	auto iterator(T& cont, PRED&& pred, RANDOM& rng = ::ghassanpl::random::default_random_engine)
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
	auto index(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return std::distance(begin(cont), iterator(cont, rng));
	}

	template <typename RANDOM = std::default_random_engine, typename T, typename PRED>
	auto index(T& cont, PRED&& pred, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return std::distance(begin(cont), iterator(cont, std::forward<PRED>(pred), rng));
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	auto* element(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::end;
		auto result = iterator(cont, rng);
		return (result != end(cont)) ? std::to_address(result) : nullptr;
	}
	
	template <typename RANDOM = std::default_random_engine, typename T, typename PRED>
	auto* element(T& cont, PRED&& predicate, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::end;
		auto result = iterator(cont, std::forward<PRED>(predicate), rng);
		return (result != end(cont)) ? std::to_address(result) : nullptr;
	}

	template <typename... T>
	auto one_of(T&&... values)
	{
		/// TODO: Forward except last, if last matches rng concept, use last as rng, else use default_random_engine
		static_assert(sizeof...(values) > 0, "at least one value must be provided");
		auto v = std::array{ std::forward<T>(values)... };
		return std::move(*element(v, ::ghassanpl::random::default_random_engine));
	}

	template <typename RANDOM = std::default_random_engine, typename T>
	auto one_of(std::initializer_list<T> values, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (values.size() == 0) throw std::invalid_argument("values");
		return *element(values, rng);
	}

	/// TODO: random_range()
	/// 
	/// TODO: template <typename T> T enum_value(enum_flags<T> set);
	

	template <typename RANDOM = std::default_random_engine, typename T>
	auto make_bag_randomizer(T& container, RANDOM& rng = ::ghassanpl::random::default_random_engine)
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
			void Shuffle() { std::ranges::shuffle(mIterators, mRNG); mCurrent = mIterators.begin(); }
		private:
			RANDOM& mRNG;
			std::vector<Iterator> mIterators;
			typename std::vector<Iterator>::iterator mCurrent;
		};

		return Randomizer{ rng, container };
	}

	template <std::convertible_to<double> T, typename RANDOM>
	size_t option_with_probability(std::span<T const> option_probabilities, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		/// Well, <random> is pretty dope it seems
		std::discrete_distribution<size_t> dist(option_probabilities.begin(), option_probabilities.end());
		return dist(rng);
	}

}
