#pragma once

#include "BaseWindow.h"
#include "OpenGLContext.h"
#include <string>

class MainWindow : public BaseWindow
{
public:
    explicit MainWindow(HINSTANCE hInstance);

    // Метод создания окна с удобными параметрами по умолчанию
    bool CreateMainWindow(
        const std::wstring& title = L"OGLE - Главное окно",
        int width  = 1280,
        int height = 720
    );

protected:
    bool OnCreate() override;
    void OnDestroy() override;
    void OnPaint(HDC hdc) override;
    void OnResize(int width, int height) override;

    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

private:
    std::wstring m_statusText;
    int          m_redrawCount {0};
    OpenGLContext m_glContext;
};