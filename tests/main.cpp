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

int main(int argc, char** argv)
{
	//ghassanpl::tests::TestRunner::RunTests();
	::testing::InitGoogleTest(&argc, argv);

	//unititialized_t<std::string> un;
	//un.brace_init("hello");
	//assert(*un == "hello");

	return RUN_ALL_TESTS();
}