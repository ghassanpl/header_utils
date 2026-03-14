/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.


#include "../include/ghassanpl/pools.h"
#include "tests_common.h"
#include <print>
#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_literals;

TEST(pool, works)
{
	{
		pool<int, 1024> p;

		EXPECT_EQ(p.allocated_elements(), 0);
		EXPECT_EQ(p.free_elements(), 0);
		EXPECT_EQ(p.capacity(), 0);
		EXPECT_EQ(p.capacity_in_bytes(), 0);

		auto ptr = p.create(10);

		EXPECT_NE(ptr, nullptr);
		EXPECT_EQ(*ptr, 10);

		EXPECT_EQ(p.allocated_elements(), 1);
		EXPECT_EQ(p.free_elements(), 1024 - 1);
		EXPECT_EQ(p.capacity(), 1024);
		EXPECT_EQ(p.capacity_in_bytes(), 1024 * sizeof(int));

		auto ptr2 = p.create(100);

		EXPECT_NE(ptr2, nullptr);
		EXPECT_EQ(*ptr2, 100);

		EXPECT_EQ(p.allocated_elements(), 2);
		EXPECT_EQ(p.free_elements(), 1024 - 2);
		EXPECT_EQ(p.capacity(), 1024);
		EXPECT_EQ(p.capacity_in_bytes(), 1024 * sizeof(int));


		p.destroy(ptr);

		EXPECT_EQ(p.allocated_elements(), 1);
		EXPECT_EQ(p.free_elements(), 1024 - 1);
		EXPECT_EQ(p.capacity(), 1024);
		EXPECT_EQ(p.capacity_in_bytes(), 1024 * sizeof(int));

		auto ptr3 = p.create(1000);

		EXPECT_NE(ptr3, nullptr);
		EXPECT_EQ(*ptr3, 1000);

		EXPECT_EQ(p.allocated_elements(), 2);
		EXPECT_EQ(p.free_elements(), 1024 - 2);
		EXPECT_EQ(p.capacity(), 1024);
		EXPECT_EQ(p.capacity_in_bytes(), 1024 * sizeof(int));

		p.destroy(ptr2);
		p.destroy(ptr3);

		EXPECT_EQ(p.allocated_elements(), 0);
		EXPECT_EQ(p.free_elements(), 1024 - 0);
		EXPECT_EQ(p.capacity(), 1024);
		EXPECT_EQ(p.capacity_in_bytes(), 1024 * sizeof(int));
	}

	{
		pool<int, 2> p;

		auto p1 = p.create(10);
		EXPECT_EQ(p.capacity(), 2);
		auto p2 = p.create(20);
		auto p3 = p.create(30);
		EXPECT_EQ(p.capacity(), 4);
		p.destroy(p1);
		EXPECT_EQ(p.capacity(), 4);
		p.destroy(p2);
		EXPECT_EQ(p.capacity(), 4);
		p.destroy(p3);

		EXPECT_EQ(p.allocated_elements(), 0);
		EXPECT_EQ(p.free_elements(), 4);
		EXPECT_EQ(p.capacity(), 4);
		EXPECT_EQ(p.capacity_in_bytes(), 4 * sizeof(int));
	}
}
