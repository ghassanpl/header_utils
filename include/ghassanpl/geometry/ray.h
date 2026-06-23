/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{

	/// \ingroup Geometry
	/// Represents a 2D ray
	template <std::floating_point T>
	struct tray
	{
		/// \internal TODO: Could this technically meet `shape`? Sure, the edge length and bbox are infinite, but that's technically still defined; but I don't think edge_point_alpha is...

		using tvec = glm::tvec2<T>;
		using value_type = T;

		tvec start{};
		tvec dir{}; ///< \internal TODO: this should not be public, as it doesn't enforce the length to be 1

		basic_line_t<T> line() const noexcept { return line_from_dir(dir); }

		static tray from_dir(tvec const& start, tvec const& dir) noexcept { return { start, glm::normalize(dir) }; }
		static tray from_points(tvec const& start, tvec const& second) noexcept { return from_dir(start, second - start); }
		
		tray& set_position(tvec const& pos) noexcept { start = pos; return *this; }
		tray& operator+=(tvec const& offs) noexcept { start += offs; return *this; }
		tray& operator-=(tvec const& offs) noexcept { start -= offs; return *this; }
		tray& translate(tvec const& offs) noexcept { return this->operator+=(offs); }

		T edge_length() const noexcept { return std::numeric_limits<T>::infinity(); }

		tvec edge_point(T t) const { return start + dir * t; }
		trec2<T> bounding_box() const { return trec2<T>::from_points(edge_point(T{}), edge_point(edge_length())); }
		tvec closest_point_to(tvec const& pt) const
		{
			const auto a = projected_alpha(pt);
			if (a < T(0)) return start;
			return start + dir * a;
		}
		T projected_alpha(tvec const& pt) const
		{
			const auto d = pt - start;
			return glm::dot(d, dir);
		}
	};

	using ray = tray<float>;
}

#define GHPL_GEOMETRY_RAY 1
#ifdef GHPL_GEOMETRY_SEGMENT
#include "ray+segment.h"
#endif