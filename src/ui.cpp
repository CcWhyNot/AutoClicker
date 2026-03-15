#include "ui.h"
#include "imgui.h"
#include <SDL.h>
#include <algorithm>
#include <mutex>
#include <string>
#include <Windows.h>

static std::string vk_to_name(int vk) {
    UINT scan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    // marcar teclas extendidas para que GetKeyNameTextA devuelva el nombre correcto
    // (Insert, Delete, flechas, Ctrl/Alt derecho, etc.)
    static const int extended[] = {
        VK_INSERT, VK_DELETE, VK_HOME, VK_END,
        VK_PRIOR, VK_NEXT, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN,
        VK_RCONTROL, VK_RMENU, VK_DIVIDE, VK_NUMLOCK
    };
    for (int e : extended) {
        if (vk == e) { scan |= 0x100; break; }
    }
    char name[64] = {};
    if (GetKeyNameTextA((LONG)(scan << 16), name, sizeof(name)))
        return name;
    return "?";
}

void ui_render(AutoclickerConfig& config, SDL_Window* window) {
    static bool esperando_tecla = false;
    static int  frames_espera   = 0;
    static bool capturando_pos  = false;
    static int  frames_captura  = 0;

    if (esperando_tecla) {
        if (frames_espera > 0) {
            frames_espera--;
        } else {
            for (int vk = 0x08; vk <= 0xFE; vk++) {
                if (GetAsyncKeyState(vk) & 0x8000) {
                    config.hotkey.store(vk);
                    config.asignando_hotkey.store(false);
                    esperando_tecla = false;
                    break;
                }
            }
        }
    }

    if (capturando_pos) {
        frames_captura--;
        if (frames_captura <= 0) {
            POINT p;
            GetCursorPos(&p);
            {
                std::lock_guard<std::mutex> lock(config.posiciones_mutex);
                config.posiciones.push_back(p);
            }
            capturando_pos = false;
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

    // --- título + créditos ---
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
    ImGui::TextColored(ImVec4(0.35f, 0.55f, 0.95f, 1.0f), "AutoClicker");
    ImGui::SameLine();
    ImGui::TextDisabled("by CcWhyNot");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("github.com/CcWhyNot");

    // --- indicador de estado (derecha) ---
    bool        prog_on    = config.programado.load();
    const char* estado_txt = prog_on  ? "PROGRAMADO"
                           : activo   ? "ACTIVO"
                                      : "INACTIVO";
    ImVec4      estado_col = prog_on  ? ImVec4(0.9f, 0.6f, 0.1f, 1.0f)
                           : activo   ? ImVec4(0.2f, 0.8f, 0.3f, 1.0f)
                                      : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    float txt_w = ImGui::CalcTextSize(estado_txt).x;

    // botón "?" antes del indicador
    ImGui::SameLine((float)w - txt_w - 16 - 30);
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.08f));
    if (ImGui::SmallButton("?"))
        ImGui::OpenPopup("##acerca");
    ImGui::PopStyleColor(2);

    ImGui::SameLine((float)w - txt_w - 16);
    ImGui::TextColored(estado_col, "%s", estado_txt);

    // --- modal Acerca de ---
    ImGui::SetNextWindowSize(ImVec2(420, 340), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((float)w / 2, (float)h / 2),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("##acerca", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

        ImGui::TextColored(ImVec4(0.35f, 0.55f, 0.95f, 1.0f), "AutoClicker");
        ImGui::SameLine();
        ImGui::TextDisabled("by CcWhyNot");
        ImGui::SameLine();
        ImGui::TextDisabled("— github.com/CcWhyNot");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::BeginChild("##licenses", ImVec2(0, -36), true);

        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "SDL2 (zlib license)");
        ImGui::TextWrapped(
            "Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>\n\n"
            "This software is provided 'as-is', without any express or implied warranty. "
            "In no event will the authors be held liable for any damages arising from the "
            "use of this software.\n\n"
            "Permission is granted to anyone to use this software for any purpose, including "
            "commercial applications, and to alter it and redistribute it freely, subject to "
            "the following restrictions:\n\n"
            "1. The origin of this software must not be misrepresented; you must not claim "
            "that you wrote the original software.\n"
            "2. Altered source versions must be plainly marked as such, and must not be "
            "misrepresented as being the original software.\n"
            "3. This notice may not be removed or altered from any source distribution."
        );

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.85f, 1.0f, 1.0f), "Dear ImGui (MIT license)");
        ImGui::TextWrapped(
            "Copyright (c) 2014-2024 Omar Cornut\n\n"
            "Permission is hereby granted, free of charge, to any person obtaining a copy "
            "of this software and associated documentation files (the \"Software\"), to deal "
            "in the Software without restriction, including without limitation the rights to "
            "use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies "
            "of the Software, and to permit persons to whom the Software is furnished to do "
            "so, subject to the following conditions:\n\n"
            "The above copyright notice and this permission notice shall be included in all "
            "copies or substantial portions of the Software.\n\n"
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
            "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
            "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT."
        );

        ImGui::EndChild();

        ImGui::Spacing();
        float close_w = 80;
        ImGui::SetCursorPosX((420 - close_w) / 2);
        if (ImGui::Button("Cerrar", ImVec2(close_w, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

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
            config.asignando_hotkey.store(true);
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

    // --- freeze mouse ---
    bool freeze = config.freeze_mouse.load();
    if (ImGui::Checkbox("Freezear ratón al activar", &freeze))
        config.freeze_mouse.store(freeze);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Mantiene el cursor en la posición donde\nse activó el autoclicker.");

    ImGui::Spacing();

    // --- posiciones marcadas ---
    bool usar_pos = config.usar_posiciones.load();
    if (ImGui::Checkbox("Usar posiciones marcadas", &usar_pos)) {
        if (!usar_pos) capturando_pos = false; // cancelar captura pendiente al desactivar
        config.usar_posiciones.store(usar_pos);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("El autoclicker cicla por los puntos marcados\nen lugar de hacer click en el cursor.");

    if (usar_pos) {
        ImGui::Indent(12.0f);
        ImGui::Spacing();

        // lista de posiciones
        {
            std::lock_guard<std::mutex> lock(config.posiciones_mutex);
            int to_remove = -1;
            if (config.posiciones.empty()) {
                ImGui::TextDisabled("(sin posiciones)");
            } else {
                for (int i = 0; i < (int)config.posiciones.size(); i++) {
                    ImGui::PushID(i);
                    ImGui::Text("%d.  X:%-5d Y:%d", i + 1,
                        config.posiciones[i].x, config.posiciones[i].y);
                    ImGui::SameLine();
                    if (ImGui::SmallButton("X"))
                        to_remove = i;
                    ImGui::PopID();
                }
            }
            if (to_remove >= 0)
                config.posiciones.erase(config.posiciones.begin() + to_remove);
        }

        ImGui::Spacing();

        // botón de captura con cuenta atrás
        if (capturando_pos) {
            int secs = (frames_captura + 59) / 60;
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                "Mueve el ratón...  %ds", secs);
        } else {
            if (ImGui::SmallButton("Capturar posición (3s)")) {
                capturando_pos = true;
                frames_captura = 3 * 60;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Limpiar todo")) {
                std::lock_guard<std::mutex> lock(config.posiciones_mutex);
                config.posiciones.clear();
                config.pos_index.store(0);
            }
        }

        ImGui::Unindent(12.0f);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- modo programado ---
    {
        static int unidad = 0; // 0=segundos, 1=minutos

        bool prog = config.programado.load();
        if (activo) ImGui::BeginDisabled();
        if (ImGui::Checkbox("Modo programado", &prog)) {
            if (prog) {
                config.activo.store(false);    // apagar manual al activar programado
                config.prox_run_tick.store(0);
            }
            config.programado.store(prog);
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Ejecuta el patrón de posiciones automáticamente\ncada X segundos/minutos.");
        if (activo) ImGui::EndDisabled();

        if (prog) {
            ImGui::Indent(12.0f);
            ImGui::Spacing();

            // intervalo
            int intervalo_raw = config.intervalo_seg.load();
            int intervalo_ui  = (unidad == 1) ? intervalo_raw / 60 : intervalo_raw;

            ImGui::Text("Cada");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            if (ImGui::InputInt("##intervalo", &intervalo_ui)) {
                intervalo_ui = std::clamp(intervalo_ui, 1, (unidad == 1) ? 60 : 3600);
                config.intervalo_seg.store((unidad == 1) ? intervalo_ui * 60 : intervalo_ui);
                config.prox_run_tick.store(0); // reiniciar timer al cambiar intervalo
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90);
            const char* unidades[] = {"segundos", "minutos"};
            if (ImGui::Combo("##unidad", &unidad, unidades, 2)) {
                // al cambiar unidad ajustar el valor mostrado (el almacenado no cambia)
            }

            // pases
            int pases = config.num_pases.load();
            ImGui::Text("Pases");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(70);
            if (ImGui::InputInt("##pases", &pases)) {
                pases = std::clamp(pases, 1, 100);
                config.num_pases.store(pases);
            }

            // countdown
            ImGui::Spacing();
            uint32_t prox  = config.prox_run_tick.load();
            uint32_t ahora = (uint32_t)SDL_GetTicks();
            if (prox == 0) {
                ImGui::TextDisabled("Esperando primer tick...");
            } else if (prox > ahora) {
                uint32_t ms = prox - ahora;
                if (ms >= 1000)
                    ImGui::TextColored(ImVec4(0.5f, 0.85f, 1.0f, 1.0f),
                        "Proxima ejecucion en: %us", ms / 1000u);
                else
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Ejecutando pronto...");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Ejecutando...");
            }

            ImGui::Unindent(12.0f);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // --- activar ---
    float btn_w = (float)w - 32;
    bool prog_activo = config.programado.load();
    if (prog_activo) ImGui::BeginDisabled();
    if (activo) {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.55f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.65f, 0.25f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.25f, 0.45f, 0.80f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.55f, 0.95f, 1.0f));
    }
    if (ImGui::Button(activo ? "DETENER" : "INICIAR", ImVec2(btn_w, 40))) {
        bool nuevo = !activo;
        if (nuevo) {
            if (config.freeze_mouse.load()) {
                POINT p;
                GetCursorPos(&p);
                config.freeze_x.store(p.x);
                config.freeze_y.store(p.y);
            }
            config.pos_index.store(0);
        }
        config.activo.store(nuevo);
    }
    ImGui::PopStyleColor(2);
    if (prog_activo) ImGui::EndDisabled();

    ImGui::End();
}
