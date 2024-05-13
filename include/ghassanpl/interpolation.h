/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/spline.hpp>
#undef GLM_ENABLE_EXPERIMENTAL

/// TODO: 
///	- lerp, unlerp, etc
///	- splines
///	- easing functions 
/// - approach

namespace ghassanpl
{
	namespace splines
	{
		template <typename P, typename T>
		inline auto bezier(P const& a, P const& b, P const& c, T t) { return glm::mix(glm::mix(a, b, t), glm::mix(b, c, t), t); }

		template <typename P, typename T>
		inline auto bezier(P const& a, P const& b, P const& c, P const& d, T t) { return bezier(glm::mix(a, b, t), glm::mix(b, c, t), glm::mix(c, d, t), t); }

		using glm::catmullRom;
		using glm::cubic;
		using glm::hermite;
	}
}
