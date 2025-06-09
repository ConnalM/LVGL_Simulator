#pragma once

// =============================================
// Main Module Toggles - Uncomment to enable
// =============================================
#define ENABLE_CONFIGMODULE    // Configuration management
#define ENABLE_RACEMODULE      // Race timing and logic
#define ENABLE_LIGHTSMODULE    // Light control system


// =============================================
// Input Submodules

// =============================================
#define ENABLE_INPUT_KEYBOARD  // Keyboard input support
// #define ENABLE_INPUT_BUTTON   // Physical button support
// #define ENABLE_INPUT_SENSOR   // Sensor input support
#define ENABLE_INPUT_TOUCH     // Touch input support

// =============================================
// Output Channels

// =============================================
#define ENABLE_OUTPUT_SERIAL  // Serial console output
#define ENABLE_OUTPUT_LCD      // LCD display output
// #define ENABLE_OUTPUT_WEB     // Web interface output




















// =============================================
// Automatic Dependency Resolution
// =============================================

// If any input submodule is enabled, enable the input module
#if defined(ENABLE_INPUT_KEYBOARD) || defined(ENABLE_INPUT_BUTTON) || \
    defined(ENABLE_INPUT_SENSOR) || defined(ENABLE_INPUT_TOUCH)
    #undef ENABLE_INPUTMODULE
    #define ENABLE_INPUTMODULE
#endif

// If any output channel is enabled, enable the display module
#if defined(ENABLE_OUTPUT_SERIAL) || defined(ENABLE_OUTPUT_LCD) || \
    defined(ENABLE_OUTPUT_WEB)
    #undef ENABLE_DISPLAYMODULE
    #define ENABLE_DISPLAYMODULE
#endif
