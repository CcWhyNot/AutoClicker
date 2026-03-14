#pragma once
#include <atomic>
#include <mutex>
#include <vector>
#include <Windows.h>

constexpr int DELAY_MIN = 1;
constexpr int DELAY_MAX = 5000;

struct AutoclickerConfig {
    std::atomic<bool> activo           = false;
    std::atomic<int>  delay_ms         = 100;
    std::atomic<int>  tipo             = 0;    // 0=izquierdo, 1=derecho
    std::atomic<bool> running          = true;
    std::atomic<int>  hotkey            = VK_F1;
    std::atomic<bool> asignando_hotkey  = false; // suspende la hotkey mientras se reasigna

    // mouse freeze al activar
    std::atomic<bool> freeze_mouse     = false;
    std::atomic<int>  freeze_x         = 0;
    std::atomic<int>  freeze_y         = 0;

    // posiciones marcadas
    std::atomic<bool> usar_posiciones  = false;
    std::atomic<int>  pos_index        = 0;
    std::vector<POINT> posiciones;
    std::mutex         posiciones_mutex;
};

void autoclicker_tick(AutoclickerConfig& config);
void autoclicker_hotkey(AutoclickerConfig& config);
