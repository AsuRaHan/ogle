#include "OpenGLDebug.h"
#include <sstream>
#include <iomanip>

OpenGLDebug::OpenGLDebug() {
}

OpenGLDebug::~OpenGLDebug() {
}

bool OpenGLDebug::Initialize() {
    LOG_INFO("Инициализация отладки OpenGL");

    // Проверяем, что функции загружены
    if (!glDebugMessageCallback || !glDebugMessageControl) {
        LOG_ERROR("Не удалось загрузить функции отладки OpenGL");
        return false;
    }

    // Устанавливаем коллбэк
    GL_CHECK(glDebugMessageCallback(DebugCallback, nullptr));

    // Включаем отладочный вывод
    GL_CHECK(glEnable(GL_DEBUG_OUTPUT));
    GL_CHECK(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS)); // Синхронный вывод для точного стека вызовов

    // Фильтрация сообщений (опционально)
    // Отключаем сообщения низкой серьезности
    GL_CHECK(glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE));


    LOG_INFO("Отладка OpenGL успешно инициализирована");

    // Инициализация работы с символами
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
        LOG_ERROR("Ошибка инициализации символов");
        return false;
    }

    return true;
}

void APIENTRY OpenGLDebug::DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam) {
    // Преобразуем перечисления в читаемые строки
    const char* sourceStr;
    switch (source) {
    case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    default:                              sourceStr = "Unknown"; break;
    }

    const char* typeStr;
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behavior"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behavior"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    default:                                typeStr = "Unknown"; break;
    }

    const char* severityStr;
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         severityStr = "High"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          severityStr = "Low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Notification"; break;
    default:                             severityStr = "Unknown"; break;
    }
    if (type == GL_DEBUG_TYPE_PERFORMANCE) {
        return;
    }
    // Захватываем стек вызовов
    const int maxFrames = 16;
    void* stack[maxFrames];
    USHORT frames = CaptureStackBackTrace(0, maxFrames, stack, NULL);

    // Инициализируем символы для трассировки
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);

    {
        std::ostringstream log;
        log << "OpenGL Debug Message:\n"
            << "  Source: " << sourceStr << "\n"
            << "  Type: " << typeStr << "\n"
            << "  ID: " << id << "\n"
            << "  Severity: " << severityStr << "\n"
            << "  Message: " << message << "\n"
            << "  Stack Trace:\n";

        // Выводим трассировку стека
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
        symbol->MaxNameLen = 255;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (USHORT i = 0; i < frames; i++) {
            DWORD64 displacement = 0;
            if (SymFromAddr(process, (DWORD64)(stack[i]), &displacement, symbol)) {
                log << "    " << i << ": " << symbol->Name << " - 0x" << std::hex << symbol->Address << " + " << std::dec << displacement << "\n";

                // Получаем информацию о строке кода
                IMAGEHLP_LINE64 lineInfo = { 0 };
                lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                DWORD lineDisplacement = 0;
                if (SymGetLineFromAddr64(process, symbol->Address, &lineDisplacement, &lineInfo)) {
                    log << "        File: " << lineInfo.FileName << "\n";
                    log << "        Line: " << lineInfo.LineNumber << "\n";
                }
                else {
                    log << "        <Line info not available>\n";
                }
            }
            else {
                log << "    " << i << ": <unknown symbol>\n";
            }
        }

        free(symbol);
        SymCleanup(process);
        LOG_ERROR(log.str());
    }

    // High-severity messages should stay visible in logs, but they should not
    // immediately kill the editor. When a debugger is attached we break there
    // so the issue is still easy to inspect locally.
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        if (IsDebuggerPresent()) {
            DebugBreak();
        }
    }
}
