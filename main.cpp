#include <iostream>

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#define NOMINMAX
#include <windows.h>
#include <winuser.h>
#include <d2d1.h>
#include <assert.h>
#include "basewin.h"
#include "scene.h"
#pragma comment(lib, "d2d1.lib")

const WCHAR WINDOW_NAME[] = L"window name";

class Scene : public GraphicsScene {

};

class MainWindow : public BaseWindow<MainWindow> {
    HANDLE m_hTimer;
    Scene m_Scene;

    BOOL InitializeTimer();
public:
    MainWindow() : m_hTimer(nullptr) {}

    void WaitTimer();

    PCWSTR ClassName() const { return L"clock window class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    MainWindow win;
    if (!win.Create(L"learning win32 ui", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HWND hwnd = m_hwnd;
    try {
        switch (uMsg) {
            case WM_CREATE: {
                return 0;
            }
            case WM_SIZE: {
                return 0;
            }
            case WM_PAINT: {
                return 0;
            }
            case WM_CLOSE: {
                return 0;
            }
            case WM_DESTROY: {
                return 0;
            }
            default: {
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
            }
        }
    } catch (const std::exception& e) {
        MessageBox(m_hwnd, L"exception caught in handle message", L"error", MB_OK | MB_ICONERROR);
        return 0;
    }
}
