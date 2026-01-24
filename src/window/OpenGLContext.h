#pragma once

#include <windows.h>
#include <string>
#include <glad/gl.h>   // от GLAD 2

#include "log/Logger.h"
// В начале OpenGLContext.cpp, после #include <windows.h>
// или лучше в отдельном заголовке, например src/gl/wglext_defs.h и include его

// WGL_ARB_pixel_format
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_STENCIL_BITS_ARB              0x2023   // опционально, если используешь

// WGL_ARB_create_context
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                 0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB    0x0002
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB          0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

namespace ogle {

	class OpenGLContext
	{
	public:
		explicit OpenGLContext(HDC hdc);
		~OpenGLContext();

		bool Initialize(int major = 4, int minor = 6, bool debug = false);

		std::string GetVersionString() const;

		void MakeCurrent() const;
		void SwapBuffers() const;
		void Cleanup();

	private:
		HDC     m_hdc{ nullptr };
		HGLRC   m_hglrc{ nullptr };
		bool    m_initialized{ false };

		bool CreateLegacyContext();
		bool UpgradeToModernContext(int major, int minor, bool debug);

		// Типы функций WGL (загружаем вручную)
		using PFNWGLCHOOSEPIXELFORMATARBPROC = BOOL(WINAPI*)(HDC, const int*, const FLOAT*, UINT, int*, UINT*);
		using PFNWGLCREATECONTEXTATTRIBSARBPROC = HGLRC(WINAPI*)(HDC, HGLRC, const int*);

		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB{ nullptr };
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB{ nullptr };
	};
} // namespace ogle