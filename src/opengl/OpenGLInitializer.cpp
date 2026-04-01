#include "OpenGLInitializer.h"

OpenGLInitializer::OpenGLInitializer(HWND hWnd, HDC hDc) : hRC(NULL), hDC(hDc),
wglChoosePixelFormatARB(nullptr), wglCreateContextAttribsARB(nullptr)
{
    m_hWnd = hWnd;
}

OpenGLInitializer::~OpenGLInitializer() {
    if (hRC) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRC);
    }
    if (hDC) {
        ReleaseDC(NULL, hDC);
    }
}

bool OpenGLInitializer::Initialize(bool fullScreen, int width, int height) {
    LOG_INFO("Начинаю инициализацию OpenGL");
    // Настройка полноэкранного режима, если требуется
    if (fullScreen) {
        // Удаляем стиль окна с заголовком и рамкой
        LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_STYLE);
        style &= ~WS_CAPTION; // Убирает заголовок
        style &= ~WS_THICKFRAME; // Убирает рамку (для изменяемых окон)
        style &= ~WS_BORDER; // Убирает простую границу для неизменяемых окон
        style &= ~WS_SIZEBOX; // Убирает размерную рамку

        SetWindowLongPtr(m_hWnd, GWL_STYLE, style);

        // Изменяем расширенный стиль
        LONG_PTR exStyle = GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
        exStyle &= ~WS_EX_WINDOWEDGE;
        exStyle &= ~WS_EX_CLIENTEDGE;
        exStyle &= ~WS_EX_STATICEDGE;

        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, exStyle);


        // Перерисовать окно
        DEVMODE dmScreenSettings;
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        //dmScreenSettings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
        //dmScreenSettings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
        
        // Если width или height равны 0, получаем текущее разрешение экрана
        if (width == 0 || height == 0) {
            EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmScreenSettings);
            if (width == 0) {
                width = dmScreenSettings.dmPelsWidth; // Устанавливаем ширину
            }
            if (height == 0) {
                height = dmScreenSettings.dmPelsHeight; // Устанавливаем высоту
            }
        }

        dmScreenSettings.dmPelsWidth = width; // Устанавливаем ширину
        dmScreenSettings.dmPelsHeight = height; // Устанавливаем высоту

        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        //SetWindowPos(m_hWnd, HWND_TOP, 0, 0, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight, SWP_SHOWWINDOW);
        //ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);

        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
            LOG_WARN("Не удалось переключиться в полноэкранный режим");
            return false;
        }

        SetWindowPos(m_hWnd, HWND_TOP, 0, 0, dmScreenSettings.dmPelsWidth, dmScreenSettings.dmPelsHeight, SWP_SHOWWINDOW);
        ShowWindow(m_hWnd, SW_SHOWMAXIMIZED);
    }

    // Регистрация класса окна
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, DefWindowProcW, 0, 0, GetModuleHandle(nullptr), nullptr, nullptr, (HBRUSH)(COLOR_WINDOW + 1), nullptr, L"OpenGLTempClass", nullptr };
    if (!RegisterClassExW(&wc)) {
        LOG_ERROR("Не удалось зарегистрировать класс временного окна: " + GetErrorMessage(GetLastError()));
        return false;
    }

    // Создание временного окна
    HWND temporaryWindow = CreateWindowExW(WS_EX_APPWINDOW, L"OpenGLTempClass", L"Temp", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(nullptr), NULL);
    if (temporaryWindow == nullptr) {
        LOG_ERROR("Не удалось создать временное окно: " + GetErrorMessage(GetLastError()));
        return false;
    }

    HDC temporaryDC = GetDC(temporaryWindow);
    if (temporaryDC == nullptr) {
        LOG_ERROR("Не удалось получить HDC для временного окна: " + GetErrorMessage(GetLastError()));
        DestroyWindow(temporaryWindow);
        return false;
    }

    if (!SetupPixelFormat(temporaryDC)) {
        LOG_ERROR("Не удалось установить формат пикселей для временного окна");
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    HGLRC temporaryRC = wglCreateContext(temporaryDC);
    if (temporaryRC == nullptr) {
        LOG_ERROR("Не удалось создать временный контекст OpenGL: " + GetErrorMessage(GetLastError()));
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    wglMakeCurrent(temporaryDC, temporaryRC);

    // Загрузка расширений OpenGL
    wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
    wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB) {
        LOG_ERROR("Не удалось загрузить OpenGL расширения");
        wglDeleteContext(temporaryRC);
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    // Выбор формата для основного окна
    const int formatAttributes[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        0
    };
    int major, minor;
    const GLubyte* version = glGetString(GL_VERSION);
    LOG_INFO("  OpenGL Version: " + std::string(reinterpret_cast<const char*>(version)));
    // Извлекаем основные и минорные версии OpenGL
    std::istringstream versionStream(reinterpret_cast<const char*>(version));
    versionStream >> major; // Основная версия
    versionStream.ignore(1); // Игнорируем точку
    versionStream >> minor; // Минорная версия

    if (major < 4 || (major == 4 && minor < 3)) {
        int result = MessageBoxW(NULL,
            L"Ваша версия OpenGL ниже 4.3. Обновите драйверы видеокарты или обратитесь к разработчику.\nХотите посетить сайт разработчика для поддержки?",
            L"Ошибка версии OpenGL",
            MB_YESNO | MB_ICONERROR);
        if (result == IDYES) {
            ShellExecuteW(NULL, L"open", L"https://github.com/AsuRaHan/GameOfLife3D", NULL, NULL, SW_SHOWNORMAL);
        }
        wglDeleteContext(temporaryRC);
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        exit(1);
    }

    const int contextAttributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, major,
        WGL_CONTEXT_MINOR_VERSION_ARB, minor,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    //====================================
    if (glGetString(GL_SHADING_LANGUAGE_VERSION) == nullptr) {
        LOG_WARN("Не удалось получить версию GLSL");
    }
    else {
        LOG_INFO("  GLSL Version: " + std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))));
    }
    // Проверка поддержки теселяции
    if (glGetString(GL_EXTENSIONS) != nullptr) {
        std::string extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
        if (extensions.find("GL_ARB_tessellation_shader") != std::string::npos) {
            LOG_INFO("  Теселяция поддерживается");
        }
        else {
            LOG_WARN("  Теселяция не поддерживается");
        }
    }
    //====================================
    int format, formatCount;
    wglChoosePixelFormatARB(hDC, formatAttributes, NULL, 1, &format, (UINT*)&formatCount);
    PIXELFORMATDESCRIPTOR pfd;
    DescribePixelFormat(hDC, format, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    SetPixelFormat(hDC, format, &pfd);

    // Создание современного контекста OpenGL
    hRC = wglCreateContextAttribsARB(hDC, NULL, contextAttributes);
    if (!hRC) {
        LOG_ERROR("Не удалось создать контекст OpenGL");
        wglDeleteContext(temporaryRC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    if (!wglMakeCurrent(hDC, hRC)) {
        LOG_ERROR("Не удалось сделать текущим OpenGL-контекст для основного окна: " + GetErrorMessage(GetLastError()));
        wglDeleteContext(temporaryRC);
        ReleaseDC(temporaryWindow, temporaryDC);
        DestroyWindow(temporaryWindow);
        return false;
    }

    wglDeleteContext(temporaryRC);
    ReleaseDC(temporaryWindow, temporaryDC);
    DestroyWindow(temporaryWindow);

    const GLubyte* mainVersion = glGetString(GL_VERSION);
    const GLubyte* mainGLSL = glGetString(GL_SHADING_LANGUAGE_VERSION);
    if (mainVersion) {
        LOG_INFO("  Основной OpenGL Version: " + std::string(reinterpret_cast<const char*>(mainVersion)));
    } else {
        LOG_ERROR("Не удалось получить GL_VERSION для основного контекста");
    }
    if (mainGLSL) {
        LOG_INFO("  Основной GLSL Version: " + std::string(reinterpret_cast<const char*>(mainGLSL)));
    } else {
        LOG_WARN("Не удалось получить GLSL_VERSION для основного контекста");
    }
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        LOG_ERROR("OpenGL error после установки контекста: " + std::to_string(err));
    }

    LOG_INFO("  wglCreateContext успешно");
    LOG_INFO("  wglMakeCurrent успешно");

    LOG_INFO("Начинаю ручную загрузку OpenGL функций");
    LoadOpenGLFunctions();

#ifdef _DEBUG
    OpenGLDebug debugger;
    if (!debugger.Initialize()) {
        LOG_WARN("Не удалось инициализировать отладку OpenGL");
        // Не завершаем программу, так как отладка опциональна
    }
#endif


    return true;
}

bool OpenGLInitializer::SetupPixelFormat(HDC hdc) {
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int pixelFormat = ChoosePixelFormat(hdc, &pfd);
    if (pixelFormat == 0) {
        LOG_ERROR("Не удалось выбрать pixel format: " + GetErrorMessage(GetLastError()));
        return false;
    }

    if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
        LOG_ERROR("Не удалось установить pixel format: " + GetErrorMessage(GetLastError()));
        return false;
    }

    return true;
}

