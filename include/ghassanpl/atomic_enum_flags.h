/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "enum_flags.h"
#include <atomic>

namespace ghassanpl
{
	template <integral_or_enum ENUM, detail::valid_integral VALUE_TYPE = unsigned long long>
	struct atomic_enum_flags : enum_flags<ENUM, std::atomic<VALUE_TYPE>>
	{
		using base_type = enum_flags<ENUM, std::atomic<VALUE_TYPE>>;
		using base_type::base_type;
	};
}