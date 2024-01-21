/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{

	template <std::floating_point T>
	struct tray
	{
		using tvec = glm::tvec2<T>;
		using value_type = T;
		using segment = tsegment<T>;

		tvec start{};
		tvec dir{}; /// TODO: this should not be public, as it doesn't enforce the length to be 1

		basic_line_t<T> line() const noexcept;

		tray from_dir(tvec const& start, tvec const& dir) noexcept { return { start, glm::normalize(dir) }; }
		tray from_points(tvec const& start, tvec const& second) noexcept { return from_dir(start, second - start); }
		
		segment& set_position(tvec const& pos) noexcept { start = pos; return *this; }
		segment& operator+=(tvec const& offs) noexcept { start += offs; return *this; }
		segment& operator-=(tvec const& offs) noexcept { start -= offs; return *this; }
		segment& translate(tvec const& offs) noexcept { return this->operator+=(offs); }

		tvec edge_point_alpha(T t) const { return start + dir * t; }
		tvec projected(tvec const& pt) const
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