/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "random.h"
#include "rec2.h"
#include "geometry/ellipse.h"
#include "geometry/polygon.h"
#include "geometry/shape_concepts.h"

namespace ghassanpl::random
{
	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	T radians(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<T> dist{ T{}, glm::two_pi<T>() };
		return dist(rng);
	}

	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	T degrees(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_real_distribution<T> dist{ T{}, T{360} };
		return dist(rng);
	}

	template <std::floating_point T = float, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> unit_vector(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return glm::rotate(glm::tvec2<T>{ T{ 1 }, T{ 0 } }, radians<T>(rng));
	}

	template <typename T, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> point_in(trec2<T> const& rect, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return { range(rect.p1.x, rect.p2.x, rng), range(rect.p1.y, rect.p2.y, rng) };
	}

	template <typename T, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> point_in(glm::tvec2<T> const& max, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return { range(T{}, max.x, rng), range(T{}, max.y, rng) };
	}

	/// TODO: This doesn't seem uniform
	template <typename T, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> point_in(geometry::tellipse<T> const& el, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		const auto phi = range(T{}, glm::two_pi<T>(), rng);
		const auto p = glm::tvec2<T>{ cos(phi), sin(phi) } * glm::sqrt(percentage<T>(rng)) * el.radii * T(0.5);
		return p + el.center;
	}

	template <typename T, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> point_in(geometry::ttriangle<T> const& tr, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		const auto r1 = glm::sqrt(percentage<T>(rng));
		const auto r2 = percentage<T>(rng);

		return (tr.a * (T(1) - r1) + tr.b * (r1 * (T(1) - r2)) + tr.c * (r2 * r1));
	}

	template <typename T, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> point_in(geometry::immutable::tpolygon<T> const& poly, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		if (poly.triangles().empty()) return {};

		auto r = range(T{}, poly.calculate_area());
		size_t i = 0;
		while (r > 0.0)
		{
			if (!poly.has_triangle(i))
				return {};

			r -= poly.triangle_area(i);
			if (r <= 0.0)
				break;
			i++;
		}

		return point_in(poly.triangle(i), rng);
	}

	/// \brief Returns a random point on the edge of the shape.
	template <typename T, geometry::shape<T> S, typename RANDOM = std::default_random_engine>
	glm::tvec2<T> point_on(S const& shape, RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		return s.edge_point_alpha(percentage(rng));
	}

	/// TODO: in(circle), in(poly)?, on(rect), on(circle), on(arc)
	/*
	template <typename T>
	auto RandomNotIn(ghassanpl::trec2<T> const& verboten, ghassanpl::trec2<T> const& bounds)
	{
		/// Must be true: bounds.contains(verboten);
		const std::array<ghassanpl::trec2<T>, 4> surrounding = {
			ghassanpl::trec2<T>{ bounds.p1.x, bounds.p1.y, bounds.p2.x, verboten.p1.y },
			ghassanpl::trec2<T>{ bounds.p1.x, verboten.p2.y, bounds.p2.x, bounds.p2.y },
			ghassanpl::trec2<T>{ bounds.p1.x, bounds.p1.y, verboten.p1.x, bounds.p2.y },
			ghassanpl::trec2<T>{ verboten.p2.x, bounds.p1.y, bounds.p2.x, bounds.p2.y },
		};
		const std::array<T, 4> areas = {
			surrounding[0].calculate_area(),
			surrounding[1].calculate_area(),
			surrounding[2].calculate_area(),
			surrounding[3].calculate_area()
		};
		const int total_area = areas[0] + areas[1] + areas[2] + areas[3];
		const int rand_pos = RandomIn(total_area);
		if (rand_pos < areas[0]) return Random(surrounding[0]);
		if (rand_pos < areas[0] + areas[1]) return Random(surrounding[1]);
		if (rand_pos < areas[0] + areas[1] + areas[2]) return Random(surrounding[2]);
		return Random(surrounding[3]);
	}
	*/

	template <typename RANDOM = std::default_random_engine>
	glm::ivec2 neighbor(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM = std::default_random_engine>
	glm::ivec2 diagonal_neighbor(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 3 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 1.0f };
		case 1: return { -1.0f, 1.0f };
		case 2: return { -1.0f, -1.0f };
		case 3: return { 1.0f, -1.0f };
		}
		return {};
	}

	template <typename RANDOM = std::default_random_engine>
	glm::ivec2 surrounding(RANDOM& rng = ::ghassanpl::random::default_random_engine)
	{
		static std::uniform_int_distribution<typename RANDOM::result_type> dist{ 0, 7 };
		switch (dist(rng))
		{
		case 0: return { 1.0f, 0.0f };
		case 1: return { 0.0f, 1.0f };
		case 2: return { -1.0f, 0.0f };
		case 3: return { 0.0f, -1.0f };
		case 4: return { 1.0f, 1.0f };
		case 5: return { -1.0f, 1.0f };
		case 6: return { -1.0f, -1.0f };
		case 7: return { 1.0f, -1.0f };
		}
		return {};
	}

	/// TODO: Should we move the below to random_seq?
	
	/// \brief Returns the nth quasi-random vector in the [0, 1]^2 space.
	template <std::floating_point T = float>
	constexpr glm::tvec2<T> halton_sequence_2d(size_t index, size_t base_x = 2, size_t base_y = 3)
	{
		return { halton_sequence<T>(index, base_x), halton_sequence<T>(index, base_y) };
	}

}