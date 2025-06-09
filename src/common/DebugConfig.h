#pragma once

// Debug configuration for the entire project

// Debug levels
#define DEBUG_LEVEL_NONE   0
#define DEBUG_LEVEL_ERROR  1
#define DEBUG_LEVEL_WARN   2
#define DEBUG_LEVEL_INFO   3
#define DEBUG_LEVEL_DEBUG  4
#define DEBUG_LEVEL_VERBOSE 5

// Current debug level (change this to control verbosity)
#ifndef CURRENT_DEBUG_LEVEL
#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_WARN  // Reduced from INFO to WARN by default
#endif

// Debug macros
#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_WARN
#define LOG_WARN(fmt, ...) Serial.printf("[WARN]  " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_WARN(fmt, ...)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define LOG_INFO(fmt, ...) Serial.printf("[INFO]  " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) Serial.printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
#define LOG_VERBOSE(fmt, ...) Serial.printf("[VERBOSE] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_VERBOSE(fmt, ...)
#endif

// Rate-limited logging (prints once every N calls)
#define RATE_LIMITED_LOG(level, interval, fmt, ...) \
do { \
    static uint32_t counter = 0; \
    if (counter++ % interval == 0) { \
        level(fmt, ##__VA_ARGS__); \
    } \
} while(0)
