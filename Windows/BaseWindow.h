//
// Created by danie on 9/5/2026.
//

#ifndef NIGHTGLOW_ENGINE_BASEWINDOW_H
#define NIGHTGLOW_ENGINE_BASEWINDOW_H

#include <string>
#include <windows.h>

class IDXGIFactory1;
class IDXGIAdapter1;

class BaseWindow {
public:
    BaseWindow(UINT width, UINT height, std::wstring name);
    virtual ~BaseWindow();

    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;

    // TODO: Replace this later with implementation of Microsoft's GameInput API.
    virtual void OnKeyDown(UINT8 key) {}
    virtual void OnKeyUp(UINT8 key) {}

    // Accessors
    [[nodiscard]] UINT GetWidth() const;
    [[nodiscard]] UINT GetHeight() const;
    [[nodiscard]] LPCWSTR GetTitle() const;

protected:
    // Viewport dimensions
    UINT m_width;
    UINT m_height;
    float m_aspectRatio;

    std::wstring GetAssetFullPath(LPCWSTR assetName) const;

    void GetHardwareAdapter(
        _In_ IDXGIFactory1* pFactory,
        _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
        bool requestHighPerformanceAdapter = false);

    void SetCustomWindowText(LPCWSTR text);

private:
    // Root path for content assets
    std::wstring m_assetsPath;

    std::wstring m_title;
};


#endif //NIGHTGLOW_ENGINE_BASEWINDOW_H
