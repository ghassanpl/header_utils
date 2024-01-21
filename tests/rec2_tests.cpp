/// This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "../include/ghassanpl/rec2.h"
#include "../include/ghassanpl/geometry/shape_concepts.h"

#include <gtest/gtest.h>

using namespace ghassanpl;
using namespace ghassanpl::geometry;

static_assert(area_shape<trec2<float>, float>);
static_assert(area_shape<trec2<double>, double>);
static_assert(area_shape<trec2<int>, int>);

#ifdef __cpp_explicit_this_parameter
TEST(rec2, values_func_forwards_value_category)
{
	irec2 a{ 1, 2, 3, 4 };
	const irec2 b{ 1, 2, 3, 4 };

	{
		auto&& [left, top, right, bottom] = a.values();
		static_assert(std::is_lvalue_reference_v<decltype(left)>);
		static_assert(!std::is_const_v<std::remove_reference_t<decltype(left)>>);
	}
	{
		auto&& [left, top, right, bottom] = b.values();
		static_assert(std::is_lvalue_reference_v<decltype(left)>);
		static_assert(std::is_const_v<std::remove_reference_t<decltype(left)>>);
	}
	{
		auto&& [left, top, right, bottom] = std::move(a).values();
		static_assert(!std::is_reference_v<decltype(left)>);
		static_assert(!std::is_const_v<std::remove_reference_t<decltype(left)>>);
	}
}
#endif

TEST(rec2, bounding_box_for_overload_works)
{
	using glm::vec2;
	rec2 r{ bounding_box_for, vec2{1,1}, vec2{3,3}, vec2{2,5} };
	EXPECT_EQ(r, rec2(1.0f, 1.0f, 3.0f, 5.0f));

	rec2 r2{ bounding_box_for, vec2{-100,100}, vec2{-300,3}, vec2{302,544} };
	EXPECT_EQ(r2, rec2(-300.0f, 3.0f, 302.0f, 544.0f));
}
