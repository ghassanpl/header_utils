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