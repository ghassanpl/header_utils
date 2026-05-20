/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "ray.h"
#include "segment.h"

namespace ghassanpl::geometry
{
	template <typename T>
	tsegment<T> to_segment(tray<T> const& r, T end_len) noexcept { return tsegment<T>::from_dir(r.start, r.dir, end_len); }

	template <typename T>
	tray<T> to_ray(tsegment<T> const& r) noexcept { return tray<T>::from_points(r.start, r.end); }

	//template <typename T>
	//tsegment3d<T> to_segment(tray3d<T> const& r, T end_len) noexcept { return tsegment<T>{ r.start, r.start + r.dir * end_len }; }

}