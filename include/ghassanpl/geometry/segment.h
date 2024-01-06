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

		tvec vec() const noexcept { return p2 - p1; }
		tvec dir() const noexcept { return glm::normalize(p2 - p1); }

		basic_line_t<T> line() const noexcept { return line_crossing_points(p1, p2); }

		constexpr std::optional<tvec> intersection(tsegment const& other) const noexcept
		{
			/// TODO: Copilot-generated :P Needs testing
			const auto r = p2 - p1;
			const auto s = other.p2 - other.p1;
			const auto rxs = glm::cross(r, s);
			const auto qp = other.p1 - p1;
			const auto qpxr = glm::cross(qp, r);
			if (glm::abs(rxs) < std::numeric_limits<T>::epsilon() && glm::abs(qpxr) < std::numeric_limits<T>::epsilon())
			{
				/// Colinear
				const auto t0 = glm::dot(qp, r) / glm::dot(r, r);
				const auto t1 = t0 + glm::dot(s, r) / glm::dot(r, r);
				if ((t0 >= 0 && t0 <= 1) || (t1 >= 0 && t1 <= 1))
					return std::nullopt;
				return std::nullopt;
			}
			if (glm::abs(rxs) < std::numeric_limits<T>::epsilon() && glm::abs(qpxr) > std::numeric_limits<T>::epsilon())
				return std::nullopt;
			const auto t = glm::cross(qp, s) / rxs;
			const auto u = glm::cross(qp, r) / rxs;
			if (rxs != 0 && t >= 0 && t <= 1 && u >= 0 && u <= 1)
				return p1 + t * r;
			return std::nullopt;
		}

		/// Shape interface

		T edge_length() const { return glm::distance(p1, p2); }
		tvec edge_point_alpha(T t) const { return glm::lerp(p1, p2, t); }
		tvec edge_point(T t) const { return glm::lerp(p1, p2, t / edge_length()); }
		trec2<T> bounding_box() const { return trec2<T>::from_points({ &p1, &p1 + 2 }); }
		tvec projected(tvec pt) const
		{
			const auto dir = this->vec();
			const auto d1 = glm::dot(pt - p1, dir);
			if (d1 <= 0)
				return p1;
			const auto d2 = glm::dot(dir, dir);
			if (d1 > d2)
				return p2;
			return p1 + dir * (d1 / d2);
		}
	};

	using segment = tsegment<float>;

	static_assert(shape<segment, float>);
}