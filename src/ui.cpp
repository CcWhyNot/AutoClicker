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

void ui_render(AutoclickerConfig& config) {
    static bool esperando_tecla = false;
    static int  frames_espera   = 0;

    // cuando esperando, escanea todas las teclas
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

    bool activo   = config.activo.load();
    int  delay_ms = config.delay_ms.load();
    int  tipo     = config.tipo.load();

    ImGui::Begin("AutoClicker");

        // hotkey configurable
        std::string nombre_tecla = vk_to_name(config.hotkey.load());
        if (esperando_tecla) {
            ImGui::TextColored(ImVec4(1,1,0,1), "Pulsa cualquier tecla...");
        } else {
            ImGui::Text("Hotkey: %s", nombre_tecla.c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Cambiar")) {
                esperando_tecla = true;
                frames_espera   = 5; // espera 5 frames para evitar registrar el click
            }
        }
        ImGui::Separator();

        if (ImGui::Checkbox("Activar", &activo))
            config.activo.store(activo);

        if (ImGui::InputInt("Delay (ms)", &delay_ms)) {
            delay_ms = std::clamp(delay_ms, DELAY_MIN, DELAY_MAX);
            config.delay_ms.store(delay_ms);
        }

        const char* tipos[] = {"Click izquierdo", "Click derecho"};
        if (ImGui::Combo("Tipo", &tipo, tipos, 2))
            config.tipo.store(tipo);

        ImGui::Separator();
        ImGui::TextColored(
            activo ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1),
            activo ? "Estado: ACTIVO" : "Estado: INACTIVO"
        );

    ImGui::End();
}
