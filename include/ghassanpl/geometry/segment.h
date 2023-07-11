/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{

	template <std::floating_point T>
	struct tsegment
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		tvec p1{};
		tvec p2{};

		T edge_length() const { return glm::distance(p1, p2); }
		glm::tvec2<T> edge_point_alpha(T t) const { return glm::lerp(p1, p2, t); }
		glm::tvec2<T> edge_point(T t) const { return glm::lerp(p1, p2, t / edge_length()); }
		trec2<T> bounding_box() const { return trec2<T>::from_points({ &p1, &p1 + 2 }); }
		glm::tvec2<T> projected(glm::tvec2<T> pt) const;
	};

	using segment = tsegment<float>;

	static_assert(shape<segment, float>);
}