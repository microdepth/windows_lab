//
// Created by psdab on 6/4/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include <atlcomcli.h>
#include <d2d1.h>

class GraphicsScene {
protected:
    CComPtr<ID2D1Factory> m_pFactory;
    CComPtr<ID2D1HwndRenderTarget> m_pRenderTarget;

    float m_fScaleX;
    float m_fScaleY;

    virtual HRESULT CreateDeviceIndependentResources() = 0;
    virtual void    DiscardDeviceIndependentResources() = 0;
    virtual HRESULT CreateDeviceDependentResources() = 0;
    virtual void    DiscardDeviceDependentResources() = 0;
    virtual void    CalculateLayout() = 0;
    virtual void    RenderScene() = 0;

    HRESULT CreateGraphicsResources(HWND hwnd) {
        hr = S_OK;
        if (m_pRenderTarget == nullptr) {
            RECT rc;
            GetClientRect(hwnd, &rc);

            D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

            hr = m_pFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(hwnd, size),
                &m_pRenderTarget);

            if (SUCCEEDED(hr)) {
                hr = CreateDeviceDependentResources();
                if (SUCCEEDED(hr)) {
                    CalculateLayout();
                }
            }
        }
        return hr;
    };
    template <typename T>
    T PixelsToDipX(T pixels) {
        return static_cast<T>(pixels / m_fScaleX);
    }
    template <typename T>
    T PixelsToDipY(T pixels) {
        return static_cast<T>(pixels / m_fScaleY);
    }
public:
    GraphicsScene() : m_fScaleX(1.0f), m_fScaleY(1.0f) {}
    virtual ~GraphicsScene() {}

    HRESULT Initialize() {
        HRESULT hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            &m_pFactory);

        if (SUCCEEDED(hr)) {
            CreateDeviceIndependentResources();
        }
        return hr;
    }

    void Render() {
        HRESULT hr = CreateGraphicsResources(hwnd);

        assert(m_pRenderTarget);

        m_pRenderTarget->BeginDraw();
        RenderScene();
        hr = m_pRenderTarget->EndDraw();

        if (hr == D2DERR_RECREATE_TARGET) {
            DiscardDeviceDependentResources();
            m_pRenderTarget.Release();
        }
    }

    HRESULT Resize(int x, int y) {
        HRESULT hr = S_OK;
        if (m_pRenderTarget) {
            hr = m_pRenderTarget->Resize(D2D1::SizeU(x, y));
            if (SUCCEEDED(hr)) {
                CalculateLayout();
            }
        }
        return hr;
    }

    void CleanUp() {
        DiscardDeviceDependentResources();
        DiscardDeviceIndependentResources();
    }
};

#endif //SCENE_H
