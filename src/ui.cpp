#include "ui.h"
#include "imgui.h"
#include <algorithm>

void ui_render(AutoclickerConfig& config) {
    // ImGui necesita referencias a tipos normales, no atomic
    bool activo   = config.activo.load();
    int  delay_ms = config.delay_ms.load();
    int  tipo     = config.tipo.load();

    ImGui::Begin("AutoClicker");

        ImGui::Text("Hotkey: F1");
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
