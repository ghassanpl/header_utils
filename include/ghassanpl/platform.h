/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#if 0
namespace ghassanpl::platform
{
#include "platform_impl.inl"

	namespace compilation
	{
#ifdef WINDOWS
		static constexpr inline const char* directory_separator = "\\";
		static constexpr inline const char* nul_device_name = "nul";
#elif LINUX
		static constexpr inline const char* directory_separator = "/";
		static constexpr inline const char* nul_device_name = "/dev/null";
#elif 
#endif
	}
}
#endif