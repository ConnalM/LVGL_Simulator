#pragma once

#ifdef NATIVE_BUILD
#include "../ArduinoCompat.h"
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
#define DPRINT(x)    Serial.print(x)
#define DPRINTLN(x) Serial.println(x)
#define DPRINTF(fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define DPRINT(x)
#define DPRINTLN(fmt, ...)
#define DPRINTF(fmt, ...)
#endif

#if DEBUG_LEVEL >= 1 // Error messages
#define DEBUG_ERROR(x)   Serial.print(F("[ERROR] ")); Serial.println(x)
#else
#define DEBUG_ERROR(x)
#endif

#if DEBUG_LEVEL >= 2 // Warning messages
#define DEBUG_WARN(x)    Serial.print(F("[WARN]  ")); Serial.println(x)
#else
#define DEBUG_WARN(x)
#endif

#if DEBUG_LEVEL >= 3 // Info messages
#define DEBUG_INFO(x)    Serial.print(F("[INFO]  ")); Serial.println(x)
#else
#define DEBUG_INFO(x)
#endif

#if DEBUG_LEVEL >= 4 // Debug messages
#define DEBUG_DETAIL(x)  Serial.print(F("[DEBUG] ")); Serial.println(x)
#else
#define DEBUG_DETAIL(x)
#endif

#endif // DEBUG_UTILS_H

