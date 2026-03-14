#include "autoclicker.h"
#include <SDL.h>
#include <Windows.h>
#include <algorithm>
#include <vector>

static void hacer_click(int tipo) {
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = (tipo == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));
    input.mi.dwFlags = (tipo == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void autoclicker_tick(AutoclickerConfig &config)
{
    // --- modo programado ---
    if (config.programado.load()) {
        uint32_t ahora = (uint32_t)SDL_GetTicks();
        uint32_t prox  = config.prox_run_tick.load();

        if (prox == 0) {
            // primer frame: programar primera ejecución
            config.prox_run_tick.store(ahora + (uint32_t)config.intervalo_seg.load() * 1000u);
            SDL_Delay(10);
            return;
        }

        if (ahora < prox) {
            SDL_Delay(10);
            return;
        }

        // es hora — copiar posiciones bajo lock y soltar antes de ejecutar
        const int tipo     = config.tipo.load();
        const int delay_ms = std::clamp(config.delay_ms.load(), DELAY_MIN, DELAY_MAX);
        const int pases    = std::clamp(config.num_pases.load(), 1, 100);

        std::vector<POINT> pts;
        {
            std::lock_guard<std::mutex> lock(config.posiciones_mutex);
            pts = config.posiciones;
        }

        if (pts.empty()) {
            hacer_click(tipo);
        } else {
            for (int p = 0; p < pases && config.programado.load(); p++) {
                for (int i = 0; i < (int)pts.size() && config.programado.load(); i++) {
                    SetCursorPos(pts[i].x, pts[i].y);
                    hacer_click(tipo);
                    SDL_Delay(delay_ms);
                }
            }
        }

        config.prox_run_tick.store(
            (uint32_t)SDL_GetTicks() + (uint32_t)config.intervalo_seg.load() * 1000u);
        return;
    }

    // --- modo normal ---
    if (!config.activo.load()) {
        SDL_Delay(10);
        return;
    }

    const int tipo     = config.tipo.load();
    const int delay_ms = std::clamp(config.delay_ms.load(), DELAY_MIN, DELAY_MAX);

    if (config.usar_posiciones.load()) {
        std::lock_guard<std::mutex> lock(config.posiciones_mutex);
        if (!config.posiciones.empty()) {
            int n   = (int)config.posiciones.size();
            int idx = config.pos_index.load() % n;
            SetCursorPos(config.posiciones[idx].x, config.posiciones[idx].y);
            config.pos_index.store((idx + 1) % n);
        }
    }

    hacer_click(tipo);

    if (config.freeze_mouse.load() && !config.usar_posiciones.load()) {
        SetCursorPos(config.freeze_x.load(), config.freeze_y.load());
    }

    SDL_Delay(delay_ms);
}

void autoclicker_hotkey(AutoclickerConfig &config)
{
    if (config.asignando_hotkey.load()) return;
    if (config.programado.load())       return; // modo programado: hotkey deshabilitada

    if (GetAsyncKeyState(config.hotkey.load()) & 0x0001) {
        bool nuevo = !config.activo.load();

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
}
