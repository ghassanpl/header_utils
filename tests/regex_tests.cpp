/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/regex.h"

#include <gtest/gtest.h>

#include "test_system.h"

using namespace std;
using namespace ghassanpl;
using namespace ghassanpl::regex;

TEST(regex_split, works)
{
	std::vector<std::string> result;
	regex_split("a b c", std::regex{ " " }, op::push_back_to(result));
	EXPECT_EQ(result, (std::vector<std::string>{ "a", "b", "c" }));

	result = regex_split("a b c", std::regex{ " " });
	EXPECT_EQ(result, (std::vector<std::string>{ "a", "b", "c" }));
}
