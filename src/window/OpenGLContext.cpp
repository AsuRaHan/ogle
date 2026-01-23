#include "OpenGLContext.h"
#include <iostream>

OpenGLContext::OpenGLContext(HDC hdc) : m_hdc(hdc) {}

OpenGLContext::~OpenGLContext()
{
    Cleanup();
}

bool OpenGLContext::Initialize(int major, int minor, bool debug)
{
    if (m_initialized) return true;

    // Шаг 1: Создаём временный legacy контекст, чтобы загрузить расширения
    if (!CreateLegacyContext()) {
        return false;
    }

    // Загружаем нужные wgl-функции
    wglChoosePixelFormatARB = reinterpret_cast<PFNWGLCHOOSEPIXELFORMATARBPROC>(
        wglGetProcAddress("wglChoosePixelFormatARB")
    );
    wglCreateContextAttribsARB = reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(
        wglGetProcAddress("wglCreateContextAttribsARB")
    );

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        // std::cerr << "Failed to load WGL ARB functions" << std::endl;
        ogle::Logger::Error("Failed to load WGL ARB functions");
        Cleanup();
        return false;
    }

    // Удаляем временный контекст
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(m_hglrc);
    m_hglrc = nullptr;

    // Шаг 2: Создаём современный контекст
    if (!UpgradeToModernContext(major, minor, debug)) {
        return false;
    }

    // Шаг 3: Загружаем GLAD
    if (!gladLoaderLoadGL()) {
        // std::cerr << "ERROR gladLoaderLoadGL() failed" << std::endl;
        ogle::Logger::Error("gladLoaderLoadGL() failed");
        Cleanup();
        return false;
    }
    m_initialized = true;
    // keep simple cout here; higher-level logging used elsewhere
    // std::cout << "OpenGL initialized: " << GetVersionString() << std::endl;
    ogle::Logger::Info("OpenGL initialized: OK" + GetVersionString());

    
    return true;
}

bool OpenGLContext::CreateLegacyContext()
{
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(m_hdc, &pfd);
    if (!format) return false;

    if (!SetPixelFormat(m_hdc, format, &pfd)) return false;

    m_hglrc = wglCreateContext(m_hdc);
    if (!m_hglrc) return false;

    if (!wglMakeCurrent(m_hdc, m_hglrc)) {
        wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
        return false;
    }

    return true;
}

bool OpenGLContext::UpgradeToModernContext(int major, int minor, bool debug)
{
    const int pixelAttribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_DEPTH_BITS_ARB,     24,
        0
    };

    int pixelFormat;
    UINT numFormats;
    if (!wglChoosePixelFormatARB(m_hdc, pixelAttribs, nullptr, 1, &pixelFormat, &numFormats) || numFormats == 0) {
        // std::cerr << "wglChoosePixelFormatARB Failed" << std::endl;
        ogle::Logger::Error("wglChoosePixelFormatARB Failed");
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(m_hdc, pixelFormat, sizeof(pfd), &pfd);
    if (!SetPixelFormat(m_hdc, pixelFormat, &pfd)) {
        // std::cerr << "SetPixelFormat for modern format failed" << std::endl;
        ogle::Logger::Error("SetPixelFormat for modern format failed");
        return false;
    }

    int contextFlags = 0;
    if (debug) contextFlags |= 0x0001; // WGL_CONTEXT_DEBUG_BIT_ARB, если поддерживается

    const int contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_FLAGS_ARB,         contextFlags,
        WGL_CONTEXT_PROFILE_MASK_ARB,  0x00000001, // WGL_CONTEXT_CORE_PROFILE_BIT_ARB
        0
    };

    m_hglrc = wglCreateContextAttribsARB(m_hdc, nullptr, contextAttribs);
    if (!m_hglrc) {
        // std::cerr << "wglCreateContextAttribsARB failed" << std::endl;
        ogle::Logger::Error("wglCreateContextAttribsARB failed");
        return false;
    }

    if (!wglMakeCurrent(m_hdc, m_hglrc)) {
        // std::cerr << "wglMakeCurrent failed for new contecst" << std::endl;
        ogle::Logger::Error("wglMakeCurrent failed for new contecst");
        wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
        return false;
    }

    return true;
}

std::string OpenGLContext::GetVersionString() const
{
    if (!m_initialized) return "Not Init";
    const char* ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    return ver ? ver : "neizvesno";
}

void OpenGLContext::MakeCurrent() const
{
    if (m_hglrc) wglMakeCurrent(m_hdc, m_hglrc);
}

void OpenGLContext::SwapBuffers() const
{
    if (m_hdc) ::SwapBuffers(m_hdc);
}

void OpenGLContext::Cleanup()
{
    if (m_hglrc) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(m_hglrc);
        m_hglrc = nullptr;
    }
    m_initialized = false;
}