#pragma once

#define GHPL_PLATFORM_VERSION_WINXPNOSP 0x05010000
#define GHPL_PLATFORM_VERSION_WINXPSP1 0x05010100
#define GHPL_PLATFORM_VERSION_WINXPSP2 0x05010200
#define GHPL_PLATFORM_VERSION_WINXPSP3 0x05010300
#define GHPL_PLATFORM_VERSION_WINVISTA 0x06000000
#define GHPL_PLATFORM_VERSION_WIN7 0x06010000
#define GHPL_PLATFORM_VERSION_WIN8 0x06020000
#define GHPL_PLATFORM_VERSION_WIN81 0x06030000
#define GHPL_PLATFORM_VERSION_WIN10 0x0A000000
#define GHPL_PLATFORM_VERSION_WIN10_TH2 0x0A000001
#define GHPL_PLATFORM_VERSION_WIN10_RS1 0x0A000002
#define GHPL_PLATFORM_VERSION_WIN10_RS2 0x0A000003
#define GHPL_PLATFORM_VERSION_WIN10_RS3 0x0A000004
#define GHPL_PLATFORM_VERSION_WIN10_RS4 0x0A000005
#define GHPL_PLATFORM_VERSION_WIN10_RS5 0x0A000006
#define GHPL_PLATFORM_VERSION_WIN10_19H1 0x0A000007
#define GHPL_PLATFORM_VERSION_WIN10_VB 0x0A000008

#ifndef GHPL_DEF_MINIMUM_PLATFORM_VERSION
#define GHPL_DEF_MINIMUM_PLATFORM_VERSION GHPL_PLATFORM_VERSION_WIN7
#else
#if GHPL_DEF_MINIMUM_PLATFORM_VERSION > WDK_NTDDI_VERSION
#warning "The requested windows platform version is higher than the one in windows headers"
#endif
#endif

#ifndef _WIN32_WINNT
#ifdef GHPL_DEF_MINIMUM_PLATFORM_VERSION
#define _WIN32_WINNT (GHPL_DEF_MINIMUM_PLATFORM_VERSION>>16)
#else
#define _WIN32_WINNT 0x0601
#endif
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef WINVER
#define WINVER _WIN32_WINNT
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION GHPL_DEF_MINIMUM_PLATFORM_VERSION
#endif

#define NOATOM
#define NOMINMAX
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOPROFILER
#define NOMCX

/// TODO: Should we do #define byte _win32_byte here? `using namespace std` will put std::byte into the global namespace, 
///		causing a conflict with the `byte` declared in windows.h

#include <Windows.h>

/*
/// Controls
#ifdef _UNICODE
	#if defined _M_IX86
		#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#elif defined _M_IA64
		#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#elif defined _M_X64
		#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
	#endif
#else
	#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#ifdef GHLIB_COMPILER_VS
	#pragma comment(lib, "Comctl32.lib")
#endif
#include <Commctrl.h>

/// Debug
#if GHLIB_DEF_MINIMUM_PLATFORM_VERSION >= GHLIB_PLATFORM_VERSION_WIN81
	#include <Processsnapshot.h> ///< Win 8.1 or above only!
#endif
#include <Dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

/// Dialog
#include <commdlg.h>

/// Multimedia
#include <mmsystem.h>
#ifdef GHLIB_COMPILER_VS
	#pragma comment(lib, "Winmm.lib")
#endif

/// Processes
#ifdef GHLIB_COMPILER_VS
	#pragma comment(lib, "psapi.lib")
#endif
#include <Psapi.h>

/// Shell
struct IUnknown; /// For clang
#include <Shlobj.h>
#include <shellapi.h>

/// User
#if GHLIB_PLATFORM_POSIX
#include <pwd.h>
#endif
*/

#undef far
#undef near
#undef NEAR
#undef FAR
#undef CALLBACK
#undef LoadString
#undef Yield
#undef DrawText
#undef LoadFont
#undef RemoveDirectory
#undef min
#undef max
#undef GetClassName