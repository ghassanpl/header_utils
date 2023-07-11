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
		void for_each_edge(FUNC&& func)
		{
			func(a, b);
			func(b, c);
			func(c, a);
		}

		template <typename FUNC>
		void for_each_segment(FUNC&& func)
		{
			func(tsegment<T>{a, b});
			func(tsegment<T>{b, c});
			func(tsegment<T>{c, a});
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

		glm::tvec2<T> edge_point_alpha(T t) const;
		glm::tvec2<T> edge_point(T t) const;
		glm::tvec2<T> projected(glm::tvec2<T> pt) const;

		float sign(tvec const& p1, tvec const& p2, tvec const& p3)
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}

		bool contains(tvec pt) const 
		{
			const auto d1 = sign(pt, a, b);
			const auto d2 = sign(pt, b, c);
			const auto d3 = sign(pt, c, a);
			const auto has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			const auto has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
			return !(has_neg && has_pos);
		}

		T calculate_area() const
		{
			const auto A = glm::sqrt((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
			const auto B = glm::sqrt((b.x - c.x) * (b.x - c.x) + (b.y - c.y) * (b.y - c.y));
			const auto C = glm::sqrt((b.x - c.x) * (a.x - c.x) + (a.y - c.y) * (a.y - c.y));
			const auto s = (A + B + C) * T(0.5);
			return glm::sqrt(s * (s - A) * (s - B) * (s - C));
		}
	};

	using triangle = ttriangle<float>;
	static_assert(area_shape<triangle, float>);

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