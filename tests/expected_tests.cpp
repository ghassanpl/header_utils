/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/expected.h"
#include "tests_common.h"

#include <gtest/gtest.h>
#include "test_system.h"

using namespace ghassanpl;

void should_throw()
{
	expected<int, undroppable<std::string>> e = unexpected("hello");
}
void should_not_throw()
{
	expected<int, undroppable<std::string>> e = 25;
	expected<int, undroppable<std::string>> e2 = unexpected("hello");
	e2.error().handle();
}

TEST(undroppable, works)
{
	EXPECT_ANY_THROW(should_throw());
	EXPECT_NO_THROW(should_not_throw());
}
