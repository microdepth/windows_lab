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
#include <windowsx.h>
#pragma comment(lib, "d2d1.lib")

const WCHAR WINDOW_NAME[] = L"window name";

class DPIScale {
    static float scale;

public:
    static void Initialize(HWND hwnd) {
        float dpi = GetDpiForWindow(hwnd);
        scale = dpi / 96.0f;
    }

    template <typename T>
    static D2D1_POINT_2F PixelsToDips(T x, T y) {
        return D2D1::Point2F(static_cast<float>(x) / scale, static_cast<float>(y) / scale);
    }
};
float DPIScale::scale = 1.0f;

class Scene : public GraphicsScene {
    CComPtr<ID2D1SolidColorBrush> m_pGreen;
    CComPtr<ID2D1SolidColorBrush> m_pBlack;
    CComPtr<ID2D1SolidColorBrush> m_pRed;

    D2D1_ELLIPSE m_ellipse;
    D2D1_ELLIPSE m_ellipse2;
    D2D1_ELLIPSE m_ellipse3;
    D2D1_POINT_2F m_cursor;
    D2D_POINT_2F m_Ticks[24];

    HRESULT CreateDeviceIndependentResources() { return S_OK; }
    void DiscardDeviceIndependentResources() {}
    HRESULT CreateDeviceDependentResources();
    void DiscardDeviceDependentResources();
    void CalculateLayout();
    void RenderScene();

    void DrawClockHand(float fHandLength, float fAngle, float fStrokeWidth);

public:
    void OnLButtonDown(int x, int y, DWORD flags);
    void OnLButtonUp(int x, int y, DWORD flags);
    void OnMouseMove(int x, int y, DWORD flags);
};
HRESULT Scene::CreateDeviceDependentResources() {
    HRESULT hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(0.5f, 1.5f, 0.5f),
        D2D1::BrushProperties(),
        &m_pGreen
        );

    if (SUCCEEDED(hr)) {
        hr = m_pRenderTarget->CreateSolidColorBrush(
            D2D1::ColorF(0, 0, 0),
            D2D1::BrushProperties(),
            &m_pBlack
            );
    }

    if (SUCCEEDED(hr)) {
        hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(1.0f, 0, 0),
                D2D1::BrushProperties(),
                &m_pRed
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
        m_ellipse.point, endPoint, m_pBlack, fStrokeWidth);
}
void Scene::RenderScene() {
    m_pRenderTarget->Clear(D2D1::ColorF(0, 0, 0));

    // draw ellipse
    m_pRenderTarget->FillEllipse(m_ellipse, m_pGreen);
    m_pRenderTarget->DrawEllipse(m_ellipse, m_pBlack);

    m_pRenderTarget->FillEllipse(m_ellipse2, m_pRed);
    m_pRenderTarget->DrawEllipse(m_ellipse2, m_pBlack);

    // draw tick marks
    for (DWORD i = 0; i < 12; i++) {
        m_pRenderTarget->DrawLine(m_Ticks[i*2], m_Ticks[i*2+1], m_pBlack, 2.0f);
    }

    // draw hands
    SYSTEMTIME time;
    GetLocalTime(&time);

    const float fHourAngle = ((360.0f / 12) * time.wHour) + (((360.0f / 12) / 60) * time.wMinute) + ((((360.0f / 12) / 60) / 60) * time.wSecond) + (((((360.0f / 12) / 60) / 60) / 1000) * time.wMilliseconds);
    const float fMinuteAngle = ((360.0f / 60) * time.wMinute) + (((360.0f / 60) / 60) * time.wSecond) + (((360.0f / 60) / 60) / 1000) * time.wMilliseconds;
    const float fSecondAngle = ((360.0f / 60) * time.wSecond) + (((360.0f / 60) / 1000) * time.wMilliseconds);
    const float fMillisecondAngle = ((360.0f / 1000) * time.wMilliseconds);

    DrawClockHand(0.6f,  fHourAngle,   6);
    DrawClockHand(0.85f, fMinuteAngle, 4);
    DrawClockHand(0.85f, fSecondAngle, 1);
    DrawClockHand(0.95f, fMillisecondAngle, 0.5);

    // reset transform
    m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}
void Scene::CalculateLayout() {
    D2D1_SIZE_F fSize = m_pRenderTarget->GetSize();

    const float x = fSize.width / 2.0f;
    const float y = fSize.height / 2.0f;
    const float radius = std::min(x, y);

    m_ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);

    D2D_POINT_2F pt1 = D2D1::Point2F(
        m_ellipse.point.x,
        m_ellipse.point.y + (m_ellipse.radiusY * 0.9f)
        );

    D2D_POINT_2F pt2 = D2D1::Point2F(
        m_ellipse.point.x,
        m_ellipse.point.y + (m_ellipse.radiusY * 0.96f)
        );

    for (DWORD i = 0; i < 12; i++) {
        D2D1::Matrix3x2F mat = D2D1::Matrix3x2F::Rotation(
            (360.0f / 12) * i, m_ellipse.point);

        m_Ticks[i*2] = mat.TransformPoint(pt1);
        m_Ticks[i*2 + 1] = mat.TransformPoint(pt2);
    }
}
void Scene::DiscardDeviceDependentResources() {
    m_pGreen.Release();
    m_pBlack.Release();
}

void Scene::OnLButtonDown(int x, int y, DWORD flags) {
    m_ellipse2.point = m_cursor = DPIScale::PixelsToDips(x, y);
    m_ellipse2.radiusX = m_ellipse2.radiusY = 1.0f;
};
void Scene::OnLButtonUp(int x, int y, DWORD flags) {

};
void Scene::OnMouseMove(int x, int y, DWORD flags) {
    if (flags & MK_LBUTTON) {
        const D2D1_POINT_2F dips = DPIScale::PixelsToDips(x, y);

        const float width = (dips.x - m_cursor.x) / 2;
        const float height = (dips.y - m_cursor.y) / 2;
        const float centerX = m_cursor.x + width;
        const float centerY = m_cursor.y + height;

        m_ellipse2.point = D2D1::Point2F(centerX, centerY);
        m_ellipse2.radiusX = width;
        m_ellipse2.radiusY = height;
    }
};

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
LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    HWND hwnd = m_hwnd;
    try {
        switch (uMsg) {
            case WM_LBUTTONDOWN: {
                SetCapture(m_hwnd);
                m_scene.OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return 0;
            }
            case WM_LBUTTONUP: {
                m_scene.OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
                ReleaseCapture();
                return 0;
            }
            case WM_MOUSEMOVE: {
                m_scene.OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return 0;
            }
            case WM_CREATE: {
                if (FAILED(m_scene.Initialize()) || !InitializeTimer()) {
                    return -1;
                }
                DPIScale::Initialize(hwnd);
                return 0;
            }
            case WM_SIZE: {
                int x = (short)LOWORD(lParam);
                int y = (short)HIWORD(lParam);
                m_scene.Resize(x, y);
                InvalidateRect(m_hwnd, nullptr, FALSE);
                return 0;
            }
            case WM_PAINT: { // todo figure out why this needs to be here and empty

            }
            case WM_DISPLAYCHANGE: {
                PAINTSTRUCT ps;
                BeginPaint(m_hwnd, &ps);
                m_scene.Render(hwnd);
                EndPaint(m_hwnd, &ps);
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {
        return 0;
    }

    MainWindow win;
    if (!win.Create(L"learning win32 ui", WS_OVERLAPPEDWINDOW)) {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        win.WaitTimer();
    };

    CoUninitialize();
    return 0;
}
