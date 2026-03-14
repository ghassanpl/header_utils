/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifndef __clang__

#define ASSUMING_TESTS
#define ASSUMING_INCLUDE_MAGIC_ENUM 0
#define ASSUMING_DEBUG 0
#include "../include/ghassanpl/assuming.h"
#include "tests_common.h"

#include <gtest/gtest.h>
#include <format>
#include <thread>

TEST(assuming, works_under_ndebug)
{
	Assuming(false); /// ensure handler not triggered under ndebug

	Assuming(true);
	AssumingNotRecursive();
	AssumingSingleThread();
	AssumingOnThread(std::this_thread::get_id());

	AssumingNull((void*)nullptr); /// because `std::to_address(nullptr)` does not compile...
	AssumingNotNull(this);
	AssumingBinOp(0, 0, ==, "equal to");
	AssumingEqual(0, 0);
	AssumingZero(0);
	AssumingNotEqual(0, 10);
	AssumingGreater(10, 0);
	AssumingLess(0, 10);
	AssumingGreaterEqual(5, 5);
	AssumingLessEqual(6, 6);
	auto meh = std::vector{ 5,6,7 };
	AssumingEmpty(std::vector<int>{});
	AssumingNotEmpty(meh);
	const char* test = nullptr;
	AssumingNullOrEmpty(test);
	AssumingNotNullOrEmpty("hello");
	AssumingValidIndex(2, meh);
	AssumingValidIterator(meh.begin() + 2, meh);
	AssumingBetween(5, 0, 10);
	AssumingBetweenInclusive(10, 0, 10);
	AssumingContainsBits(4, 0xFF);
	std::set<int> set{ 0, 10, 20 };
	AssumingContains(10, set);

	return;

	AssumingNotReachable();
}

#endif