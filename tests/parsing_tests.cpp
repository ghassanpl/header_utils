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

TEST(wilson_parsing, doesnt_crash_or_loop_on_invalid_values)
{
	formats::wilson::parse_array("}"); formats::wilson::parse_object("}"); formats::wilson::parse_value("}");
	formats::wilson::parse_array("]"); formats::wilson::parse_object("]"); formats::wilson::parse_value("]");
	formats::wilson::parse_array(")"); formats::wilson::parse_object(")"); formats::wilson::parse_value(")");
	formats::wilson::parse_array(","); formats::wilson::parse_object(","); formats::wilson::parse_value(",");
	//EXPECT_EQ(, nlohmann::json{});
}

auto wilson_decade = R"([ChangeImageOf [this] ToTile chest_open]
[Play chest_open At [here] Waiting false]
[AddLog [Phrase OpenChest FromGroup logs]]
Once)"sv;

TEST(parsing_functions, wilson_parsing_basic_decade)
{
	auto parsed = formats::wilson::parse_array(wilson_decade);
	std::cout << parsed.dump(1) << "\n";
	EXPECT_EQ(parsed.size(), 4);
}