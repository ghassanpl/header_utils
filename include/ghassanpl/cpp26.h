/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "min-cpp-version/cpp23.h"
#include <concepts>

namespace ghassanpl
{
	template <std::unsigned_integral T>
	T saturating_add(T x, T y)
	{
		T res = x + y;
		res |= -(res < x);
		return res;
	}

	template <std::unsigned_integral T>
	T saturating_sub(T x, T y)
	{
		T res = x - y;
		res &= -(res <= x);
		return res;
	}
}