//
// Created by danie on 9/6/2026.
//

#include "NightglowApplication.h"
#include "../Windows/BaseWindow.h"

HWND NightglowApplication::m_hwnd = nullptr;

bool NightglowApplication::OnInit() { return true; }

void NightglowApplication::OnUpdate() {}

void NightglowApplication::OnRender(){}

void NightglowApplication::OnDestroy(){}

void NightglowApplication::OnResize(uint32_t width, uint32_t height){}

NightglowApplication::NightglowApplication(uint32_t width, uint32_t height, std::wstring title)
    : m_width(width)
    , m_height(height)
    , m_title(std::move(title))
{}

NightglowApplication::~NightglowApplication() = default;

int NightglowApplication::Run(HINSTANCE hInstance, int nCmdShow) {
    if (!InitWindow(hInstance, nCmdShow)) { return -1; }
    if (!OnInit()) { return -1; }

    m_isRunning = true;

    MSG msg{};
    while (m_isRunning) {
        while (PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE )) {
            if (msg.message == WM_QUIT) {
                m_isRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (!m_isRunning) { break; }
        OnUpdate();
        OnRender();
    }

    OnDestroy();
    return static_cast<int>(msg.wParam);
}

// TODO: Replace this with proper input handling later.
LRESULT NightglowApplication::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    BaseWindow* pWindow = reinterpret_cast<BaseWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (message) {
    case WM_LBUTTONDOWN: {
            MessageBox(nullptr, L"Hello, World", L"Hello", MB_OK);
        }
        return 0;
    case WM_KEYDOWN:
        if( wParam == VK_ESCAPE ) {
            DestroyWindow(m_hwnd);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    // Handle any messages the switch statement didn't.
    return DefWindowProc(hWnd, message, wParam, lParam);
}

bool NightglowApplication::InitWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wc.lpszClassName = L"BasicWndClass";

    if (!RegisterClassEx(&wc)) {
        MessageBox(nullptr, L"RegisterClass FAILED", nullptr, 0);
        return false;
    }

    m_hwnd = CreateWindowEx(
        0,
        L"BasicWndClass",                // Registered WNDCLASS instance to use.
        m_title.c_str(),                // window title
        WS_OVERLAPPEDWINDOW,            // style flags
        CW_USEDEFAULT,                  // x-coordinate
        CW_USEDEFAULT,                  // y-coordinate
        m_width,                        // width
        m_height,                       // height
        nullptr,                        // parent window
        nullptr,                        // menu handle
        hInstance,                      // app instance
        this
    );

    if (!m_hwnd) {
        MessageBox(nullptr, L"CreateWindow FAILED", nullptr, 0);
        return false;
    }

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}
