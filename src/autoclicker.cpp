#include "autoclicker.h"
#include <SDL.h>
#include <Windows.h>
#include <algorithm>

void autoclicker_tick(AutoclickerConfig &config)
{
    if (!config.activo.load()) {
        SDL_Delay(10);  // reposo cuando inactivo, evita spinloop
        return;
    }

    // leer atómicos una sola vez para consistencia durante el tick
    const int tipo     = config.tipo.load();
    const int delay_ms = std::clamp(config.delay_ms.load(), DELAY_MIN, DELAY_MAX);

    INPUT input = {0};
    input.type = INPUT_MOUSE;

    input.mi.dwFlags = (tipo == 0) ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    input.mi.dwFlags = (tipo == 0) ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));

    SDL_Delay(delay_ms);
}

void autoclicker_hotkey(AutoclickerConfig &config)
{
    if (GetAsyncKeyState(VK_F1) & 0x0001)
        config.activo = !config.activo.load();
}
