/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/symbol.h"

#include <gtest/gtest.h>

using ghassanpl::symbol;
TEST(symbol_test, symbol_works_on_empty_strings)
{
	symbol::values().clear();

	EXPECT_EQ(symbol{ {} }, symbol{});

	EXPECT_EQ(symbol{ "" }, symbol{ {} });
	EXPECT_EQ("", symbol{ {} });
	EXPECT_EQ(symbol{ "" }, std::string_view{});

	EXPECT_EQ(symbol::values().size(), 0);
}

TEST(symbol_test, symbol_works_on_stringable_objects)
{
	symbol::values().clear();

	std::string str = "hello";
	std::string_view sv = "hello";
	decltype(auto) slit = "hello";
	const char* cstr = "hello";
	
	symbol a{ str };
	symbol b{ sv };
	symbol c{ slit };
	symbol d{ cstr };

	EXPECT_EQ(a, b);
	EXPECT_EQ(a, c);
	EXPECT_EQ(a, d);
	EXPECT_EQ(b, c);
	EXPECT_EQ(b, d);
	EXPECT_EQ(c, d);

	EXPECT_EQ(symbol::values().size(), 1);
}

TEST(symbol_test, doesnt_make_unnecessary_copies)
{
	symbol::values().clear();

	symbol sym{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum." };
	symbol sym2{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum." };

	symbol s2 = sym;
	symbol s3 = s2;

	s3 = sym2;

	std::set<const char*> ptrs;
	ptrs.insert(sym.value.data());
	ptrs.insert(sym2.value.data());
	ptrs.insert(s2.value.data());
	ptrs.insert(s3.value.data());

	EXPECT_EQ(symbol::values().size(), 1);
	EXPECT_EQ(ptrs.size(), 1);
}