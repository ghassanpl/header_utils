/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "../include/ghassanpl/random.h"
#include "../include/ghassanpl/random_geom.h"
#include "../include/ghassanpl/random_seq.h"

using namespace ghassanpl;

TEST(random, basics)
{
	random::integer();

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

	random::in_range('a', 'z');

	random::normal();

	random::coin();

	random::halton_sequence(15);

	random::with_probability(0.5);
	double res = 0;
	random::with_probability(0.3, res);

	random::one_in(5);

	std::vector<int> woo{ 5,1,6,3,34,234,234,23 };
	for (int i = 0; i < 100; ++i)
	{
		auto it = random::iterator(woo);
		EXPECT_GE(std::to_address(it), woo.data());
		EXPECT_LT(std::to_address(it), woo.data() + woo.size());
		EXPECT_NO_THROW(std::ignore = *it);

		auto ite = random::iterator(woo, [](auto v) { return (v % 2); });
		EXPECT_GE(std::to_address(ite), woo.data());
		EXPECT_LT(std::to_address(ite), woo.data() + woo.size());
		EXPECT_NO_THROW(std::ignore = *ite);
		EXPECT_EQ(((*ite) % 2), 1);

		auto ix = random::index(woo);
		EXPECT_GE(ix, 0);
		EXPECT_LT(ix, (ptrdiff_t)woo.size());

		auto ixe = random::index(woo, [](auto v) { return (v % 2); });
		EXPECT_GE(ixe, 0);
		EXPECT_LT(ixe, (ptrdiff_t)woo.size());
		EXPECT_EQ((woo[ixe] % 2), 1);

		{
			auto e = random::element(woo);
			EXPECT_GE(std::to_address(e), woo.data());
			EXPECT_LT(std::to_address(e), woo.data() + woo.size());

			auto ee = random::element(woo, [](auto v) { return (v % 2); });
			EXPECT_GE(std::to_address(ee), woo.data());
			EXPECT_LT(std::to_address(ee), woo.data() + woo.size());
			EXPECT_EQ(((*ee) % 2), 1);
		}

		random::one_of(1, 2, 3, 4, 5, 6, 7);
		auto il = { 1, 2, 3, 4, 5, 6, 7 };
		auto v = random::one_of(il);
		EXPECT_GE(v, 1);
		EXPECT_LE(v, 7);
	}
}