/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/scope.h"
#include "tests_common.h"

#include <gtest/gtest.h>

using namespace ghassanpl;


TEST(scoped_value_change, works)
{
	int v = 5;
	{
		scoped_value_change scoped_value{ v, 10 };
		ASSERT_TRUE(scoped_value.valid());
		EXPECT_EQ(v, 10);
	}
	EXPECT_EQ(v, 5);

	{
		scoped_value_change scoped_value{ v, 10 };
		scoped_value.revert();
		ASSERT_FALSE(scoped_value.valid());
		EXPECT_EQ(v, 5);
	}

	{
		scoped_value_change scoped_value{ v, 10 };
		scoped_value.release();
		ASSERT_FALSE(scoped_value.valid());
	}
	EXPECT_EQ(v, 10);


	{
		scoped_value_change scoped_value{ v, 5 };
		scoped_value_change scoped_value2 = std::move(scoped_value);
		scoped_value.revert();
		ASSERT_FALSE(scoped_value.valid());
		ASSERT_TRUE(scoped_value2.valid());
		EXPECT_EQ(v, 5);
	}
	EXPECT_EQ(v, 10);
}

TEST(optional_scoped_value_change, works)
{
	const char* data = "hello";
	char* data2 = _strdup(data);
	std::string_view v = data;
	{
		optional_scoped_value_change<std::string_view> scoped_value{ v, data2 };
		EXPECT_EQ(v.data(), data);
	}
	EXPECT_EQ(v.data(), data);
	free(data2);
}
