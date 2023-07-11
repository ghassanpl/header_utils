#pragma once

#include "common.h"

#include <string>

namespace ghassanpl
{


	struct last_windows_error_t
	{
		uint32_t value{};
		std::string formatted;
	};

	namespace detail
	{
		extern "C" __declspec(dllimport) win::DWORD __stdcall FormatMessageA(win::DWORD dwFlags, const void* lpSource, win::DWORD dwMessageId, win::DWORD dwLanguageId, char* lpBuffer, win::DWORD nSize, va_list * Arguments);
		extern "C" __declspec(dllimport) win::HANDLE __stdcall LocalFree(win::HANDLE hMem);
	}

	last_windows_error_t get_last_windows_error()
	{
		const auto le = win::GetLastError(); /// must get as soon as possible; maybe even should be moved into a defaulted argument, forcing it to be called at the callsite

		last_windows_error_t result{};
		result.value = le;
		if (le != 0)
		{
			char* messageBuffer = nullptr;
			size_t size = detail::FormatMessageA(0x00000100 | 0x00001000 | 0x00000200 | 0xFF, nullptr, le, 0, (char*)&messageBuffer, 0, nullptr);
			result.formatted = std::string{ messageBuffer, size };
			detail::LocalFree(messageBuffer);
		}
		return result;
	}
}
