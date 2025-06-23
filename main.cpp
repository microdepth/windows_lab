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
    CComPtr<ID2D1SolidColorBrush> m_pFill;
    CComPtr<ID2D1SolidColorBrush> m_pStroke;

    D2D1_ELLIPSE m_ellipse;
    D2D_POINT_2F m_Ticks[24];

    HRESULT CreateDeviceIndependentResources() { return S_OK; }
    void    DiscardDeviceIndependentResources() { }
    HRESULT CreateDeviceDependentResources();
    void    DiscardDeviceDependentResources();
    void    CalculateLayout();
    void    RenderScene();

    void    DrawClockHand(float fHandLength, float fAngle, float fStrokeWidth);
};

HRESULT Scene::CreateDeviceDependentResources() {
    HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(1.0f, 1.0f, 0),
        D2D1::BrushProperties(),
        &m_pFill
        );

    if (SUCCEEDED(hr)) {
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0, 0, 0),
            D2D1::BrushProperties(),
            &m_pStroke
            );
    }
    return hr;
}
void Scene::DrawClockHand(float fHandLength, float fAngle, float fStrokeWidth) {
    m_pRenderTarget->SetTransform(
        D2D1::Matrix3x2F::Rotation(fAngle, m_ellipse.point)
            );
    
    D2D_POINT_2F endPoint = D2D1::Point2F(
        m_ellipse.point.x,
        m_ellipse.point.y - (m_ellipse.radiusY * fHandLength)
        );

    m_pRenderTarget->DrawLine(
        m_ellipse.point, endPoint, m_pStroke, fStrokeWidth);
}

class MainWindow : public BaseWindow<MainWindow> {
    HANDLE m_hTimer;
    Scene m_scene;

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
                if (FAILED(m_scene.Initialize()) || !InitializeTimer()) {
                    return -1;
                }
                return 0;
            }
            case WM_SIZE: {
                int x = (int)(short)LOWORD(lParam);
                int y = (int)(short)HIWORD(lParam);
                m_scene.Resize(x, y);
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return 0;
            }
            case WM_PAINT: {}

            case WM_DISPLAYCHANGE: {
                PAINTSTRUCT ps;
                BeginPaint(m_hwnd, &ps);
                m_scene.Render(hwnd);
                EndPaint(m_hwnd, &ps);
                return 0;
            }
            case WM_CLOSE: {
                return 0;
            }
            case WM_DESTROY: {
                CloseHandle(m_hTimer);
                m_scene.CleanUp();
                PostQuitMessage(0);
                return 0;
            }
            case WM_ERASEBKGND: {
                return 1;
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
BOOL MainWindow::InitializeTimer() {
    m_hTimer = CreateWaitableTimer(nullptr, FALSE, nullptr);
    if (m_hTimer == nullptr) {
        return FALSE;
    }

    LARGE_INTEGER li = {0};

    if (!SetWaitableTimer(m_hTimer, &li, (1000/60), nullptr, nullptr, FALSE)) {
        CloseHandle(m_hTimer);
        m_hTimer = nullptr;
        return FALSE;
    }

    return TRUE;
}
void MainWindow::WaitTimer() {
    if (MsgWaitForMultipleObjects(1, &m_hTimer, FALSE, INFINITE, QS_ALLINPUT)
            == WAIT_OBJECT_0) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}
