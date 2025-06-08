#include "SerialDisplay.h"
#include <stdarg.h>
#include "DisplayManager.h"

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        static bool firstCall = true; \
        unsigned long now = millis(); \
        if (firstCall || (now - lastDebugPrint > DEBUG_THROTTLE_MS)) { \
            DisplayManager::getInstance().debug(String("[SerialDisplay] ") + __FUNCTION__, "SerialDisplay"); \
            lastDebugPrint = now; \
            firstCall = false; \
        } \
    } while(0)

SerialDisplay::SerialDisplay()
    : _initialized(false)
    , _width(80)  // Default width for serial console
    , _height(24) // Default height for serial console
{
    DEBUG_PRINT_METHOD();
    // Constructor implementation
}

SerialDisplay::~SerialDisplay() {
    DEBUG_PRINT_METHOD();
    // Destructor implementation
}

bool SerialDisplay::initialize() {
    DEBUG_PRINT_METHOD();
    // Skip if already initialized
    if (_initialized) {
        DisplayManager::getInstance().debug(F("Already initialized"), "SerialDisplay");
        return true;
    }
    
    DisplayManager::getInstance().debug(F("Initializing..."), "SerialDisplay");
    
    // Serial is already initialized in setup(), so we just need to set the flag
    _initialized = true;
    
    DisplayManager::getInstance().debug(F("Initialized successfully"), "SerialDisplay");
    return true;
}

void SerialDisplay::update() {
    DEBUG_PRINT_METHOD();
    // No-op for serial display
}

void SerialDisplay::clear() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Clear serial by printing newlines
    for (int i = 0; i < 20; i++) {
        Serial.println();
    }
}

void SerialDisplay::print(const String& message, bool newLine) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Print to serial directly
    if (newLine) {
        Serial.println(message);
    } else {
        Serial.print(message);
    }
}

void SerialDisplay::printf(const char* format, ...) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Format the string
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Print to serial directly
    Serial.print(buffer);
}

int SerialDisplay::getWidth() const {
    DEBUG_PRINT_METHOD();
    return _width;
}

int SerialDisplay::getHeight() const {
    DEBUG_PRINT_METHOD();
    return _height;
}

DisplayType SerialDisplay::getDisplayType() const {
    DEBUG_PRINT_METHOD();
    return DisplayType::Serial;
}
