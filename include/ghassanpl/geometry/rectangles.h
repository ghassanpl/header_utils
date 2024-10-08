/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "geometry_common.h"
#include "../rec2.h"

namespace ghassanpl::geometry
{
	static_assert(polygon_shape<float, rec2>);
}