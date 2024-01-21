#pragma once

#include "common.h"

namespace ghassanpl
{
	namespace win
	{
		extern "C"
		{
			typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
				COORD      dwSize;
				COORD      dwCursorPosition;
				WORD       wAttributes;
				SMALL_RECT srWindow;
				COORD      dwMaximumWindowSize;
			} CONSOLE_SCREEN_BUFFER_INFO;

			typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
				ULONG      cbSize;
				COORD      dwSize;
				COORD      dwCursorPosition;
				WORD       wAttributes;
				SMALL_RECT srWindow;
				COORD      dwMaximumWindowSize;
				WORD       wPopupAttributes;
				BOOL       bFullscreenSupported;
				COLORREF   ColorTable[16];
			} CONSOLE_SCREEN_BUFFER_INFOEX, * PCONSOLE_SCREEN_BUFFER_INFOEX;

			BOOL GHPL_WINAPI SetConsoleMode(HANDLE hConsoleHandle, DWORD  dwMode);
			BOOL GHPL_WINAPI GetConsoleMode(HANDLE  hConsoleHandle, DWORD* lpMode);
			HANDLE GHPL_WINAPI GetStdHandle(DWORD handle);
			BOOL GHPL_WINAPI SetConsoleTitleW(LPCWSTR lpConsoleTitle);

			BOOL GHPL_WINAPI SetConsoleScreenBufferSize(HANDLE hConsoleOutput, COORD  dwSize);

			BOOL GHPL_WINAPI GetConsoleScreenBufferInfoEx(HANDLE hConsoleOutput, PCONSOLE_SCREEN_BUFFER_INFOEX lpConsoleScreenBufferInfoEx);
			BOOL GHPL_WINAPI GetConsoleScreenBufferInfo(HANDLE hConsoleOutput, CONSOLE_SCREEN_BUFFER_INFO* lpConsoleScreenBufferInfo);
			BOOL GHPL_WINAPI SetConsoleScreenBufferInfoEx(HANDLE hConsoleOutput, CONSOLE_SCREEN_BUFFER_INFOEX const* lpConsoleScreenBufferInfo);
		}
	}
}
