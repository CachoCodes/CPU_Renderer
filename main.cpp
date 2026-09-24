#include <windows.h>
#include <vector>
#include <functional>
#include <cstdint>
#include <cstdio>
#include <chrono>
#include <cmath>
#include <iostream> 
#include <algorithm>
#include <stdexcept>
#include <string>
int frames = 0;
float fps = 0.0f;
auto last_time = std::chrono::high_resolution_clock::now();
float delta = 0.f;
constexpr float PI = 3.14159f;

#include "renderer.hpp"




BITMAPINFO bitmap_info{};

void resize_framebuffer(int new_width, int new_height) {
    if (new_width <= 0 || new_height <= 0) return;

    win_size.x = new_width;
    win_size.y = new_height;

    delete[] framebuffer;
    framebuffer = new uint32_t[win_size.x * win_size.y];

    delete[] depth_buffer;
    depth_buffer = new float[win_size.x * win_size.y]; // <- nuevo

    bitmap_info.bmiHeader.biWidth = win_size.x;
    bitmap_info.bmiHeader.biHeight = -win_size.y;
}


LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            resize_framebuffer(LOWORD(lParam), HIWORD(lParam));
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


int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE, LPSTR, int) {
    WNDCLASSA window_class{};

    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = h_instance;
    window_class.lpszClassName = "CPU_Renderer";
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&window_class)) {
        return 0;
    }


    HWND hwnd = CreateWindowExA(
        0,
        "CPU_Renderer",
        "CPU Renderer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        win_size.x,
        win_size.y,
        nullptr,
        nullptr,
        h_instance,
        nullptr
    );

    if (!hwnd) {
        return 0;
    }


    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    resize_framebuffer(win_size.x, win_size.y);

    ShowWindow(hwnd, SW_SHOW);


    bool running = true;

    init_render();

    if(debug_mode){
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    }

    auto last_frame_time = std::chrono::high_resolution_clock::now();
    auto fps_time = last_frame_time;

    while (running) {
        MSG msg{};

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running) {
            break;
        }

        auto frame_start = std::chrono::high_resolution_clock::now();

        delta = std::chrono::duration<float>(frame_start - last_frame_time).count();

        last_frame_time = frame_start;

        render_loop(framebuffer, win_size);

        frames++;

        auto now = std::chrono::high_resolution_clock::now();

    float elapsed = std::chrono::duration<float>(
        now - fps_time
    ).count();

    if (elapsed >= 1.0f) {
        fps = frames / elapsed;
        frames = 0;
        fps_time = now;

        char title[64];
        std::snprintf(title, sizeof(title),
            "CPU Renderer - %.0f FPS", fps);

        SetWindowTextA(hwnd, title);
    }

    HDC hdc = GetDC(hwnd);

    StretchDIBits(
        hdc,
        0, 0,
        win_size.x, win_size.y,
        0, 0,
        win_size.x, win_size.y,
        framebuffer,
        &bitmap_info,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    ReleaseDC(hwnd, hdc);
    }


    delete[] framebuffer;
    framebuffer = nullptr;

    return 0;
}