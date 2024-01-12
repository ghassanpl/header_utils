/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "../rec2.h"

namespace ghassanpl::geometry
{
	static_assert(shape<rec2, float>);

	template <typename T>
	tpolygon<T> polygon_from(const trec2<T>& r /*, winding_t winding = clockwise */)
	{
		return tpolygon<T>{
			r.left_top(),
			r.right_top(),
			r.right_bottom(),
			r.left_bottom()
		};
	}

	template <typename T>
	std::array<tsegment<T>, 4> edges_of(const trec2<T>& r /*, winding_t winding = clockwise */)
	{
		return std::array<tsegment<T>, 4>{
			tsegment<T>{r.left_top(), r.right_top()},
			tsegment<T>{r.right_top(), r.right_bottom()},
			tsegment<T>{r.right_bottom(), r.left_bottom()},
			tsegment<T>{r.left_bottom(), r.left_top()}
		};
	}

	/*
	template <typename T>
	tcircle<T> inner_circle(const trec2<T>& r);
	template <typename T>
	tcircle<T> outer_circle(const trec2<T>& r);
	*/
}