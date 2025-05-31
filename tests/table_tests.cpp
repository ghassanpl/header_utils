/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/table.h"
#include "tests_common.h"

#include <gtest/gtest.h>

using namespace ghassanpl;

struct row_type
{
	const std::string id;
	int a;
	int b;
};

TEST(table_type, works)
{
	table<row_type, &row_type::id> t;
	t["yo"].a = 5;
}
