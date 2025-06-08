#pragma once

#ifdef SIMULATOR
// Provide Arduino-like functionality for the simulator
#include <string>
#include <cstdint>
#include <SDL.h>

// Define String class similar to Arduino's
typedef std::string String;

// Define common Arduino types
typedef uint8_t byte;
typedef bool boolean;

// Define common Arduino functions
#define millis() SDL_GetTicks()
#define delay(ms) SDL_Delay(ms)

// Define common Arduino constants
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Add printf-like functionality for String
namespace std {
    inline string to_string(const char* format, ...) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        return std::string(buffer);
    }
}

// Add other Arduino compatibility as needed

#else
// Use actual Arduino for ESP32 builds
#include <Arduino.h>
#endif
