#pragma once

#include <functional>
#include <string>
#include <utility>
#include <windows.h>

class IWindow
{
public:
    using MessageObserver = std::function<void(UINT, WPARAM, LPARAM)>;

    virtual ~IWindow() = default;
    virtual bool Create(HINSTANCE hInstance, HWND hParent = nullptr) = 0;
    virtual void Show(int nCmdShow) = 0;
    virtual HWND Handle() const = 0;
    virtual void SetMessageObserver(MessageObserver observer) { (void)observer; }
    virtual void AddMessageObserver(MessageObserver observer) { SetMessageObserver(std::move(observer)); }

    // OpenGL-related descriptors/getters-setters
    virtual HDC GetDeviceContext() const { return nullptr; }
    virtual void SetDeviceContext(HDC hdc) { (void)hdc; }

    virtual HGLRC GetGLContext() const { return nullptr; }
    virtual void SetGLContext(HGLRC hglrc) { (void)hglrc; }

    // Window style switching API
    virtual DWORD Style() const { return 0; }
    virtual DWORD ExStyle() const { return 0; }
    virtual void SetStyle(DWORD style) { (void)style; }
    virtual void SetExStyle(DWORD exStyle) { (void)exStyle; }

    // Window state
    virtual int Width() const { return 0; }
    virtual int Height() const { return 0; }
    virtual void SetSize(int width, int height) { (void)width; (void)height; }

    virtual std::wstring Title() const { return {}; }
    virtual void SetTitle(const std::wstring& title) { (void)title; }
    // Window state persistence
    virtual bool LoadWindowState(const std::string& filePath) { (void)filePath; return false; }
    virtual bool SaveWindowState(const std::string& filePath) const { (void)filePath; return false; }
};
