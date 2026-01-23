#include "BaseWindow.h"

ATOM BaseWindow::s_classAtom = 0;
const wchar_t* BaseWindow::s_className = L"OGLE_BaseWindowClass_2025";

BaseWindow::BaseWindow(HINSTANCE hInst) : m_hInstance(hInst)
{
}

BaseWindow::~BaseWindow()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    if (m_hdc)
    {
        ::ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }
}

bool BaseWindow::RegisterWindowClass(HINSTANCE hInst)
{
    if (s_classAtom) return true;

    WNDCLASSEX wc{};
    wc.cbSize        = sizeof wc;
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = StaticWndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = s_className;
    wc.hIconSm       = LoadIcon(nullptr, IDI_APPLICATION);

    ATOM atom = RegisterClassEx(&wc);
    if (!atom)
    {
        wchar_t buf[256];
        wsprintf(buf, L"RegisterClassEx failed\nError code: %lu", GetLastError());
        MessageBox(nullptr, buf, L"Ошибка регистрации класса окна", MB_ICONERROR);
        return false;
    }

    s_classAtom = atom;
    return true;
}

bool BaseWindow::Create(
    const std::wstring& title,
    int width, int height, int x, int y,
    DWORD style, DWORD exStyle,
    HWND parent, HMENU menu
)
{
    if (!RegisterWindowClass(m_hInstance))
        return false;

    return CreateImpl(title, width, height, x, y, style, exStyle, parent, menu);
}

bool BaseWindow::CreateImpl(
    const std::wstring& title,
    int width, int height, int x, int y,
    DWORD style, DWORD exStyle,
    HWND parent, HMENU menu
)
{
    m_title = title;

    RECT rc{ 0, 0, width, height };
    AdjustWindowRectEx(&rc, style, FALSE, exStyle);

    m_hwnd = CreateWindowEx(
        exStyle,
        s_className,
        title.c_str(),
        style,
        x, y,
        rc.right - rc.left,
        rc.bottom - rc.top,
        parent,
        menu,
        m_hInstance,
        this
    );

    if (!m_hwnd)
    {
        wchar_t buf[256];
        wsprintf(buf, L"CreateWindowEx failed\nError code: %lu\nClass: %s", GetLastError(), s_className);
        MessageBox(nullptr, buf, L"Ошибка создания окна", MB_ICONERROR);
        return false;
    }

    m_hdc = ::GetDC(m_hwnd);
    if (!m_hdc)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        return false;
    }

    CenterWindow();

    if (!OnCreate())
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
        return false;
    }

    return true;
}

void BaseWindow::Show(int nCmdShow)
{
    if (m_hwnd)
    {
        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);
    }
}

int BaseWindow::RunMessageLoop()
{
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

LRESULT BaseWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        OnDestroy();
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        OnClose();
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(m_hwnd, &ps);
        OnPaint(hdc);
        EndPaint(m_hwnd, &ps);
        return 0;
    }

    case WM_SIZE:
        OnResize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    default:
        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

LRESULT CALLBACK BaseWindow::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE)
    {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        BaseWindow* self = static_cast<BaseWindow*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        return TRUE;
    }

    BaseWindow* self = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (self)
        return self->HandleMessage(msg, wParam, lParam);

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void BaseWindow::CenterWindow()
{
    if (!m_hwnd) return;

    RECT rc{};
    GetWindowRect(m_hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);

    SetWindowPos(m_hwnd, nullptr, (sw - w) / 2, (sh - h) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void BaseWindow::OnResize(int width, int height)
{
    m_clientWidth = width;
    m_clientHeight = height > 0 ? height : 1;  // защита от 0

    if (m_onResizeCallback)
    {
        m_onResizeCallback(width, height);  // вызываем callback
    }

    RequestRedraw();
}

void BaseWindow::SetResizeCallback(const std::function<void(int width, int height)>& callback)
    {
        m_onResizeCallback = callback;
    }
float BaseWindow::GetAspectRatio() const noexcept
{
    // Если высота = 0 или отрицательная — возвращаем 1.0f (стандартный квадратный aspect)
    // Это предотвращает деление на ноль и NaN в матрице проекции
    if (m_clientHeight <= 0) {
        return 1.0f;
    }

    return static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight);
}