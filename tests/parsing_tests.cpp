/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/parsing.h"
#include <memory>

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace std::string_view_literals;

TEST(parsing_functions, basics)
{
}

TEST(parsing_functions, decade_parsing)
{
	{
		auto sv = "add 5 to hello"sv;
		auto result = parsing::decade::parse_expression(sv);
		auto fcall = dynamic_cast<parsing::decade::function_call_expression*>(result.get());
		ASSERT_NE(result, nullptr);
		ASSERT_NE(fcall, nullptr);
		EXPECT_EQ(fcall->name, "add:to:");
		auto num = dynamic_cast<parsing::decade::literal_expression*>(fcall->arguments[0].get());
		auto hello = dynamic_cast<parsing::decade::identifier_expression*>(fcall->arguments[1].get());
		ASSERT_NE(num, nullptr);
		ASSERT_NE(hello, nullptr);
		EXPECT_EQ(num->literal.range, "5");
		EXPECT_EQ(hello->identifier, "hello");
	}


	{
		auto sv = "5 + 'hello'"sv;
		auto result = parsing::decade::parse_expression(sv);
		auto fcall = dynamic_cast<parsing::decade::function_call_expression*>(result.get());
		ASSERT_NE(result, nullptr);
		ASSERT_NE(fcall, nullptr);
		EXPECT_EQ(fcall->name, ":+:");
		auto num = dynamic_cast<parsing::decade::literal_expression*>(fcall->arguments[0].get());
		auto str = dynamic_cast<parsing::decade::literal_expression*>(fcall->arguments[1].get());
		ASSERT_NE(num, nullptr);
		ASSERT_NE(str, nullptr);
		EXPECT_EQ(num->literal.range, "5");
		EXPECT_EQ(str->literal.range, "'hello'");
	}
}