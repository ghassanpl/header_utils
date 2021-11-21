#pragma once

#include "enum_flags.h"
#include <atomic>

namespace ghassanpl
{
	template <detail::integral_or_enum ENUM, detail::valid_integral VALUE_TYPE = unsigned long long>
	struct atomic_enum_flags : enum_flags<ENUM, std::atomic<VALUE_TYPE>>
	{
		using base_type = enum_flags<ENUM, std::atomic<VALUE_TYPE>>;
		using base_type::base_type;
	};
}