#pragma once

#ifdef SIMULATOR
#include "ArduinoCompat.h"
#else
#include <Arduino.h>
#endif

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

// Define DEBUG_LEVEL: 0=None, 1=Error, 2=Warning, 3=Info, 4=Debug
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 4 // Default to highest level if not set externally
#endif

#if DEBUG_LEVEL > 0
#ifdef SIMULATOR
// In simulator, use printf/cout instead of Serial
#include <iostream>
#include <cstdio>
#define DPRINT(x)    std::cout << x
#define DPRINTLN(x) std::cout << x << std::endl
#define DPRINTF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
// In production, use Arduino Serial
#define DPRINT(x)    Serial.print(x)
#define DPRINTLN(x) Serial.println(x)
#define DPRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#endif
#else
#define DPRINT(x)
#define DPRINTLN(fmt, ...)
#define DPRINTF(fmt, ...)
#endif

#if DEBUG_LEVEL >= 1 // Error messages
#ifdef SIMULATOR
#define DEBUG_ERROR(x)   std::cout << "[ERROR] " << x << std::endl
#else
#define DEBUG_ERROR(x)   Serial.print(F("[ERROR] ")); Serial.println(x)
#endif
#else
#define DEBUG_ERROR(x)
#endif

#if DEBUG_LEVEL >= 2 // Warning messages
#ifdef SIMULATOR
#define DEBUG_WARN(x)    std::cout << "[WARN]  " << x << std::endl
#else
#define DEBUG_WARN(x)    Serial.print(F("[WARN]  ")); Serial.println(x)
#endif
#else
#define DEBUG_WARN(x)
#endif

#if DEBUG_LEVEL >= 3 // Info messages
#ifdef SIMULATOR
#define DEBUG_INFO(x)    std::cout << "[INFO]  " << x << std::endl
#else
#define DEBUG_INFO(x)    Serial.print(F("[INFO]  ")); Serial.println(x)
#endif
#else
#define DEBUG_INFO(x)
#endif

#if DEBUG_LEVEL >= 4 // Debug messages
#ifdef SIMULATOR
#define DEBUG_DETAIL(x)  std::cout << "[DEBUG] " << x << std::endl
#else
#define DEBUG_DETAIL(x)  Serial.print(F("[DEBUG] ")); Serial.println(x)
#endif
#else
#define DEBUG_DETAIL(x)
#endif

#endif // DEBUG_UTILS_H

