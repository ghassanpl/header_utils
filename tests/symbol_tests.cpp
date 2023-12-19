/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/symbol.h"

#include <gtest/gtest.h>

using ghassanpl::symbol;
using ghassanpl::default_symbol_provider;


static_assert(ghassanpl::symbol_provider<default_symbol_provider>);
static_assert(std::regular<symbol>);

TEST(symbol_test, symbol_works_on_empty_strings)
{
	default_symbol_provider::instance().clear();

	EXPECT_EQ(symbol{ {} }, symbol{});

	EXPECT_EQ(symbol{ "" }, symbol{ {} });
	EXPECT_EQ("", symbol{ {} });
	EXPECT_EQ(symbol{ "" }, std::string_view{});

	EXPECT_EQ(default_symbol_provider::instance().size(), 1);

	/// TODO: Also check if empty symbols have the same value across different translation units and across library boundaries
}

TEST(symbol_test, symbol_works_on_stringable_objects)
{
	default_symbol_provider::instance().clear();

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

	default_symbol_provider::instance().clear();
}

TEST(symbol_test, doesnt_make_unnecessary_copies)
{
	default_symbol_provider::instance().clear();

	symbol sym{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum." };
	symbol sym2{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum." };

	symbol s2 = sym;
	symbol s3 = s2;

	s3 = sym2;

	std::set<std::string const*> ptrs;
	ptrs.insert(sym.value);
	ptrs.insert(sym2.value);
	ptrs.insert(s2.value);
	ptrs.insert(s3.value);

	EXPECT_EQ(default_symbol_provider::instance().size(), 2);
	EXPECT_EQ(ptrs.size(), 1);
}

TEST(symbol_test, hashes_properly)
{
	default_symbol_provider::instance().clear();

	symbol sym{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum." };
	symbol sym2{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum." };
	symbol sym3{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum.." };
	symbol sym4{ "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean nec augue libero. Fusce eget ipsum vulputate, rutrum turpis vel, tincidunt nunc. Aliquam erat volutpat. Ut elementum, dui at lacinia lacinia, mauris dolor ornare nisl, vitae bibendum dui odio vitae arcu. Aenean tempor volutpat quam at vestibulum" };

	std::hash<symbol> hasher;
	EXPECT_EQ(hasher(sym), hasher(sym2));
	EXPECT_NE(hasher(sym2), hasher(sym3));
	EXPECT_NE(hasher(sym3), hasher(sym4));
	EXPECT_NE(hasher(sym2), hasher(sym4));
}