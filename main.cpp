#include <iostream>

#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <winuser.h>
#include <d2d1.h>
#include <assert.h>
#include <atlbase.h>
#include "basewin.h"
#pragma comment(lib, "d2d1.lib")

float g_DPIScale = 1.0f;

template <typename T>
void SafeRelease(T** ppt) {
    if (*ppt) {
        (*ppt)->Release();
        *ppt = nullptr;
    }
}

void InitializeDPIScale(HWND hwnd) {
    float dpi = GetDpiForWindow(hwnd);
    g_DPIScale = dpi / USER_DEFAULT_SCREEN_DPI;
}

template <typename T>
float PixelsToDips(T x) { // we gotta add constraints on this type somehow to make sure nothing inappropriate is getting through
    return static_cast<float>(x) / g_DPIScale;
}

class GraphicsScene {

};

class MainWindow : public BaseWindow<MainWindow> {
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    D2D1_ELLIPSE ellipse;
    D2D1_RECT_F rect;

    void CalculateLayout();
    HRESULT CreateGraphicsResources();
    void DiscardGraphicsResources();
    void OnPaint();
    void Resize();
public:
    MainWindow() : pFactory(nullptr), pRenderTarget(nullptr), pBrush(nullptr), ellipse() {}

    PCWSTR ClassName() const { return L"circle window class"; }
    LRESULT CALLBACK HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        try {
            switch (uMsg) {
                case WM_CREATE: {
                    if (FAILED(D2D1CreateFactory(
                        D2D1_FACTORY_TYPE_SINGLE_THREADED,
                        &pFactory))) {
                        return -1;
                    }
                    return 0;
                }
                case WM_SIZE: {
                    Resize();
                    return 0;
                }
                case WM_PAINT: {
                    OnPaint();
                    return 0;
                }
                case WM_CLOSE: {
                    if (MessageBox(hwnd, L"quit?", L"my app", MB_OKCANCEL) == IDOK) {
                        DestroyWindow(hwnd);
                    }
                    return 0;
                }
                case WM_DESTROY: {
                    DiscardGraphicsResources();
                    SafeRelease(&pFactory);
                    PostQuitMessage(0);
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
    };
};

void MainWindow::CalculateLayout() {
    if (pRenderTarget != nullptr) {
        D2D1_SIZE_F ellipseSize = pRenderTarget->GetSize();
        const float ellipseCenterX = ellipseSize.width / 2;
        const float ellipseCenterY = ellipseSize.height / 2;
        const float radius = std::min(ellipseCenterX, ellipseCenterY);
        ellipse = D2D1::Ellipse(D2D1::Point2F(ellipseCenterX, ellipseCenterY), radius, radius);

        D2D1_SIZE_F rectSize = pRenderTarget->GetSize();
        const float rectWidth = rectSize.width;
        const float rectHeight = rectSize.height;
        rect = D2D1::RectF(0.0f, 0.0f, rectWidth, rectHeight);
    }
}
HRESULT MainWindow::CreateGraphicsResources() {
    HRESULT hr = S_OK;
    if (pRenderTarget == nullptr) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr)) {
            const D2D1_COLOR_F color = D2D1::ColorF(0.0f, 1.0f, 0.0f, 0.5f);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr)) {
                CalculateLayout();
            }
        }
    }
    return hr;
}
void MainWindow::DiscardGraphicsResources() {
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}
void MainWindow::OnPaint() {
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr)) {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);
        pRenderTarget->BeginDraw();

        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Gray));
        pRenderTarget->DrawRectangle(rect, pBrush, 10.0f, nullptr);
        pRenderTarget->FillEllipse(ellipse, pBrush);

        pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}
void MainWindow::Resize() {
    if (pRenderTarget != nullptr) {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

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