/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/formats.h"
#include "../include/ghassanpl/wilson.h"
#include "../include/ghassanpl/json_helpers.h"

#include <gtest/gtest.h>

using namespace ghassanpl;

TEST(wilson_parsing, doesnt_crash_or_loop_on_invalid_values)
{
	auto results = { 
		formats::wilson::parse_array("}"), formats::wilson::parse_object("}"), formats::wilson::parse("}"),
		formats::wilson::parse_array("]"), formats::wilson::parse_object("]"), formats::wilson::parse("]"),
		formats::wilson::parse_array(")"), formats::wilson::parse_object(")"), formats::wilson::parse(")"),
		formats::wilson::parse_array(","), formats::wilson::parse_object(","), formats::wilson::parse(","),

		formats::wilson::parse("{ Walking -> Running }"),
		formats::wilson::parse("[ Walking -> Running ]"),
		formats::wilson::parse("Walking -> Running"),
		formats::wilson::parse("{ Walking : Running }"),
		formats::wilson::parse("[ Walking : Running ]"),
	};
	//EXPECT_EQ(, nlohmann::json{});
}

TEST(wilson_parsing, will_parse_map_with_key_but_no_value)
{
	auto result = formats::wilson::parse_object("Required)", ')');
	EXPECT_EQ(result, nlohmann::json::object({ {"Required", true} }));
}