bool OpenGLInitializer::checkForDepthIssues() {
    // Получаем размер окна
    int width = 800; // Замените на фактическую ширину вашего окна
    int height = 600; // Замените на фактическую высоту вашего окна

    // Создаем буфер для хранения значений глубины
    GLfloat* depthBuffer = new GLfloat[width * height];

    // Читаем значения глубины из буфера
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthBuffer);

    // Пример простой проверки: если значение глубины в центре экрана слишком близко к 1.0 (что соответствует farPlane)
    // Это может указывать на проблемы с глубиной
    GLfloat centerDepth = depthBuffer[(height / 2) * width + (width / 2)];

    delete[] depthBuffer;

    // Если значение глубины слишком близко к farPlane, это может быть проблемой
    if (centerDepth > 0.9f) {
        return true; // Проблема с глубиной
    }

    return false; // Проблем нет
}

void OpenGLInitializer::printSystemInfo() {
    // Получаем информацию о видеокарте
    const GLubyte* renderer = glGetString(GL_RENDERER); // Название видеокарты
    const GLubyte* vendor = glGetString(GL_VENDOR);     // Производитель видеокарты
    const GLubyte* version = glGetString(GL_VERSION);   // Версия OpenGL
    const GLubyte* shadingLanguageVersion = glGetString(GL_SHADING_LANGUAGE_VERSION); // Версия шейдерного языка

    // Выводим информацию
    LOG_INFO(std::string("Видеокарта:               ") + reinterpret_cast<const char*>(renderer));
    LOG_INFO(std::string("Производитель:            ") + reinterpret_cast<const char*>(vendor));
    LOG_INFO(std::string("Версия OpenGL:            ") + reinterpret_cast<const char*>(version));
    LOG_INFO(std::string("Версия шейдерного языка:  ") + reinterpret_cast<const char*>(shadingLanguageVersion));

    // Получаем информацию о поддерживаемых расширениями
    GLint numExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    LOG_INFO("Поддерживаемые расширения:");
    for (GLint i = 0; i < numExtensions; ++i) {
        LOG_INFO(std::string("      ") + reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i)));
    }
}

std::string OpenGLInitializer::GetErrorMessage(DWORD errorCode) {
    LPVOID lpMsgBuf;
    DWORD bufLen;

    // Получаем сообщение об ошибке
    bufLen = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Используем язык по умолчанию
        (LPWSTR)&lpMsgBuf,
        0,
        NULL);

    std::string message;
    if (bufLen > 0) {
        // Преобразуем wide string в narrow string
        wchar_t* wideMessage = (wchar_t*)lpMsgBuf;
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wideMessage, -1, NULL, 0, NULL, NULL);
        message.resize(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wideMessage, -1, &message[0], size_needed, NULL, NULL);
        LocalFree(lpMsgBuf); // Освобождаем память
    }
    else {
        // Если bufLen равно 0, значит произошла ошибка
        DWORD error = GetLastError();
        message = "Не удалось получить сообщение об ошибке. Код ошибки: " + std::to_string(error);
    }

    return message;
}