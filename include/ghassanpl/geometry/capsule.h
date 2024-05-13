/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "segment.h"
#include "shape_concepts.h"
#include <glm/ext/scalar_constants.hpp>

namespace ghassanpl::geometry
{
	template <std::floating_point T>
	struct tcapsule
	{
		tsegment<T> axis;
		T radius{};

		auto const& centroid() const { return axis.center(); }

		bool contains(glm::tvec2<T> pt) const
		{
			return axis.distance(pt) <= radius;
		}

		T edge_length() const
		{
			return axis.length() * 2 + glm::pi<T>() * 2 * radius;
		}

		glm::tvec2<T> edge_point_alpha(T t) const;

		T calculate_area() const { return axis.length() * 2 * radius + glm::pi<T>() * radius * radius; }
		glm::tvec2<T> edge_point(T t) const { return edge_point_alpha(t / edge_length()); }
		trec2<T> bounding_box() const { return axis.bounding_box().grown(radius); }

		glm::tvec2<T> closest_point_to(glm::tvec2<T> pt) const
		{
			/// https://math.stackexchange.com/questions/475436/2d-point-projection-on-an-ellipse
			throw "unimplemented";
		}
	};

	using capsule = tcapsule<float>;

	static_assert(area_shape<float, capsule>);
}