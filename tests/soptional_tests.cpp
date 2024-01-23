/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/soptional.h"

#include <gtest/gtest.h>

TEST(soptional_test, basics_work)
{
	ghassanpl::sentinel_optional<int> opt;
	ghassanpl::sentinel_optional<int> opt2 = 4;
	(void)opt2;
	EXPECT_FALSE(opt.has_value());
	opt = 5;
	EXPECT_TRUE(opt.has_value());
	EXPECT_EQ(opt.value(), 5);
	opt.reset();
	EXPECT_FALSE(opt.has_value());
	EXPECT_EQ(std::bit_cast<int>(opt), 0);
	
	opt = 0;
	EXPECT_FALSE(opt.has_value());
}

TEST(soptional_test, works_with_set_sentinel)
{
	ghassanpl::sentinel_optional<int, -1> opt;
	EXPECT_FALSE(opt.has_value());
	opt = 5;
	EXPECT_TRUE(opt.has_value());
	EXPECT_EQ(opt.value(), 5);
	opt.reset();
	EXPECT_FALSE(opt.has_value());
	EXPECT_EQ(std::bit_cast<int>(opt), -1);

	opt = -1;
	EXPECT_FALSE(opt.has_value());
}