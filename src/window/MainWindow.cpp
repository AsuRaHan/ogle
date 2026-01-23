#include "MainWindow.h"
#include <format>       // C++20 std::format
#include <wingdi.h>     // для GDI функций

MainWindow::MainWindow(HINSTANCE hInstance)
    : BaseWindow(hInstance)
    , m_glContext(::GetDC(GetHWND()))   // пока nullptr, но после Create будет готово
    , m_statusText(L"Приложение запущено | Нажмите ESC для выхода")
{
}

bool MainWindow::CreateMainWindow(const std::wstring& title, int width, int height)
{
    return Create(
        title,
        width,
        height,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WS_OVERLAPPEDWINDOW,
        0
    );
}

bool MainWindow::OnCreate()
{
    // Инициализируем OpenGL после создания окна (HDC уже есть)
    m_glContext = OpenGLContext(::GetDC(GetHWND()));  // перезаписываем с правильным HDC

    if (!m_glContext.Initialize(4, 6, true)) {  // true = debug контекст, если драйвер поддерживает
        MessageBox(nullptr, L"Не удалось инициализировать OpenGL 4.6", L"Ошибка", MB_ICONERROR);
        return false;
    }

    // Убираем GDI-рисование — теперь будет OpenGL
    // (можно временно закомментировать OnPaint или оставить заглушку)

    RequestRedraw();
    return true;
}

void MainWindow::OnDestroy()
{
    // Здесь освобождаем ресурсы, созданные в OnCreate
    // пока ничего
}

void MainWindow::OnPaint([[maybe_unused]] HDC hdc)
{
    // m_glContext.MakeCurrent();
    // glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT);
    // m_glContext.SwapBuffers();
    // RECT clientRect{};
    // GetClientRect(m_hwnd, &clientRect);

    // // Фон
    // HBRUSH bgBrush = CreateSolidBrush(RGB(20, 20, 35));  // тёмно-синий фон
    // FillRect(hdc, &clientRect, bgBrush);
    // DeleteObject(bgBrush);

    // // Основной текст по центру
    // SetBkMode(hdc, TRANSPARENT);
    // SetTextColor(hdc, RGB(220, 220, 255));

    // HFONT bigFont = CreateFont(
    //     40, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
    //     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    //     DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    // );

    // HFONT oldFont = (HFONT)SelectObject(hdc, bigFont);

    // std::wstring welcome = L"Добро пожаловать в OGLE";
    // SIZE textSize{};
    // GetTextExtentPoint32(hdc, welcome.c_str(), static_cast<int>(welcome.size()), &textSize);

    // int tx = (clientRect.right - textSize.cx) / 2;
    // int ty = (clientRect.bottom - textSize.cy) / 2 - 80;

    // TextOut(hdc, tx, ty, welcome.c_str(), static_cast<int>(welcome.size()));

    // // Статусная строка внизу
    // SetTextColor(hdc, RGB(150, 200, 255));

    // HFONT smallFont = CreateFont(
    //     18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    //     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    //     DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Consolas"
    // );

    // SelectObject(hdc, smallFont);

    // std::wstring status = std::format(
    //     L"{}   |   Размер: {} × {}   |   Перерисовок: {}",
    //     m_statusText,
    //     m_clientWidth,
    //     m_clientHeight,
    //     m_redrawCount++
    // );

    // TextOut(hdc, 20, clientRect.bottom - 35, status.c_str(), static_cast<int>(status.size()));

    // // Восстановление
    // SelectObject(hdc, oldFont);
    // DeleteObject(bigFont);
    // DeleteObject(smallFont);
}

void MainWindow::OnResize(int width, int height)
{
    BaseWindow::OnResize(width, height);
    RequestRedraw();  // перерисовываем при изменении размера
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE)
            {
                DestroyWindow(m_hwnd);
                return 0;
            }
            break;

        default:
            break;
    }

    return BaseWindow::HandleMessage(msg, wParam, lParam);
}