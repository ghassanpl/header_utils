/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/not-ready/paths.h"
#include <memory>
#include <filesystem>
#include <print>

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace ghassanpl::string_ops;
using namespace std::string_view_literals;
namespace fs = std::filesystem;

TEST(paths, basics)
{
	const std::u8string_view s = u8"aąsdf";
	const auto comp = make_sv(s);
	
	virtual_path path = string_ops::to_utf8<std::string>(s);
	EXPECT_EQ(path, comp);
	//std::println("{}", path.native());
	
	virtual_path path_from_u8 = u8"aąsdf";
	EXPECT_EQ(path_from_u8, comp);

	virtual_path path_from_c16 = u"aąsdf";
	EXPECT_EQ(path_from_c16, comp);

	virtual_path path_from_c32 = U"aąsdf";
	EXPECT_EQ(path_from_c32, comp);

	virtual_path path_from_wc = L"aąsdf";
	EXPECT_EQ(path_from_wc, comp);

	/*
	fs::path p{ u8"aąsdf" };
	virtual_path path_from_fspath = p;
	EXPECT_EQ(path_from_fspath, comp);
	path_from_fspath = p;
	EXPECT_EQ(path_from_fspath, comp);
	*/

	virtual_path copy = path;
	copy = path;
	virtual_path moved = std::move(copy);
	moved = std::move(moved);

	//*/
}