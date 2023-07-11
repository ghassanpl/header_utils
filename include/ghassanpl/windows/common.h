#pragma once

namespace ghassanpl::win
{
#ifndef WINUSERAPI
	/// Base Windows Stuff
	extern "C"
	{
		typedef unsigned char BYTE;
		typedef unsigned short WORD;
		typedef unsigned long DWORD;
		typedef unsigned int UINT;
		typedef short SHORT;
		typedef float FLOAT;
		typedef int BOOL;
		typedef BYTE BOOLEAN;
		typedef long LONG;
		typedef unsigned long ULONG;
		typedef DWORD COLORREF;
		typedef DWORD* LPCOLORREF;
		typedef struct HWND_gh* HWND;
		typedef struct HINSTANCE_gh* HINSTANCE;
		typedef HINSTANCE HMODULE;
		typedef struct HICON_gh* HICON;
		typedef const wchar_t* PCWSTR, * LPCWSTR;
		typedef const char* PCCH, * LPCCH;
		typedef const char* LPCSTR;
		typedef const wchar_t* LPCTSTR;
		typedef wchar_t* LPWSTR;
		typedef __int64 LONG_PTR;
		typedef __int64 INT_PTR;
		typedef unsigned __int64 ULONG_PTR;
		typedef unsigned __int64 UINT_PTR;
		typedef long HRESULT;
		typedef LONG_PTR LPARAM;
		typedef UINT_PTR WPARAM;
		typedef void* HANDLE;
		using FARPROC = INT_PTR(__stdcall*)();

		static constexpr auto GHASSANPL_S_OK = ((HRESULT)0L);
		static constexpr auto GHASSANPL_S_FALSE = ((HRESULT)1L);

		typedef struct _COORD {
			SHORT X;
			SHORT Y;
		} COORD, * PCOORD;
		typedef struct _SMALL_RECT {
			SHORT Left;
			SHORT Top;
			SHORT Right;
			SHORT Bottom;
		} SMALL_RECT;

		__declspec(dllimport) HWND __stdcall GetActiveWindow(void);
		__declspec(dllimport) HMODULE __stdcall GetModuleHandleW(LPCWSTR lpModuleName);
		__declspec(dllimport) HMODULE __stdcall LoadLibraryW(LPCWSTR lpFileName);
		__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
		__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE hModule);
		__declspec(dllimport) DWORD __stdcall GetLastError(void);
		__declspec(dllimport) void __stdcall DebugBreak(void);

		#define GHASSANPL_WINAPI __declspec(dllimport)
		#define GHASSANPL_WINGDIAPI __declspec(dllimport)
		#define GHASSANPL_APIENTRY __stdcall
	}
#else
	/// Base Windows Stuff
	extern "C"
	{
		using BYTE = ::BYTE;
		using WORD = ::WORD;
		using DWORD = ::DWORD;
		using UINT = ::UINT;
		using FLOAT = ::FLOAT;
		using BOOL = ::BOOL;
		using BOOLEAN = ::BOOLEAN;
		using LONG = ::LONG;
		using HWND = ::HWND;
		using HINSTANCE = ::HINSTANCE;
		using HMODULE = ::HMODULE;
		using HICON = ::HICON;
		using LPCWSTR = ::LPCWSTR;
		using LPCSTR = ::LPCSTR;
		using LPCCH = ::LPCCH;
		using LPWSTR = ::LPWSTR;
		using LONG_PTR = ::LONG_PTR;
		using ULONG_PTR = ::ULONG_PTR;
		using UINT_PTR = ::UINT_PTR;
		using HRESULT = ::HRESULT;
		using LPARAM = ::LPARAM;
		using WPARAM = ::WPARAM;
		using FARPROC = INT_PTR(__stdcall*)();
		using HANDLE = ::HANDLE;

		static constexpr auto GHASSANPL_S_OK = S_OK;
		static constexpr auto GHASSANPL_S_FALSE = S_FALSE;

		__declspec(dllimport) HWND __stdcall GetActiveWindow(void);
		__declspec(dllimport) HMODULE __stdcall GetModuleHandleW(LPCWSTR lpModuleName);
		__declspec(dllimport) HMODULE __stdcall LoadLibraryW(LPCWSTR lpFileName);
		__declspec(dllimport) BOOL __stdcall FreeLibrary(HMODULE hModule);
		__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
		__declspec(dllimport) DWORD __stdcall GetLastError(void);
		__declspec(dllimport) void __stdcall DebugBreak(void);

#define GHASSANPL_WINGDIAPI __declspec(dllimport)
#define GHASSANPL_APIENTRY __stdcall
	}
#endif
}
