#include "ui.h"
#include "imgui.h"
#include <algorithm>
#include <string>
#include <Windows.h>

static std::string vk_to_name(int vk) {
    UINT scan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    char name[64] = {};
    if (GetKeyNameTextA((LONG)(scan << 16), name, sizeof(name)))
        return name;
    return "?";
}

void ui_render(AutoclickerConfig& config, SDL_Window* window) {
    static bool esperando_tecla = false;
    static int  frames_espera   = 0;

    if (esperando_tecla) {
        if (frames_espera > 0) {
            frames_espera--;
        } else {
            for (int vk = 0x08; vk <= 0xFE; vk++) {
                if (GetAsyncKeyState(vk) & 0x8000) {
                    config.hotkey.store(vk);
                    esperando_tecla = false;
                    break;
                }
            }
        }
    }

    // ventana ImGui ocupa toda la ventana SDL
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)w, (float)h));
    ImGui::Begin("##main", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_NoMove     |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    bool activo   = config.activo.load();
    int  delay_ms = config.delay_ms.load();
    int  tipo     = config.tipo.load();

    // --- título ---
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
    ImGui::TextColored(ImVec4(0.35f, 0.55f, 0.95f, 1.0f), "AutoClicker");
    ImGui::Separator();
    ImGui::Spacing();

    // --- hotkey ---
    ImGui::Text("Hotkey");
    ImGui::SameLine(100);
    if (esperando_tecla) {
        ImGui::TextColored(ImVec4(1,1,0,1), "Pulsa una tecla...");
    } else {
        std::string nombre = vk_to_name(config.hotkey.load());
        ImGui::Text("%s", nombre.c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton("Cambiar")) {
            esperando_tecla = true;
            frames_espera   = 5;
        }
    }

    ImGui::Spacing();

    // --- delay ---
    ImGui::Text("Delay (ms)");
    ImGui::SameLine(100);
    ImGui::SetNextItemWidth(120);
    if (ImGui::InputInt("##delay", &delay_ms)) {
        delay_ms = std::clamp(delay_ms, DELAY_MIN, DELAY_MAX);
        config.delay_ms.store(delay_ms);
    }

    ImGui::Spacing();

    // --- tipo de click ---
    ImGui::Text("Tipo");
    ImGui::SameLine(100);
    ImGui::SetNextItemWidth(150);
    const char* tipos[] = {"Click izquierdo", "Click derecho"};
    if (ImGui::Combo("##tipo", &tipo, tipos, 2))
        config.tipo.store(tipo);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- activar ---
    float btn_w = (float)w - 32;
    if (activo) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.55f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.25f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.25f, 0.45f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.55f, 0.95f, 1.0f));
    }
    if (ImGui::Button(activo ? "DETENER" : "INICIAR", ImVec2(btn_w, 40))) {
        config.activo.store(!activo);
    }
    ImGui::PopStyleColor(2);

    ImGui::End();
}
