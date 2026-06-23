/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "../include/ghassanpl/containers.h"

TEST(map_find_value, works)
{
	std::map<int, std::string> m;
	auto [it, ins] = m.emplace(0, "hello");
	EXPECT_NE(ghassanpl::map_find_value(m, &it->second), nullptr);
}