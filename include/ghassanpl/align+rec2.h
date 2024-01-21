/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include "rec2.h"
#include "align.h"

namespace ghassanpl
{
	template <typename T>
	[[nodiscard]] constexpr trec2<T> aligned(trec2<T> const& smaller, trec2<T> const& larger, align alignment)
	{
		return trec2<T>::from_size(
			larger.position() + glm::tvec2<T>{ 
				ghassanpl::aligned<T>(smaller.width(), larger.width(), horizontal_from(alignment)), 
				ghassanpl::aligned<T>(smaller.height(), larger.height(), vertical_from(alignment)) 
			}, 
			smaller.size()
		);
	}

	template <typename T>
	[[nodiscard]] constexpr trec2<T> aligned(glm::tvec2<T> inner_size, trec2<T> const& larger, align alignment)
	{
		return trec2<T>::from_size(
			larger.position() + glm::tvec2<T>{ 
				ghassanpl::aligned<T>(inner_size.x, larger.width(), horizontal_from(alignment)),
				ghassanpl::aligned<T>(inner_size.y, larger.height(), vertical_from(alignment))
			},
			inner_size
		);
	}
}