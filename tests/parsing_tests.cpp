/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/parsing.h"
#include "../include/ghassanpl/wilson.h"
#include <memory>

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_view_literals;

TEST(parsing_functions, basics)
{
}

constexpr auto wilson_decade = R"([ChangeImageOf [this] ToTile chest_open]
[Play chest_open At [here] Waiting false]
[AddLog [Phrase OpenChest FromGroup logs]]
Once)"sv;

TEST(parsing_functions, wilson_parsing_basic_decade)
{
	auto parsed = formats::wilson::parse_array(wilson_decade).value();
	//std::cout << parsed.dump(1) << "\n";
	EXPECT_EQ(parsed.size(), 4);
}