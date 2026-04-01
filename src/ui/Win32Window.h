#pragma once

#include "IWindow.h"
#include <functional>
#include <vector>

class Win32Window : public IWindow
{
public:
    using MessageHandler = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

    Win32Window();
    ~Win32Window() override;

    bool Create(HINSTANCE hInstance, HWND hParent = nullptr);
    void Show(int nCmdShow) override;
    HWND Handle() const override;

    HDC GetDeviceContext() const override;
    void SetDeviceContext(HDC hdc) override;

    HGLRC GetGLContext() const override;
    void SetGLContext(HGLRC hglrc) override;

    int Width() const override;
    int Height() const override;
    void SetSize(int width, int height) override;

    std::wstring Title() const override;
    void SetTitle(const std::wstring& title) override;

    // Configurable window style
    DWORD Style() const override;
    DWORD ExStyle() const override;
    void SetStyle(DWORD style) override;
    void SetExStyle(DWORD exStyle) override;

    void SetMessageHandler(MessageHandler handler);
    void SetMessageObserver(MessageObserver observer) override;
    void AddMessageObserver(MessageObserver observer) override;

protected:
    virtual PCWSTR ClassName() const;
    virtual LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    HDC m_hdc;
    HGLRC m_hglrc;
    int m_width;
    int m_height;
    std::wstring m_title;
    DWORD m_style;
    DWORD m_exStyle;
    HWND m_parent;
    MessageHandler m_handler;
    std::vector<MessageObserver> m_observers;
};
