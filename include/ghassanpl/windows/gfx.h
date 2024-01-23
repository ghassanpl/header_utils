/// \copyright This Source Code Form is subject to the terms of the Mozilla Public
/// License, v. 2.0. If a copy of the MPL was not distributed with this
/// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "common.h"

namespace ghassanpl
{
	namespace win
	{
		GHPL_WIN_DECLARE_HANDLE(HMENU);
		GHPL_WIN_DECLARE_HANDLE(HCURSOR);
		GHPL_WIN_DECLARE_HANDLE(HBRUSH);
		GHPL_WIN_DECLARE_HANDLE(HINSTANCE);
		GHPL_WIN_DECLARE_HANDLE(HGDIOBJ);
		GHPL_WIN_DECLARE_HANDLE(HGLRC);
		GHPL_WIN_DECLARE_HANDLE(HDC);

		enum PFDPixelType : BYTE
		{
			PFD_TYPE_RGBA_ = 0,
			PFD_TYPE_COLORINDEX_ = 1,
		};
		static_assert(sizeof(PFDPixelType) == sizeof(BYTE) && alignof(PFDPixelType) == alignof(BYTE), "PDFPixelType does not match BYTE");

		enum PFDLayerType : BYTE
		{
			PFD_MAIN_PLANE_ = 0,
			PFD_OVERLAY_PLANE_ = 1,
			PFD_UNDERLAY_PLANE_ = BYTE(-1),
		};
		static_assert(sizeof(PFD_UNDERLAY_PLANE_) == sizeof(BYTE) && alignof(PFDLayerType) == alignof(BYTE), "PDFLayerType does not match BYTE");

		enum PFDFlags : DWORD
		{
			PFD_DOUBLEBUFFER_ = 0x1,
			PFD_STEREO_ = 0x2,
			PFD_DRAW_TO_WINDOW_ = 0x4,
			PFD_DRAW_TO_BITMAP_ = 0x8,
			PFD_SUPPORT_GDI_ = 0x10,
			PFD_SUPPORT_OPENGL_ = 0x20,
			PFD_GENERIC_FORMAT_ = 0x40,
			PFD_NEED_PALETTE_ = 0x80,
			PFD_NEED_SYSTEM_PALETTE_ = 0x100,
			PFD_SWAP_EXCHANGE_ = 0x200,
			PFD_SWAP_COPY_ = 0x400,
			PFD_SWAP_LAYER_BUFFERS_ = 0x800,
			PFD_GENERIC_ACCELERATED_ = 0x1000,
			PFD_SUPPORT_DIRECTDRAW_ = 0x2000,
			PFD_DIRECT3D_ACCELERATED_ = 0x4000,
			PFD_SUPPORT_COMPOSITION_ = 0x8000,
			PFD_DEPTH_DONTCARE_ = 0x20000000,
			PFD_DOUBLEBUFFER_DONTCARE_ = 0x40000000,
			PFD_STEREO_DONTCARE_ = 0x80000000,
		};

		typedef struct tagPIXELFORMATDESCRIPTOR {
			WORD nSize;
			WORD nVersion;
			DWORD dwFlags;
			PFDPixelType iPixelType;
			BYTE cColorBits;
			BYTE cRedBits;
			BYTE cRedShift;
			BYTE cGreenBits;
			BYTE cGreenShift;
			BYTE cBlueBits;
			BYTE cBlueShift;
			BYTE cAlphaBits;
			BYTE cAlphaShift;
			BYTE cAccumBits;
			BYTE cAccumRedBits;
			BYTE cAccumGreenBits;
			BYTE cAccumBlueBits;
			BYTE cAccumAlphaBits;
			BYTE cDepthBits;
			BYTE cStencilBits;
			BYTE cAuxBuffers;
			PFDLayerType iLayerType;
			BYTE bReserved;
			DWORD dwLayerMask;
			DWORD dwVisibleMask;
			DWORD dwDamageMask;
		} PIXELFORMATDESCRIPTOR, * LPPIXELFORMATDESCRIPTOR;

		enum
		{
			WHITE_BRUSH_ = 0,
			LTGRAY_BRUSH_ = 1,
			GRAY_BRUSH_ = 2,
			DKGRAY_BRUSH_ = 3,
			BLACK_BRUSH_ = 4,
			NULL_BRUSH_ = 5,
			HOLLOW_BRUSH_ = NULL_BRUSH_,
			WHITE_PEN_ = 6,
			BLACK_PEN_ = 7,
			NULL_PEN_ = 8,
			OEM_FIXED_FONT_ = 10,
			ANSI_FIXED_FONT_ = 11,
			ANSI_VAR_FONT_ = 12,
			SYSTEM_FONT_ = 13,
			DEVICE_DEFAULT_FONT_ = 14,
			DEFAULT_PALETTE_ = 15,
			SYSTEM_FIXED_FONT_ = 16,
			DEFAULT_GUI_FONT_ = 17,
			DC_BRUSH_ = 18,
			DC_PEN_ = 19,
		};

		extern "C"
		{
			GHPL_WINAPI HGDIOBJ GHPL_APIENTRY GetStockObject(int fnObject);
			GHPL_WINAPI int GHPL_APIENTRY ChoosePixelFormat(HDC hdc, const PIXELFORMATDESCRIPTOR* ppfd);
			GHPL_WINAPI BOOL GHPL_APIENTRY SetPixelFormat(HDC hdc, int iPixelFormat, const PIXELFORMATDESCRIPTOR* ppfd);
			GHPL_WINAPI BOOL GHPL_APIENTRY SwapBuffers(HDC hdc);
			GHPL_WINAPI int GHPL_APIENTRY GetPixelFormat(HDC hdc);
			GHPL_WINAPI int GHPL_APIENTRY DescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);

			GHPL_WINAPI HGLRC GHPL_APIENTRY wglCreateContext(HDC hdc);
			GHPL_WINAPI HGLRC GHPL_APIENTRY wglGetCurrentContext();
			GHPL_WINAPI BOOL GHPL_APIENTRY wglDeleteContext(HGLRC context);
			GHPL_WINAPI HDC GHPL_APIENTRY wglGetCurrentDC();
			GHPL_WINAPI BOOL GHPL_APIENTRY wglMakeCurrent(HDC hdc, HGLRC hglrc);
			GHPL_WINAPI PROC GHPL_APIENTRY wglGetProcAddress(LPCSTR lpszProc);

			GHPL_WINAPI HDC GHPL_APIENTRY GetDC(HWND hWnd);
			GHPL_WINAPI int GHPL_APIENTRY ReleaseDC(HWND hWnd, HDC hDC);

			static constexpr intptr_t DPI_AWARENESS_CONTEXT_UNAWARE = (-1);
			static constexpr intptr_t DPI_AWARENESS_CONTEXT_SYSTEM_AWARE = (-2);
			static constexpr intptr_t DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE = (-3);
			static constexpr intptr_t DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = (-4);
			static constexpr intptr_t DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED = (-5);

			GHPL_WINAPI int GHPL_APIENTRY SetProcessDpiAwarenessContext(intptr_t value);
			GHPL_WINAPI UINT GHPL_APIENTRY GetDpiForSystem(VOID);
			GHPL_WINAPI UINT GHPL_APIENTRY GetDpiForWindow(HWND hwnd);
		}
	}
}
