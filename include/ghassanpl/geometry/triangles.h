/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "./segment.h"

namespace ghassanpl::geometry
{

	template <std::floating_point T>
	struct ttriangle
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;

		tvec a{};
		tvec b{};
		tvec c{};

		template <typename FUNC>
		void for_each_edge(FUNC&& func) const
		{
			func(a, b);
			func(b, c);
			func(c, a);
		}

		trec2<T> bounding_box() const
		{
			trec2<T> res;
			res += a; res += b; res += c;
			return res;
		}

		T edge_length() const
		{
			return glm::distance(a, b) + glm::distance(b, c) + glm::distance(c, a);
		}

		tvec edge_point_alpha(T t) const;
		tvec edge_point(T t) const;
		tvec closest_point_to(tvec pt) const;

		static auto sign(tvec const& p1, tvec const& p2, tvec const& p3) noexcept
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}

		winding_order winding() const noexcept
		{
			const auto d = sign(a, b, c);
			return d > 0 ? winding_order::clockwise : winding_order::counter_clockwise;
		}

		bool contains(tvec pt) const noexcept
		{
			const auto d1 = sign(pt, a, b);
			const auto d2 = sign(pt, b, c);
			const auto d3 = sign(pt, c, a);
			const auto has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			const auto has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
			return !(has_neg && has_pos);
		}

		auto calculate_area() const noexcept
		{
			const auto A = std::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
			const auto B = std::sqrt((b.x - c.x) * (b.x - c.x) + (b.y - c.y) * (b.y - c.y));
			const auto C = std::sqrt((b.x - c.x) * (a.x - c.x) + (a.y - c.y) * (a.y - c.y));
			const auto s = (A + B + C) * T(0.5);
			return std::sqrt(s * (s - A) * (s - B) * (s - C));
		}

		auto centroid() const
		{
			return (a + b + c) / T(3);
		}

		glm::tvec3<T> barycentric_coords_of(glm::tvec2<T> pt) const
		{
			const auto v0 = b - a;
			const auto v1 = c - a;
			const auto v2 = pt - a;
			const auto d00 = glm::dot(v0, v0);
			const auto d01 = glm::dot(v0, v1);
			const auto d11 = glm::dot(v1, v1);
			const auto d20 = glm::dot(v2, v0);
			const auto d21 = glm::dot(v2, v1);
			const auto denom = d00 * d11 - d01 * d01;
			const auto v = (d11 * d20 - d01 * d21) / denom;
			const auto w = (d00 * d21 - d01 * d20) / denom;
			const auto u = 1 - v - w;
			return { u, v, w };
		}

		glm::tvec2<T> euclidean_coords_of(glm::tvec3<T> barycentric) const
		{
			return a * barycentric.x + b * barycentric.y + c * barycentric.z;
		}

		/*
		tcircle<T> circumcircle()
		{
			Doub a0, a1, c0, c1, det, asq, csq, ctr0, ctr1, rad2;
			a0 = a.x[0] - b.x[0]; a1 = a.x[1] - b.x[1];
			c0 = c.x[0] - b.x[0]; c1 = c.x[1] - b.x[1];
			det = a0 * c1 - c0 * a1;
			if (det == 0.0) throw("no circle thru colinear points");
			det = 0.5 / det;
			asq = a0 * a0 + a1 * a1;
			csq = c0 * c0 + c1 * c1;
			ctr0 = det * (asq * c1 - csq * a1);
			ctr1 = det * (csq * a0 - asq * c0);
			rad2 = ctr0 * ctr0 + ctr1 * ctr1;
			return { {ctr0 + b.x[0], ctr1 + b.x[1]}, sqrt(rad2));
		}
		*/

		template <typename FUNC>
		void for_each_vertex(FUNC&& func) const
		{
			func(a);
			func(b);
			func(c);
		}
		
		size_t vertex_count() const { return 3; }
		
		size_t edge_count() const { return 3; }
		
		auto edge(size_t index) const -> std::optional<std::pair<glm::tvec2<T>, glm::tvec2<T>>>
		{
			switch (index)
			{
			case 0: return std::make_pair(a, b);
			case 1: return std::make_pair(b, c);
			case 2: return std::make_pair(c, a);
			}
			return std::nullopt;
		}
		
		auto vertex(size_t index) const -> std::optional<tvec>
		{
			switch (index)
			{
			case 0: return a;
			case 1: return b;
			case 2: return c;
			}
			return std::nullopt;
		}
	};

	using triangle = ttriangle<float>;
	static_assert(area_shape<float, triangle>);
	static_assert(polygon_shape<float, triangle>);

	template <std::integral IDX = size_t>
	struct tindexed_triangle
	{
		std::array<IDX, 3> indices{};

		template <std::ranges::random_access_range T>
		auto a(T&& range) const -> std::ranges::range_value_t<T>
		{
			return range[indices[0]];
		}
		template <std::ranges::random_access_range T>
		auto b(T&& range) const -> std::ranges::range_value_t<T>
		{
			return range[indices[1]];
		}
		template <std::ranges::random_access_range T>
		auto c(T&& range) const -> std::ranges::range_value_t<T>
		{
			return range[indices[2]];
		}

		template <std::ranges::random_access_range T>
		auto as_triangle(T&& range) const -> ttriangle<typename std::ranges::range_value_t<T>::value_type> { return { at(range, indices[0]), at(range, indices[1]), at(range, indices[2]) }; }
	};

	using indexed_triangle = tindexed_triangle<size_t>;
}