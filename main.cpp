#include "../overlay/overlay.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx9.h"
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

LPDIRECT3D9           g_pD3D = nullptr;
LPDIRECT3DDEVICE9     g_pd3dDevice = nullptr;
D3DPRESENT_PARAMETERS g_d3dpp = {};

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateDeviceD3D(HWND hWnd)
{
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_pD3D) return false;

    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    HRESULT hr = g_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &g_d3dpp,
        &g_pd3dDevice
    );
    return SUCCEEDED(hr);
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release();       g_pD3D = nullptr; }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ImGuiOverlayClass";
    RegisterClassExW(&wc);

    HWND hWnd = CreateWindowW(
        wc.lpszClassName,
        L"ImGui DX9 Overlay",
        WS_OVERLAPPEDWINDOW,
        100, 100, 1280, 720,
        nullptr, nullptr,
        hInstance, nullptr
    );

    if (!CreateDeviceD3D(hWnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, hInstance);
        return 1;
    }

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    overlay::init();

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (GetAsyncKeyState(VK_INSERT) & 1)
            overlay::visible = !overlay::visible;

        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(30, 30, 30, 255), 1.0f, 0);

        if (SUCCEEDED(g_pd3dDevice->BeginScene()))
        {
            overlay::render();
            g_pd3dDevice->EndScene();
        }

        g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
    }

    overlay::shutdown();
    CleanupDeviceD3D();
    DestroyWindow(hWnd);
    UnregisterClassW(wc.lpszClassName, hInstance);

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice && wParam != SIZE_MINIMIZED)
        {
            g_d3dpp.BackBufferWidth = LOWORD(lParam);
            g_d3dpp.BackBufferHeight = HIWORD(lParam);
            ImGui_ImplDX9_InvalidateDeviceObjects();
            g_pd3dDevice->Reset(&g_d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}