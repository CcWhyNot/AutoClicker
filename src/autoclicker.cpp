#include "autoclicker.h"
#include <SDL.h>
#include <Windows.h>
#include <algorithm>

void autoclicker_tick(AutoclickerConfig &config)
{
    if (!config.activo.load()) {
        SDL_Delay(10);
        return;
    }

    const int tipo     = config.tipo.load();
    const int delay_ms = std::clamp(config.delay_ms.load(), DELAY_MIN, DELAY_MAX);

    // mover cursor a la siguiente posición marcada (si está habilitado)
    if (config.usar_posiciones.load()) {
        std::lock_guard<std::mutex> lock(config.posiciones_mutex);
        if (!config.posiciones.empty()) {
            int n   = (int)config.posiciones.size();
            int idx = config.pos_index.load() % n;
            SetCursorPos(config.posiciones[idx].x, config.posiciones[idx].y);
            config.pos_index.store((idx + 1) % n);
        }
    }

    INPUT input = {};
    input.type = INPUT_MOUSE;

    input.mi.dwFlags = (tipo == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    input.mi.dwFlags = (tipo == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));

    // restaurar cursor si freeze está activo (solo cuando NO se usan posiciones)
    if (config.freeze_mouse.load() && !config.usar_posiciones.load()) {
        SetCursorPos(config.freeze_x.load(), config.freeze_y.load());
    }

    SDL_Delay(delay_ms);
}

void autoclicker_hotkey(AutoclickerConfig &config)
{
    if (config.asignando_hotkey.load()) return;

    if (GetAsyncKeyState(config.hotkey.load()) & 0x0001) {
        bool nuevo = !config.activo.load();

        // guardar posición del cursor al activar (para freeze y para reset de índice)
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
