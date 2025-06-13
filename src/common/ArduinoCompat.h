#pragma once

#ifdef SIMULATOR
// Include standard C++ headers for simulator
#include <string>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <cstdint>
#include <SDL.h>

// Arduino String compatibility
// First, check if String is already defined as a typedef
#ifndef String

// Define String as a class with Arduino-like behavior
class String : public std::string {
public:
    // Default constructors
    String() : std::string() {}
    String(const char* str) : std::string(str ? str : "") {}
    String(const std::string& str) : std::string(str) {}
    String(char c) : std::string(1, c) {}
    
    // Integer constructors
    String(int num) : std::string(std::to_string(num)) {}
    String(unsigned int num) : std::string(std::to_string(num)) {}
    String(long num) : std::string(std::to_string(num)) {}
    String(unsigned long num) : std::string(std::to_string(num)) {}
    String(long long num) : std::string(std::to_string(num)) {}
    String(unsigned long long num) : std::string(std::to_string(num)) {}
    
    // Float constructors
    String(float num, int digits = 2) : std::string(std::to_string(num)) {}
    String(double num, int digits = 2) : std::string(std::to_string(num)) {}
    
    // Arduino compatibility methods
    bool isEmpty() const { return empty(); }
};

// Arduino-compatible functions for simulator

// Time functions
uint32_t millis();
uint32_t micros();
void delay(uint32_t ms);
void delayMicroseconds(uint32_t us);

// Math functions
long map(long x, long in_min, long in_max, long out_min, long out_max);
long constrain(long amt, long low, long high);

// Add Arduino-like String functions to std::string
namespace ArduinoCompat {
    // Convert various types to string
    inline std::string toString(int num) { return std::to_string(num); }
    inline std::string toString(unsigned int num) { return std::to_string(num); }
    inline std::string toString(long num) { return std::to_string(num); }
    inline std::string toString(unsigned long num) { return std::to_string(num); }
    inline std::string toString(long long num) { return std::to_string(num); }
    inline std::string toString(unsigned long long num) { return std::to_string(num); }
    inline std::string toString(float num, int digits = 2) { return std::to_string(num); }
    inline std::string toString(double num, int digits = 2) { return std::to_string(num); }
    
    // Handle any size_type (for vector::size(), etc.)
    template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
    inline std::string toString(T num) { return std::to_string(static_cast<long long>(num)); }
    
    // Check if string is empty (Arduino compatibility)
    inline bool isEmpty(const std::string& str) { return str.empty(); }
}

// Extend std::string with Arduino-like constructors
inline String operator+(const String& lhs, int rhs) {
    return lhs + ArduinoCompat::toString(rhs);
}

inline String operator+(const String& lhs, unsigned int rhs) {
    return lhs + ArduinoCompat::toString(rhs);
}

inline String operator+(const String& lhs, long rhs) {
    return lhs + ArduinoCompat::toString(rhs);
}

inline String operator+(const String& lhs, unsigned long rhs) {
    return lhs + ArduinoCompat::toString(rhs);
}

#endif // String not defined

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

// Define F() macro for simulator (just returns the string as-is)
#define F(x) x

// Serial HAL: Provide extern Serial object for both simulator and production
#ifdef SIMULATOR
    #include "Sim/TerminalSerial.h"
    extern TerminalSerial Serial;
#else
    #include <HardwareSerial.h>
    extern HardwareSerial Serial;
#endif

// Add other Arduino compatibility as needed

#else
// Use actual Arduino for ESP32 builds
#include <Arduino.h>
#endif
