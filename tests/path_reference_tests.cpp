/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/path_reference.h"

#include <gtest/gtest.h>
#include <print>

using namespace ghassanpl;

TEST(path_reference, lexing_basics_work)
{
	caching_path_reference<int*> pr;
	pr = "hello";
	pr.pointer();
	pr.path();
	pr.path_empty();
	std::string s;
	pr == "hello";
	pr == pr;
	pr == s;
	pr == (int*)0;
	pr <=> "hello";
	pr <=> pr;
	pr <=> s;
	pr <=> (int*)0;

	if (pr) {}
	std::string path{ pr };
}
