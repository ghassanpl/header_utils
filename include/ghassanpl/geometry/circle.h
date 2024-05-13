/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "shape_concepts.h"
#include <glm/ext/scalar_constants.hpp>

namespace ghassanpl::geometry
{
	template <std::floating_point T>
	struct tcircle
	{
		glm::tvec2<T> center;
		T radius{};

		auto const& centroid() const { return center; }

		constexpr bool is_valid() const noexcept
		{
			return radius >= T{};
		}

		constexpr tcircle valid() const noexcept
		{
			return { center, std::abs(radius) };
		}

		constexpr bool contains(glm::tvec2<T> pt) const noexcept
		{
			const auto d = pt - center;
			return (d.x * d.x) / (radius * radius) + (d.y * d.y) / (radius * radius) <= T(1);
		}

		constexpr T edge_length() const noexcept
		{
			return glm::pi<T>() * 2 * radius;
		}

		constexpr glm::tvec2<T> edge_point_alpha(T t) const
		{
			const auto angle = t * glm::pi<T>() * 2;
			return center + glm::tvec2<T>{cos(angle), sin(angle)} * radius;
		}

		constexpr T calculate_area() const noexcept { return glm::pi<T>() * radius * radius; }
		constexpr glm::tvec2<T> edge_point(T t) const noexcept { return edge_point_alpha(t / edge_length()); }
		constexpr trec2<T> bounding_box() const noexcept { return trec2<T>::from_center_and_half_size(center, {radius, radius}); }

		glm::tvec2<T> closest_point_to(glm::tvec2<T> pt) const
		{
			return center + glm::normalize(pt - center) * radius;
		}
	};

	using circle = tcircle<float>;

	static_assert(area_shape<float, circle>);
}