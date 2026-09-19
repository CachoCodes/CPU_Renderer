#include <windows.h>
#include <cstdint>
#include "renderer.hpp"

BITMAPINFO bitmapInfo{};

void resizeFramebuffer(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) return;

    winSize.x = newWidth;
    winSize.y = newHeight;

    delete[] framebuffer;
    framebuffer = new uint32_t[winSize.x * winSize.y];

    bitmapInfo.bmiHeader.biWidth = winSize.x;
    bitmapInfo.bmiHeader.biHeight = -winSize.y;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            resizeFramebuffer(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "CPURenderer";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "CPURenderer", "CPURenderer", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, winSize.x, winSize.y, nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;

    ShowWindow(hwnd, SW_SHOW);

    bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmapInfo.bmiHeader.biWidth = winSize.x;
    bitmapInfo.bmiHeader.biHeight = -winSize.y;
    bitmapInfo.bmiHeader.biPlanes = 1;
    bitmapInfo.bmiHeader.biBitCount = 32;
    bitmapInfo.bmiHeader.biCompression = BI_RGB;

    resizeFramebuffer(winSize.x, winSize.y);

    bool running = true;

   while (running) {
    MSG msg;

    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) running = false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RENDERLOOP(framebuffer, winSize);

    HDC hdc = GetDC(hwnd);

    StretchDIBits(hdc, 0, 0, winSize.x, winSize.y, 0, 0, winSize.x, winSize.y, framebuffer, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(hwnd, hdc);
}

    delete[] framebuffer;

    return 0;
}