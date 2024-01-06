/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"

namespace ghassanpl::geometry
{
	template <std::ranges::range RANGE, typename T = std::ranges::range_value_t<RANGE>>
	constexpr bool are_colinear(RANGE const& range, T tolerance = precision_limits<T>::point_on_line_max_distance)
	{
		if (std::ranges::size(range) < 3)
			return true;

		auto it = std::ranges::begin(range);

		const auto line = line_crossing_points(*it++, *it++);
		const auto end = std::ranges::end(range);
		while (it != end)
		{
			if (line.distance(*it++) > tolerance)
				return false;
		}

		return true;
	}
}