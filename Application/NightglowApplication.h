//
// Created by danie on 9/6/2026.
//
#pragma once

#ifndef NIGHTGLOW_ENGINE_NIGHTGLOWAPPLICATION_H
#define NIGHTGLOW_ENGINE_NIGHTGLOWAPPLICATION_H

#include <windows.h>
#include <cstdint>
#include <string>

class BaseWindow;

class NightglowApplication {
public:
    NightglowApplication(uint32_t width, uint32_t height, std::wstring title);
    virtual ~NightglowApplication();

    NightglowApplication(const NightglowApplication&) = delete;
    NightglowApplication& operator=(const NightglowApplication&) = delete;


    int Run(HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

protected:
    // Called once after the window is created. Return false to abort
    // startup. Set up your D3D12 device/swap chain/command queue here.
    virtual bool OnInit();

    // Called once per frame before OnRender(). Update scene state here.
    virtual void OnUpdate();

    // Called once per frame. Record and submit command lists here.
    virtual void OnRender();

    // Called once before the window is destroyed. Release D3D12
    // resources here.
    virtual void OnDestroy();

    // Called whenever the client area changes size (e.g. resize the
    // swap chain buffers here).
    virtual void OnResize(uint32_t width, uint32_t height);


private:
    static HWND m_hwnd;
    uint32_t m_width;
    uint32_t m_height;
    std::wstring m_title;
    bool m_isRunning = false;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool InitWindow(HINSTANCE hInstance, int nCmdShow);
};


#endif //NIGHTGLOW_ENGINE_NIGHTGLOWAPPLICATION_H
