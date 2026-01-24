// src/systems/WindowSystem.cpp
#include "WindowSystem.h"

namespace ogle {

	ATOM WindowSystem::s_classAtom = 0;
	const wchar_t* WindowSystem::s_className = L"OGLE_WindowClass_2026";

	WindowSystem::WindowSystem(HINSTANCE hInstance, const std::wstring& title, int width, int height)
		: m_hInstance(hInstance), m_title(title), m_clientWidth(width), m_clientHeight(height) {
	}

	WindowSystem::~WindowSystem() {
		Cleanup();
	}

	bool WindowSystem::Initialize() {
		// Пока пустой — можно добавить подготовку ресурсов, которые не требуют HWND
		// Например: загрузка конфига, подготовка callback'ов и т.д.
		return true;
	}

	bool WindowSystem::InitWindow() {
		if (!RegisterWindowClass()) {
			Logger::Error("Failed to register window class");
			return false;
		}

		RECT rc{ 0, 0, m_clientWidth, m_clientHeight };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		m_hwnd = CreateWindowEx(
			0, s_className, m_title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top,
			nullptr, nullptr, m_hInstance, this
		);

		if (!m_hwnd) {
			Logger::Error("CreateWindowEx failed: " + std::to_string(GetLastError()));
			return false;
		}

		m_hdc = GetDC(m_hwnd);
		if (!m_hdc) {
			Logger::Error("GetDC failed");
			DestroyWindow(m_hwnd);
			return false;
		}

		CenterWindow();

		Logger::Info("WindowSystem initialized: " + std::to_string(m_clientWidth) + "x" + std::to_string(m_clientHeight));
		return true;
	}

	void WindowSystem::Shutdown() {
		Cleanup();
	}

	void WindowSystem::Cleanup() {
		if (m_hwnd) {
			DestroyWindow(m_hwnd);
			m_hwnd = nullptr;
		}
		if (m_hdc) {
			ReleaseDC(nullptr, m_hdc);
			m_hdc = nullptr;
		}
	}

	bool WindowSystem::RegisterWindowClass() {
		if (s_classAtom) return true;

		WNDCLASSEX wc{};
		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = StaticWndProc;
		wc.hInstance = m_hInstance;
		wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszClassName = s_className;
		wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

		s_classAtom = RegisterClassEx(&wc);
		if (!s_classAtom) {
			Logger::Error("RegisterClassEx failed: " + std::to_string(GetLastError()));
			return false;
		}

		return true;
	}

	LRESULT CALLBACK WindowSystem::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		if (msg == WM_NCCREATE) {
			CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
			return TRUE;
		}

		WindowSystem* self = reinterpret_cast<WindowSystem*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (self) {
			return self->HandleMessage(msg, wParam, lParam);
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	LRESULT WindowSystem::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		switch (msg) {
		case WM_SIZE: {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam) > 0 ? HIWORD(lParam) : 1;
			m_clientWidth = width;
			m_clientHeight = height;
			OnResize(width, height);
			return 0;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(m_hwnd, msg, wParam, lParam);
		}
	}

	void WindowSystem::OnWindowMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
		// Здесь можно добавить дополнительную обработку, если нужно
		// Но базовая уже в HandleMessage
	}

	void WindowSystem::OnResize(int width, int height) {
		for (auto& cb : m_resizeListeners) {
			cb(width, height);
		}
		Logger::Debug("Window resized: " + std::to_string(width) + "x" + std::to_string(height));
	}

	void WindowSystem::Show(int nCmdShow) {
		if (m_hwnd) {
			ShowWindow(m_hwnd, nCmdShow);
			UpdateWindow(m_hwnd);
		}
	}

	void WindowSystem::CenterWindow() {
		if (!m_hwnd) return;
		RECT rc{};
		GetWindowRect(m_hwnd, &rc);
		int w = rc.right - rc.left;
		int h = rc.bottom - rc.top;
		int sw = GetSystemMetrics(SM_CXSCREEN);
		int sh = GetSystemMetrics(SM_CYSCREEN);
		SetWindowPos(m_hwnd, nullptr, (sw - w) / 2, (sh - h) / 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

} // namespace ogle