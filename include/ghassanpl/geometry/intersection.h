/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "shape_concepts.h"
#include "circle.h"
#include "triangles.h"
#include "squares.h"
#include "segment.h"
#include "ray.h"
#include "polygon.h"
#include "ellipse.h"

namespace ghassanpl::geometry
{
	/// TODO: Steal from:
	/// https://github.com/RandyGaul/cute_headers/blob/master/cute_c2.h

	template <typename T, typename U>
	bool intersects(T const&, U const&) noexcept = delete;

	template <typename T>
	bool intersects(trec2<T> const& a, trec2<T> const& b) noexcept
	{
		return a.intersects(b);
	}

	template <typename T>
	bool intersects<tcircle<T>, tcircle<T>>(tcircle<T> const& a, tcircle<T> const& b) noexcept
	{
		/// TODO: sqrt is expensive, use squared distance instead
		return glm::distance(a.center, b.center) < a.radius + b.radius; /// <= ?
	}

	/*
	int c2CircletoAABB(c2Circle A, c2AABB B)
	{
		c2v L = c2Clampv(A.p, B.min, B.max);
		c2v ab = c2Sub(A.p, L);
		float d2 = c2Dot(ab, ab);
		float r2 = A.r * A.r;
		return d2 < r2;
	}
	*/
	/// Circle vs Capsule: https://github.com/RandyGaul/cute_headers/blob/master/cute_c2.h#L1393

	template <typename T>
	struct tshape_intersection
	{
		enum shape_relation relation {};
		glm::tvec3<T> points[2] {};
		glm::tvec2<T> normal {};
	};

	template <typename T, typename U>
	void intersection(T const&, U const&) noexcept = delete;

}
