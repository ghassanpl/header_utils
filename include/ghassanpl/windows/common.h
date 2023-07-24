/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include <cstdint>

namespace ghassanpl::win
{
#ifndef WINUSERAPI
	/// Base Windows Stuff
	extern "C"
	{
		typedef char CHAR;
		typedef unsigned char BYTE;
		typedef unsigned short WORD;
		typedef unsigned long DWORD;
		typedef unsigned int UINT;
		typedef signed char    INT8, * PINT8;
		typedef signed short   INT16, * PINT16;
		typedef signed int     INT32, * PINT32;
		typedef signed __int64 INT64, * PINT64;
		typedef unsigned char  UINT8, * PUINT8;
		typedef unsigned short UINT16, * PUINT16;
		typedef unsigned int   UINT32, * PUINT32;
		typedef unsigned __int64 UINT64, * PUINT64;
		typedef signed int LONG32, * PLONG32;
		typedef unsigned int ULONG32, * PULONG32;
		typedef unsigned int DWORD32, * PDWORD32;
		typedef short SHORT;
		typedef float FLOAT;
		typedef int BOOL;
		typedef BYTE BOOLEAN;
		typedef long LONG;
		typedef unsigned long ULONG;
		typedef const wchar_t* PCWSTR, * LPCWSTR;
		typedef const char* PCCH, * LPCCH;
		typedef const char* LPCSTR;
		/// typedef char TCHAR, * PTCHAR;
		/// typedef const TCHAR* LPCTSTR;
		typedef void VOID;
		typedef void* LPVOID;
		typedef wchar_t* LPWSTR;

		typedef intptr_t LONG_PTR;
		typedef intptr_t INT_PTR;
		typedef uintptr_t ULONG_PTR;
		typedef uintptr_t UINT_PTR;

		typedef ULONG_PTR SIZE_T, *PSIZE_T;
		typedef LONG_PTR SSIZE_T, *PSSIZE_T;
		typedef ULONG_PTR DWORD_PTR, * PDWORD_PTR;
		typedef __int64 LONG64, * PLONG64;
		typedef unsigned __int64 ULONG64, * PULONG64;
		typedef unsigned __int64 DWORD64, * PDWORD64;
		typedef long HRESULT;
		typedef LONG_PTR LPARAM;
		typedef UINT_PTR WPARAM;
		typedef void* HANDLE;
		using FARPROC = INT_PTR(__stdcall*)();
		using PROC = INT_PTR(__stdcall*)();

		#define GHPL_WIN_DECLARE_HANDLE(name) typedef struct name##_gh* name
		GHPL_WIN_DECLARE_HANDLE(HWND);
		GHPL_WIN_DECLARE_HANDLE(HINSTANCE);
		GHPL_WIN_DECLARE_HANDLE(HICON);

		typedef HINSTANCE HMODULE;

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
		typedef struct _POINTFLOAT {
			FLOAT x;
			FLOAT y;
		} POINTFLOAT, * LPPOINTFLOAT;
		typedef struct _RECT {
			LONG left;
			LONG top;
			LONG right;
			LONG bottom;
		} RECT, * LPRECT;
		typedef struct tagPOINT {
			LONG x;
			LONG y;
		} POINT, * PPOINT;

		typedef DWORD COLORREF;
		typedef DWORD* LPCOLORREF;

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
	}
#endif

	#define GHPL_WINAPI __declspec(dllimport)
	#define GHPL_WINGDIAPI __declspec(dllimport)
	#define GHPL_APIENTRY __stdcall
	#define GHPL_CALLBACK __stdcall

	enum
	{
		MEM_COMMIT_ = 0x1000,
		MEM_RESERVE_ = 0x2000,
		MEM_RESET_ = 0x80000,
		MEM_RESET_UNDO_ = 0x100000,
		MEM_TOP_DOWN_ = 0x100000,
		MEM_WRITE_WATCH_ = 0x200000,
		MEM_PHYSICAL_ = 0x400000,
		MEM_LARGE_PAGES_ = 0x20000000,

