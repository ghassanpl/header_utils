/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include "test_system.h"

#include "../include/ghassanpl/mmap.h"
#include "../include/ghassanpl/mmap_impl.h"
#include "../include/ghassanpl/bytes.h"
#include "../include/ghassanpl/string_ops.h"
#include "../include/ghassanpl/uninitialized.h"
#include <print>

using namespace ghassanpl;
using namespace ghassanpl::string_ops;
using namespace std;

/*
consteval auto builder(std::string_view str)
{
	std::vector<int> result{ 1,2,3,4,5 };
	return result;
}

consteval auto func(std::string_view str)
{
	return [str] {
		auto const int_vec = builder(str);
		std::array<int, 16> result{};
		auto const end_pos = ranges::copy(int_vec, ranges::begin(result)).out;
		auto const right_size = ranges::distance(ranges::begin(result), end_pos);
		return std::pair{ result, right_size };
	}();
}
*/

int main(int argc, char** argv)
{
	//auto bleh = func("asd");
	//ghassanpl::tests::TestRunner::RunTests();
	::testing::InitGoogleTest(&argc, argv);

	//unititialized_t<std::string> un;
	//un.brace_init("hello");
	//assert(*un == "hello");

	return RUN_ALL_TESTS();
}