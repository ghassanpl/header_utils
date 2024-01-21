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

TEST(wilson, parses_undelimited_strings_correctly)
{
	EXPECT_EQ(formats::wilson::parse_word_or_string("hello"), "hello");
	EXPECT_EQ(formats::wilson::parse_word_or_string("true"), true);
	EXPECT_EQ(formats::wilson::parse_word_or_string("false"), false);
	EXPECT_EQ(formats::wilson::parse_word_or_string("null"), nullptr);
	EXPECT_EQ(formats::wilson::parse_word_or_string("nil"), nullptr);
	EXPECT_FALSE(formats::wilson::parse_word_or_string("0"));
}

TEST(wilson, outputs_as_string_correctly)
{
	auto result = formats::wilson::parse("{ Required = true, int = 1, float = 5.5, string = 'hello'; arr = [5 6 7], arrpar = (5; 6; 7), n = null\n nested = { nested = {} } }").value();
	EXPECT_EQ(formats::wilson::parse(formats::wilson::to_string(result)).value(), result);
}
