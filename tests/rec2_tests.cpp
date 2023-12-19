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
