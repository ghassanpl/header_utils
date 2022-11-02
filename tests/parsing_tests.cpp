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

	{
		auto str = R"(
project 'proj'
| ada
| 	for Languages use ("ada", "c");
| 	for Main use ("main.adb");
| - approval testing? here or in module?
| - code coverage ?

	author 'Ghassan Al-Mashareqa'
	company 'Ghassan.pl'
	copyright '2020 Ghassan.pl'
	type 'executable' | executable/dll/etc
	license 'MIT' at 'some-where-out-there'
	| maybe something like
	|		'license' [filename] ['from' url]
	| readme ?s
	| import modules from external sources
	|	- urls
	| - other directories
	| - .h/.dll modules/libraries
	| warnings/errors
	| publishing? or pre/post build stuff?
	| settings/resources?

	dependencies
		'magic_enum' from 'vcpkg/magic_enum'

	vcs
		remote 'origin' at 'http//github.com/ghassanpl/dec-test-project'
		ignore '*.png'

	editing
		indent 'tab' size 4
		line-endings 'crlf'

	options
		option 'use-format' is true | Use format library

	root '.'
		directory 'build'
			vcs 'ignore' | ignores this directory
			stores 'objects'
			
		| directory 'build' stores objects
		directory 'tests'
			stores 'test data'
			stores 'test results'
			

		directory 'external'
			directory 'format'
				stores-external-project 'http//github.com/blah/bleh'
				

		if 'use-format'
			directory 'external/format' stores-external-project 'http//github.com/blah/bleh'
			

		directory 'log'
			stores 'build logs'
			stores 'execution logs'
			
	
		directory 'bench' stores 'benchmark results'
		directory 'output' stores 'output'
		directory 'api' stores 'api'
		directory 'cache' stores 'cache'
		directory 'docs' stores 'documentation'

		directory 'src'
			source '*.dec'
			directory 'other'
				source 'abc.dec'
				

	build
		| specifies the build process, pre- and post- events
	
	install
		| specifies the install process
		
		)";
		/*
		if, else, else if
		for
			for a in b
			for a from x to y (by n)
			for a from x through y (by n)
		while
		continue [for/while]
		break [for/while/case]
		return
		throw
		assuming
		let
		ignore/drop/discard
		| future stuff: 
		|   match/case/switch + when
		|
		|		let location = person @ "location"
		|   else
		|			print("I hope the weather is nice near you.")
		|			return



		function
		class

		let str = "bleh"
		| either
		condition e means str is empty
		| or
		let e mean str is empty
		assuming e is false
		set str to ""
		assuming e is true

		expr:
			new
			lambda
	*/
		auto lines = parsing::decade::parse_lines(str);
		EXPECT_EQ(lines->sub_lines.size(), 15);
	}
}