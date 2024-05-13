/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include "geometry_common.h"
#include "points.h"
#include "triangles.h"

namespace ghassanpl::geometry
{
	static inline constexpr size_t invalid_index = std::numeric_limits<size_t>::max();

	template <typename T>
	struct vertex_weight
	{
		size_t index = invalid_index;
		T weight = {};
	};
	/// TODO: https://github.com/delfrrr/delaunator-cpp/blob/master/include/delaunator.hpp
	/// https://en.wikipedia.org/wiki/Delaunay_triangulation

	template <typename T>
	struct delaunay_triangulation
	{
		using tvec = glm::tvec2<T>;

		tvec_span<T> points;
		std::vector<tindexed_triangle<>> triangles;
		/// TODO: hull
		/// TODO: halfedges
		/// TODO: voronoi data
		
		size_t triangle_at(tvec const& pt) const;
		
		std::array<vertex_weight<T>, 3> interpolation_of(tvec const& pt) const
		{
			const auto idx = triangle_at(pt);
			if (idx == invalid_index)
				return {};
			const auto tri = triangles[idx].as_triangle(points);
			const auto bary = tri.barycentric_coords_of(pt);
			return {
				vertex_weight{triangles[idx].indices[0], bary.x},
				vertex_weight{triangles[idx].indices[1], bary.y},
				vertex_weight{triangles[idx].indices[2], bary.z}
			};
		}
	};

	template <typename T, typename FUNC>
	auto linear_interpolation(std::span<vertex_weight<T>> weights, FUNC&& value_func)
	{
		auto result = T{};
		auto weight_sum = T{}; /// Should sum to 1.0 in most cases, as weights are normalized baricentric coordinates
		for (auto const& vw : weights)
		{
			result += vw.weight * value_func(vw.index);
			weight_sum += vw.weight;
		}
		return result /= weight_sum;
	}
}