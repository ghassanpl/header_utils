#pragma once

#include <random>
#include <concepts>
#include <numeric>

namespace ghassanpl::random
{
	/// TODO: Tests

	namespace
	{
		inline static std::default_random_engine default_random_engine;
	}

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

	template <typename RANDOM, typename T>
	requires is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>
	T integer_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (from >= to) return T{};
		std::uniform_int_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM, std::floating_point T>
	T real_range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (from >= to) return T{};
		std::uniform_real_distribution<T> dist{ from, to };
		return dist(rng);
	}

	template <typename RANDOM, typename T>
	T range(T from, T to, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (from >= to) return T{};

		if constexpr (std::is_enum_v<T>)
		{
			std::uniform_int_distribution<std::underlying_type_t<T>> dist(from, to);
			return (T)dist(rng);
		}
		else if constexpr (std::is_floating_point_v<T>)
			return real_range(from, to, rng);
		else if constexpr (is_any_of_v<T, short, int, long, long long, unsigned short, unsigned int, unsigned long, unsigned long long>)
			return integer_range(from, to, rng);
	}

#if 0
	template <typename RANDOM = std::default_random_engine>
	radians_t radians(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<radians_t::base_type> dist{ radians_t::base_type{}, glm::two_pi<radians_t::base_type>() };
		return radians_t{ dist(rng) };
	}

	template <std::floating_point T, typename RANDOM = std::default_random_engine>
	T radians(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<T> dist{ T{}, glm::two_pi<T>() };
		return dist(rng);
	}

	template <typename RANDOM = std::default_random_engine>
	degrees_t degrees(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<degrees_t::base_type> dist{ degrees_t::base_type{0}, degrees_t::base_type{360} };
		return degrees_t{ dist(rng) };
	}

	template <std::floating_point T, typename RANDOM = std::default_random_engine>
	T degrees(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<T> dist{ T{}, T{360} };
		return dist(rng);
	}

	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> unit_vector(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return glm::rotate(glm::tvec2<T>{ T{ 1 }, T{ 0 } }, radians<T>(rng));
	}

	template <typename RANDOM = std::default_random_engine>
	ivec2 neighbor(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM = std::default_random_engine>
	ivec2 diagonal_neighbor(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 1.0f };
		case 1: return { -1.0f, 1.0f };
		case 2: return { -1.0f, -1.0f };
		case 3: return { 1.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM = std::default_random_engine>
	ivec2 surrounding(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 7 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		case 4: return { 1.0f, 1.0f };
		case 5: return { -1.0f, 1.0f };
		case 6: return { -1.0f, -1.0f };
		case 7: return { 1.0f, -1.0f };
		}
		return {};
	}
#endif
	
	template <typename RANDOM = std::default_random_engine>
	bool with_probability(double probability, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return percentage(rng) < std::clamp(probability, 0.0, 1.0);
	}

	template <typename RANDOM = std::default_random_engine>
	bool one_in(size_t n, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (n == 0) return false;
		return with_probability(1.0 / double(n), rng);
	}

	template <typename RANDOM, typename T>
	void shuffle(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::begin;
		using std::end;
		std::shuffle(begin(cont), end(cont), rng);
	}

	template <typename RANDOM, typename T>
	auto iterator(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::size;
		using std::begin;
		return begin(cont) + integer_range(0LL, (int64_t)size(cont) - 1, rng);
	}

	template <typename RANDOM, typename T, typename PRED>
	auto iterator(T& cont, PRED&& pred, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::size;
		using std::begin;
		using std::end;
		auto begin_it = begin(cont);
		const auto end_it = end(cont);
		const auto valid_count = std::count_if(begin_it, end_it, pred);
		auto item_position = integer_range(0LL, (int64_t)valid_count - 1, rng);
		for (; begin_it != end_it; ++begin_it)
			if (pred(*begin_it) && item_position-- == 0)
				return begin_it;
		return end_it;
	}

	template <typename RANDOM, typename T>
	auto index(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return std::distance(begin(cont), iterator(cont, rng)) - 1;
	}

	template <typename RANDOM, typename T, typename PRED>
	auto index(T& cont, PRED&& pred, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return std::distance(begin(cont), iterator(cont, std::move(pred), rng)) - 1;
	}

	template <typename RANDOM, typename T>
	auto* element(T& cont, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::end;
		auto result = iterator(cont, rng);
		return (result != end(cont)) ? std::to_address(result) : nullptr;
	}
	
	template <typename RANDOM, typename T, typename PRED>
	auto* element(T& cont, PRED&& predicate, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		using std::end;
		auto result = iterator(cont, std::move(predicate), rng);
		return (result != end(cont)) ? std::to_address(result) : nullptr;
	}

	template <typename RANDOM, typename T>
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
			auto Next() { if (mCurrent == mIterators.end()) Shuffle(); return *mCurrent++; }
			void Shuffle() { std::shuffle(mIterators.begin(), mIterators.end(), mRNG); mCurrent = mIterators.begin(); }
		private:
			RANDOM& mRNG;
			std::vector<Iterator> mIterators;
			typename std::vector<Iterator>::iterator mCurrent;
		};

		return Randomizer{ rng, container };
	}
}
