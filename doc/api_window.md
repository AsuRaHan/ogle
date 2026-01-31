# Window / OpenGL API

## OpenGLContext

**Заголовок:** `src/window/OpenGLContext.h`

Создание и управление контекстом OpenGL (WGL на Windows).

| Метод | Описание |
|-------|----------|
| `explicit OpenGLContext(HDC hdc)` | Конструктор. Принимает device context окна. |
| `~OpenGLContext()` | Деструктор (освобождение контекста). |
| `bool Initialize(int major = 4, int minor = 6, bool debug = false)` | Создать современный контекст OpenGL (по умолчанию 4.6). При неудаче создаётся legacy-контекст. Возвращает true при успехе. |
| `std::string GetVersionString() const` | Строка версии OpenGL (например "4.6"). |
| `void MakeCurrent() const` | Сделать контекст текущим для потока. |
| `void SwapBuffers() const` | Поменять буферы (отобразить кадр). |
| `void Cleanup()` | Освободить контекст и связанные ресурсы. |
