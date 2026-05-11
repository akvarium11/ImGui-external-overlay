#include "overlay.h"
#include "imgui.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../imgui/backends/imgui_impl_dx9.h"
#include <d3d9.h>

extern LPDIRECT3DDEVICE9 g_pd3dDevice;

namespace overlay {

    void init()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_Init(GetActiveWindow());
        ImGui_ImplDX9_Init(g_pd3dDevice);
    }

    void render()
    {
        if (!visible) return;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("ImGui Overlay", &visible, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Hello from ImGui DX9 Overlay!");
        ImGui::Separator();

        static float f = 0.0f;
        ImGui::SliderFloat("Float", &f, 0.0f, 1.0f);

        static int counter = 0;
        if (ImGui::Button("Button")) counter++;
        ImGui::SameLine();
        ImGui::Text("Counter = %d", counter);

        ImGui::Text("Press INSERT to toggle visibility");

        ImGui::End();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    void shutdown()
    {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

}