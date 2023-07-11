/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "shape_concepts.h"

namespace ghassanpl::geometry
{
	template <std::floating_point T>
	struct tellipse
	{
		glm::tvec2<T> center;
		glm::tvec2<T> radii;

		static tellipse from_rect(trec2<T> const& rec) noexcept
		{
			return { rec.center(), rec.half_size() };
		}

		bool contains(glm::tvec2<T> pt) const
		{
			const auto d = pt - center;
			return (d.x * d.x) / (radii.x * radii.x) + (d.y * d.y) / (radii.y * radii.y) <= T(1);
		}

		T edge_length() const
		{
			/// Ramanujan was a smart cookie.
			/// Shamelessly stolen from https://www.mathsisfun.com/geometry/ellipse-perimeter.html
			const auto a = std::max(radii.x, radii.y);
			const auto b = std::min(radii.x, radii.y);
			const auto h = ((a - b) * (a - b)) / ((a + b) * (a + b));
			return glm::pi<T>() * (a + b) * (1 + ((3 * h) / (10 + sqrtf(4 - 3 * h))));
		}
		
		glm::tvec2<T> edge_point_alpha(T t) const
		{
			throw "unimplemented";
		}
		
		T calculate_area() const { return glm::pi<T>() * radii.x * radii.y; }
		glm::tvec2<T> edge_point(T t) const { return edge_point_alpha(t / edge_length()); }
		trec2<T> bounding_box() const { return trec2<T>{center - radii, center + radii}; }
		
		glm::tvec2<T> projected(glm::tvec2<T> pt) const
		{
			/// https://math.stackexchange.com/questions/475436/2d-point-projection-on-an-ellipse
			throw "unimplemented";
		}
	};

	using ellipse = tellipse<float>;

	static_assert(area_shape<ellipse, float>);
}