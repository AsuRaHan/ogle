#pragma once

#include <windows.h>
#include <functional>
#include <string>

class BaseWindow
{
public:
    explicit BaseWindow(HINSTANCE hInstance);
    virtual ~BaseWindow();

    BaseWindow(const BaseWindow&) = delete;
    BaseWindow& operator=(const BaseWindow&) = delete;
    BaseWindow(BaseWindow&&) = delete;
    BaseWindow& operator=(BaseWindow&&) = delete;

    bool Create(
        const std::wstring& title,
        int width     = CW_USEDEFAULT,
        int height    = CW_USEDEFAULT,
        int x         = CW_USEDEFAULT,
        int y         = CW_USEDEFAULT,
        DWORD style   = WS_OVERLAPPEDWINDOW,
        DWORD exStyle = 0,
        HWND  parent  = nullptr,
        HMENU menu    = nullptr
    );

    void Show(int nCmdShow = SW_SHOW);
    int  RunMessageLoop();

    HWND GetHWND()          const noexcept { return m_hwnd; }
    HDC  GetDrawingDC()     const noexcept { return m_hdc;  }

    int  GetClientWidth()   const { return m_clientWidth;  }
    int  GetClientHeight()  const { return m_clientHeight; }
    void SetResizeCallback(const std::function<void(int width, int height)>& callback);
    float GetAspectRatio() const noexcept;
protected:
    virtual bool OnCreate()                     { return true; }
    virtual void OnDestroy()                    {}
    virtual void OnPaint([[maybe_unused]] HDC hdc) {}
    virtual void OnResize(int w, int h);
    virtual void OnClose()                      { DestroyWindow(m_hwnd); }

    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void CenterWindow();
    void RequestRedraw() { if (m_hwnd) InvalidateRect(m_hwnd, nullptr, TRUE); }

    std::function<void(int width, int height)> m_onResizeCallback = nullptr;  // callback

    HWND      m_hwnd          { nullptr };
    HDC       m_hdc           { nullptr };
    std::wstring m_title;
    int       m_clientWidth   { 800 };
    int       m_clientHeight  { 600 };
    HINSTANCE m_hInstance     { nullptr };

private:
    static ATOM               s_classAtom;
    static const wchar_t*     s_className;

    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static bool RegisterWindowClass(HINSTANCE hInst);

    bool CreateImpl(
        const std::wstring& title,
        int width, int height, int x, int y,
        DWORD style, DWORD exStyle,
        HWND parent, HMENU menu
    );
};