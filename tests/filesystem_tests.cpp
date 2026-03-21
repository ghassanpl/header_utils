/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/filesystem.h"

#include <gtest/gtest.h>

#include "test_system.h"

using namespace std;
using namespace ghassanpl;
using namespace ghassanpl::fs;

TEST(path_append, works_like_operator_slash)
{
	path p1 = "C:\\Users\\Ghassan\\Desktop\\test.txt";
	path rel_dir = "world/blah/";
	path rel_file = "world/blah/bleh";

	{
		const string base = "hello";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
	{
		const string base = "hello/";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
	{
		const string base = "hello/asd///";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
	{
		const string base = "";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}

	{
		const string base = "hello ";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
	{
		const string base = "hello/ ";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
	{
		const string base = "hello/asd/// ";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
	{
		const string base = "  ";
		string s1 = base;
		path_append(s1, rel_dir);
		EXPECT_EQ(s1, base / rel_dir);
	}
}
