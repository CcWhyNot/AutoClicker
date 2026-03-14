#pragma once
#include <atomic>
#include <Windows.h>

constexpr int DELAY_MIN = 1;
constexpr int DELAY_MAX = 5000;

struct AutoclickerConfig {
    std::atomic<bool> activo   = false;
    std::atomic<int>  delay_ms = 100;
    std::atomic<int>  tipo     = 0;    // 0=izquierdo, 1=derecho
    std::atomic<bool> running  = true;
    std::atomic<int>  hotkey   = VK_F1;
};

void autoclicker_tick(AutoclickerConfig& config);
void autoclicker_hotkey(AutoclickerConfig& config);
