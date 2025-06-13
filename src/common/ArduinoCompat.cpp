#ifdef SIMULATOR
#include "ArduinoCompat.h"
#include <chrono>
#include <thread>
#include <algorithm>

// Time functions implementation
static auto start_time = std::chrono::high_resolution_clock::now();

uint32_t millis() {
    auto now = std::chrono::high_resolution_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count());
}

uint32_t micros() {
    auto now = std::chrono::high_resolution_clock::now();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count());
}

void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(uint32_t us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// Math functions implementation
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long constrain(long amt, long low, long high) {
    return std::max(low, std::min(amt, high));
}

#endif // SIMULATOR
