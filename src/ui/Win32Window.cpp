#include "Win32Window.h"
#include <windows.h>

namespace {
    constexpr int WINDOW_WIDTH = 800;
    constexpr int WINDOW_HEIGHT = 600;
}

Win32Window::Win32Window()
    : m_hwnd(nullptr)
    , m_hdc(nullptr)
    , m_hglrc(nullptr)
    , m_width(WINDOW_WIDTH)
    , m_height(WINDOW_HEIGHT)
    , m_title(L"My Windows App")
    , m_style(WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_THICKFRAME))
    , m_exStyle(0)
    , m_parent(nullptr)
    , m_handler(nullptr)
{}

Win32Window::~Win32Window()
{
    m_hglrc = nullptr;

    if (m_hdc && m_hwnd)
    {
        ReleaseDC(m_hwnd, m_hdc);
        m_hdc = nullptr;
    }

    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
}

PCWSTR Win32Window::ClassName() const
{
    return L"OGLE3DWindowClass";
}

bool Win32Window::Create(HINSTANCE hInstance, HWND hParent)
{
    m_parent = hParent;

    WNDCLASSW wc = {};
    wc.lpfnWndProc = Win32Window::WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = ClassName();
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!GetClassInfoW(hInstance, ClassName(), &wc))
    {
        if (!RegisterClassW(&wc))
            return false;
    }

    RECT rc = {0, 0, m_width, m_height};
    AdjustWindowRectEx(&rc, m_style, FALSE, m_exStyle);
    int adjustedWidth = rc.right - rc.left;
    int adjustedHeight = rc.bottom - rc.top;

    m_hwnd = CreateWindowExW(
        m_exStyle,
        ClassName(),
        m_title.c_str(),
        m_style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        adjustedWidth,
        adjustedHeight,
        hParent,
        nullptr,
        hInstance,
        this);

    if (!m_hwnd)
        return false;

    m_hdc = GetDC(m_hwnd);
    return true;
}

void Win32Window::Show(int nCmdShow)
{
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
}

HWND Win32Window::Handle() const
{
    return m_hwnd;
}

void Win32Window::SetMessageHandler(MessageHandler handler)
{
    m_handler = std::move(handler);
}

void Win32Window::SetMessageObserver(MessageObserver observer)
{
    m_observers.clear();
    if (observer) {
        m_observers.push_back(std::move(observer));
    }
}

void Win32Window::AddMessageObserver(MessageObserver observer)
{
    if (observer) {
        m_observers.push_back(std::move(observer));
    }
}

HDC Win32Window::GetDeviceContext() const
{
    return m_hdc;
}

void Win32Window::SetDeviceContext(HDC hdc)
{
    m_hdc = hdc;
}

HGLRC Win32Window::GetGLContext() const
{
    return m_hglrc;
}

void Win32Window::SetGLContext(HGLRC hglrc)
{
    m_hglrc = hglrc;
}

int Win32Window::Width() const
{
    return m_width;
}

int Win32Window::Height() const
{
    return m_height;
}

void Win32Window::SetSize(int width, int height)
{
    m_width = (width < 1) ? 1 : width;
    m_height = (height < 1) ? 1 : height;

    if (!m_hwnd)
        return;

    RECT rc = { 0, 0, m_width, m_height };
    AdjustWindowRectEx(&rc, m_style, FALSE, m_exStyle);
    const int adjustedWidth = rc.right - rc.left;
    const int adjustedHeight = rc.bottom - rc.top;

    SetWindowPos(m_hwnd, nullptr, 0, 0, adjustedWidth, adjustedHeight,
        SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);
}

std::wstring Win32Window::Title() const
{
    return m_title;
}

DWORD Win32Window::Style() const
{
    return m_style;
}

DWORD Win32Window::ExStyle() const
{
    return m_exStyle;
}

void Win32Window::SetStyle(DWORD style)
{
    m_style = style;
    if (m_hwnd)
    {
        SetWindowLongPtrW(m_hwnd, GWL_STYLE, style);
        SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }
}

void Win32Window::SetExStyle(DWORD exStyle)
{
    m_exStyle = exStyle;
    if (m_hwnd)
    {
        SetWindowLongPtrW(m_hwnd, GWL_EXSTYLE, exStyle);
        SetWindowPos(m_hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    }
}

void Win32Window::SetTitle(const std::wstring& title)
{
    m_title = title;
    if (m_hwnd)
    {
        SetWindowTextW(m_hwnd, m_title.c_str());
    }
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    Win32Window* window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    if (msg == WM_NCCREATE)
    {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = reinterpret_cast<Win32Window*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->m_hwnd = hwnd;
    }

    if (window)
    {
        for (auto& observer : window->m_observers) {
            if (observer) {
                observer(msg, wParam, lParam);
            }
        }

        if (window->m_handler)
            return window->m_handler(hwnd, msg, wParam, lParam);

        return window->HandleMessage(msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Win32Window::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SIZE:
        m_width = LOWORD(lParam);
        m_height = HIWORD(lParam);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcW(m_hwnd, msg, wParam, lParam);
    }
}


// ------------------------------------------------------------------
// Window state persistence helpers
// ------------------------------------------------------------------
bool Win32Window::SetPosition(int x, int y)
{
    if (!m_hwnd)
        return false;
    // Use SWP_NOSENDCHANGING to avoid excessive repaint
    return SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOZORDER) != 0;
}

bool Win32Window::LoadWindowState(const std::string& filePath)
{
    // std::ifstream fileIn(filePath);
    // if (!fileIn.is_open())
    //     return false;
    // int x, y, w, h;
    // fileIn >> x >> y >> w >> h;
    // if (fileIn.fail())
    //     return false;
    // SetSize(w, h);
    // SetPosition(x, y);
    return true;
}

bool Win32Window::SaveWindowState(const std::string& filePath) const
{
    if (!m_hwnd)
        return false;
    RECT rect;
    if (!GetWindowRect(m_hwnd, &rect))
        return false;
    int x = rect.left;
    int y = rect.top;
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    // std::ofstream fileOut(filePath);
    // if (!fileOut.is_open())
    //     return false;
    // fileOut << x << ' ' << y << ' ' << w << ' ' << h;
    return true;
}