		PAGE_NOACCESS_ = 0x1,
		PAGE_READONLY_ = 0x2,
		PAGE_READWRITE_ = 0x4,
		PAGE_WRITECOPY_ = 0x8,
		PAGE_EXECUTE_ = 0x10,
		PAGE_EXECUTE_READ_ = 0x20,
		PAGE_EXECUTE_READWRITE_ = 0x40,
		PAGE_EXECUTE_WRITECOPY_ = 0x80,
		PAGE_GAURD_ = 0x100,
		PAGE_NOCACHE_ = 0x200,
		PAGE_WRITECOMBINE_ = 0x400,
		PAGE_TARGETS_INVALID_ = 0x40000000,
		PAGE_TARGETS_NO_UPDATE_ = 0x40000000,
		
		MEM_DECOMMIT_ = 0x4000,
		MEM_RELEASE_ = 0x8000,
		
		HEAP_NO_SERIALIZE_ = 0x1,
		HEAP_GENERATE_EXCEPTIONS_ = 0x4,
		HEAP_ZERO_MEMORY_ = 0x8,
		HEAP_REALLOC_IN_PLACE_ONLY_ = 0x10,
	};

	enum
	{
		COINIT_MULTITHREADED_ = 0x0,
		COINIT_APARTMENTTHREADED_ = 0x2,
		COINIT_DISABLE_OLE1DDE_ = 0x4,
		COINIT_SPEED_OVER_MEMORY_ = 0x8
	};

	static constexpr HRESULT S_OK_ = 0;
	static constexpr HRESULT S_FALSE_ = 1;
	static constexpr HRESULT E_NOTIMPL_ = (HRESULT)0x80004001L;
	static constexpr HRESULT E_NOINTERFACE_ = (HRESULT)0x80004002L;
	static constexpr HRESULT E_POINTER_ = (HRESULT)0x80004003L;
	static constexpr HRESULT E_ABORT_ = (HRESULT)0x80004004L;
	static constexpr HRESULT E_FAIL_ = (HRESULT)0x80004005L;
	static constexpr HRESULT E_UNEXPECTED_ = (HRESULT)0x8000FFFFL;
	static constexpr HRESULT E_ACCESSDENIED_ = (HRESULT)0x80070005L;
	static constexpr HRESULT E_HANDLE_ = (HRESULT)0x80070006L;
	static constexpr HRESULT E_OUTOFMEMORY_ = (HRESULT)0x8007000EL;
	static constexpr HRESULT E_INVALIDARG_ = (HRESULT)0x80070057L;

	static constexpr HRESULT RPC_E_CHANGED_MODE_ = (HRESULT)0x80010106L; /// Possible return value from CoInitializeEx

	extern "C"
	{
		GHPL_WINAPI HWND GHPL_APIENTRY GetActiveWindow(void);
		GHPL_WINAPI HMODULE GHPL_APIENTRY GetModuleHandleW(LPCWSTR lpModuleName);
		GHPL_WINAPI HMODULE GHPL_APIENTRY LoadLibraryW(LPCWSTR lpFileName);
		GHPL_WINAPI FARPROC GHPL_APIENTRY GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
		GHPL_WINAPI BOOL GHPL_APIENTRY FreeLibrary(HMODULE hModule);
		GHPL_WINAPI DWORD GHPL_APIENTRY GetLastError(void);
		GHPL_WINAPI void GHPL_APIENTRY DebugBreak(void);

		GHPL_WINAPI LPVOID GHPL_APIENTRY VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect);
		GHPL_WINAPI BOOL GHPL_APIENTRY VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD  dwFreeType);
		GHPL_WINAPI HANDLE GHPL_APIENTRY GetProcessHeap(void);
		GHPL_WINAPI LPVOID GHPL_APIENTRY HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
		GHPL_WINAPI BOOL GHPL_APIENTRY HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
		GHPL_WINAPI LPVOID GHPL_APIENTRY HeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem, SIZE_T dwBytes);
		GHPL_WINAPI VOID GHPL_APIENTRY ExitProcess(UINT uExitCode);

		GHPL_WINAPI HRESULT GHPL_APIENTRY CoInitializeEx(LPVOID pvReserved = nullptr, DWORD dwCoInit = COINIT_MULTITHREADED_);
		GHPL_WINAPI void GHPL_APIENTRY CoUninitialize();
	}
}
