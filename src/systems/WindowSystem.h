// src/systems/WindowSystem.h
#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include <vector>

#include "core/ISystem.h"
#include "log/Logger.h"

namespace ogle {

class WindowSystem final : public ISystem {
public:
    WindowSystem(HINSTANCE hInstance, const std::wstring& title, int width = 1280, int height = 720);
    ~WindowSystem() override;

    const std::string& GetName() const override { static std::string n = "WindowSystem"; return n; }

    bool Initialize() override;
    bool InitWindow();
    void Update(float deltaTime) override {}     // окно не тикает
    void Render() override {}                    // рендер не здесь
    void Shutdown() override;

    // События от Engine
    void OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;
    void OnResize(int width, int height) override;

    // Полезные геттеры
    HWND GetHWND() const { return m_hwnd; }
    HDC  GetHDC() const  { return m_hdc; }
    int  GetWidth() const  { return m_clientWidth; }
    int  GetHeight() const { return m_clientHeight; }
    float GetAspectRatio() const {
        return m_clientHeight > 0 ? static_cast<float>(m_clientWidth) / m_clientHeight : 1.0f;
    }

    // Подписка на ресайз (для RenderSystem и т.д.)
    using ResizeCallback = std::function<void(int, int)>;
    void AddResizeListener(ResizeCallback cb) { m_resizeListeners.push_back(std::move(cb)); }

    void Show(int nCmdShow = SW_SHOW);

private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    static ATOM s_classAtom;
    static const wchar_t* s_className;

    HINSTANCE m_hInstance;
    HWND      m_hwnd {nullptr};
    HDC       m_hdc  {nullptr};

    std::wstring m_title;
    int m_clientWidth  = 1280;
    int m_clientHeight = 720;

    std::vector<ResizeCallback> m_resizeListeners;

    bool RegisterWindowClass();
    void CenterWindow();
    void Cleanup();
};

} // namespace ogle