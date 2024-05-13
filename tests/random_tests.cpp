/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "tests_common.h"
#include <gtest/gtest.h>

#include "../include/ghassanpl/random.h"
#include "../include/ghassanpl/random_geom.h"
#include "../include/ghassanpl/random_seq.h"

#include <ranges>
#include <print>

using namespace ghassanpl;

static_assert(std::uniform_random_bit_generator<random::philox64_engine>);

TEST(random, basics)
{
	ignore = random::integer();

	for (int i = 0; i < 100; ++i)
	{
		auto f = random::percentage();
		EXPECT_GE(f, 0.0) << i;
		EXPECT_LE(f, 1.0) << i;

		auto d6 = random::dice(6);
		EXPECT_GE(d6, 1) << i;
		EXPECT_LE(d6, 6) << i;

		auto twod6 = random::dice(2, 6);
		EXPECT_GE(twod6, 2) << i;
		EXPECT_LE(twod6, 12) << i;

		auto d6_ = random::dice<6>();
		EXPECT_GE(d6_, 1) << i;
		EXPECT_LE(d6_, 6) << i;

		auto twod6_ = random::dice<2, 6>();
		EXPECT_GE(twod6_, 2) << i;
		EXPECT_LE(twod6_, 12) << i;

		auto ir = random::in_integer_range(-50, 100);
		EXPECT_GE(ir, -50) << i;
		EXPECT_LE(ir, 100) << i;

		auto rr = random::in_real_range(-5.2, 10.6);
		EXPECT_GE(rr, -5.2) << i;
		EXPECT_LE(rr, 10.6) << i;
	}

	ignore = random::in_range('a', 'z');

	ignore = random::normal();

	ignore = random::coin();

	ignore = random::halton_sequence(15);

	ignore = random::with_probability(0.5);
	double res = 0;
	ignore = random::with_probability(0.3, res);

	ignore = random::one_in(5);

	std::vector<int> woo{ 5,1,6,3,34,234,234,23 };
	for (int i = 0; i < 100; ++i)
	{
		auto it = random::iterator(woo);
		EXPECT_GE(std::to_address(it), woo.data());
		EXPECT_LT(std::to_address(it), woo.data() + woo.size());
		EXPECT_NO_THROW(std::ignore = *it);

		auto ite = random::iterator_if(woo, [](auto v) { return (v % 2); });
		EXPECT_GE(std::to_address(ite), woo.data());
		EXPECT_LT(std::to_address(ite), woo.data() + woo.size());
		EXPECT_NO_THROW(std::ignore = *ite);
		EXPECT_EQ(((*ite) % 2), 1);

		auto ix = random::index(woo);
		EXPECT_GE(ix, 0);
		EXPECT_LT(ix, (ptrdiff_t)woo.size());

		auto ixe = random::index_if(woo, [](auto v) { return (v % 2); });
		EXPECT_GE(ixe, 0);
		EXPECT_LT(ixe, (ptrdiff_t)woo.size());
		EXPECT_EQ((woo[ixe] % 2), 1);

		{
			auto e = random::element(woo);
			EXPECT_GE(std::to_address(e), woo.data());
			EXPECT_LT(std::to_address(e), woo.data() + woo.size());

			auto ee = random::element_if(woo, [](auto v) { return (v % 2); });
			EXPECT_GE(std::to_address(ee), woo.data());
			EXPECT_LT(std::to_address(ee), woo.data() + woo.size());
			EXPECT_EQ(((*ee) % 2), 1);
		}

		ignore = random::one_of(1, 2, 3, 4, 5, 6, 7);
		auto il = { 1, 2, 3, 4, 5, 6, 7 };
		auto v = random::one_of(il);
		EXPECT_GE(v, 1);
		EXPECT_LE(v, 7);
	}
}
TEST(random_seq, philox64_gives_reasonable_results)
{
	//auto results = std::ranges::to<std::vector>(std::views::iota(0, 16) | std::views::transform([](auto i) { return random::philox64(i, 0xCAFEBEEB); }));
	//auto results2 = std::ranges::to<std::vector>(std::views::iota(0, 16) | std::views::transform([](auto i) { return random::philox64(i, 0xCAFEBEEB); }));
	//auto results3 = std::ranges::to<std::vector>(std::views::iota(0, 16) | std::views::transform([](auto i) { return random::philox64(i, 0xCAFEBEEC); }));
	//auto results4 = std::ranges::to<std::vector>(std::views::iota(1, 17) | std::views::transform([](auto i) { return random::philox64(i, 0xCAFEBEEB); }));
	//EXPECT_EQ(results, results2);
	//EXPECT_NE(results, results3);
	//EXPECT_NE(results, results4);
}

TEST(random_seq, philox64_engine_gives_reasonable_results)
{
	//random::philox64_engine engine{ 0, 0 };
	//std::uniform_real_distribution<double> dist(0.0, 100.0);
	//
	//auto results = std::ranges::to<std::vector>(std::views::iota(0, 16) | std::views::transform( [&](auto i) { return dist(engine); }));
	//engine.reset(0, 0);
	//auto results2 = std::ranges::to<std::vector>(std::views::iota(0, 16) | std::views::transform([&](auto i) { return dist(engine); }));
	//engine.reset(10, 20);
	//auto results3 = std::ranges::to<std::vector>(std::views::iota(0, 16) | std::views::transform([&](auto i) { return dist(engine); }));
	//EXPECT_EQ(results, results2);
	//EXPECT_NE(results, results3);
}
