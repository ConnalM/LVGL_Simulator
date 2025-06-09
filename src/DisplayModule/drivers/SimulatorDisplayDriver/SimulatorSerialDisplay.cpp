#include "SimulatorSerialDisplay.h"
#include <cstdarg>
#include <cstdio>
#include <string>

// Forward declaration for log_message from main.cpp
extern void log_message(const char* format, ...);

SimulatorSerialDisplay::SimulatorSerialDisplay() : _initialized(false), _portName(""), _baudRate(0) {
}

SimulatorSerialDisplay::~SimulatorSerialDisplay() {
    if (_initialized) {
        SerialBridge::getInstance().close();
    }
}

bool SimulatorSerialDisplay::initialize() {
    if (_initialized) {
        return true;
    }
    
    // If port is not configured, just log messages without actual serial communication
    if (_portName.empty()) {
        log_message("SimulatorSerialDisplay: No port configured, will only log messages");
        _initialized = true;
        return true;
    }
    
    // Initialize the serial bridge
    if (SerialBridge::getInstance().initialize(_portName, _baudRate)) {
        _initialized = true;
        log_message("SimulatorSerialDisplay: Initialized on port %s at %d baud", _portName.c_str(), _baudRate);
        return true;
    } else {
        log_message("SimulatorSerialDisplay: Failed to initialize on port %s", _portName.c_str());
        return false;
    }
}

void SimulatorSerialDisplay::update() {
    // Process any incoming serial data
    if (_initialized && !_portName.empty()) {
        SerialBridge::getInstance().update();
    }
}

void SimulatorSerialDisplay::clear() {
    if (_initialized) {
        // Send a clear screen command (ANSI escape sequence)
        if (!_portName.empty()) {
            SerialBridge::getInstance().send("\033[2J\033[H");
        }
        log_message("SimulatorSerialDisplay: Clear screen");
    }
}

void SimulatorSerialDisplay::print(const String& message, bool newLine) {
    if (_initialized) {
        std::string stdMessage = message.c_str();
        
        if (newLine) {
            stdMessage += "\r\n";
        }
        
        if (!_portName.empty()) {
            SerialBridge::getInstance().send(stdMessage);
        }
        
        // Log the message
        log_message("SimulatorSerialDisplay: %s", stdMessage.c_str());
    }
}

void SimulatorSerialDisplay::printf(const char* format, ...) {
    if (_initialized) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        std::string message = buffer;
        
        if (!_portName.empty()) {
            SerialBridge::getInstance().send(message);
        }
        
        // Log the message
        log_message("SimulatorSerialDisplay: %s", message.c_str());
    }
}

DisplayType SimulatorSerialDisplay::getDisplayType() const {
    return DisplayType::Serial;
}

bool SimulatorSerialDisplay::configurePort(const std::string& portName, int baudRate) {
    _portName = portName;
    _baudRate = baudRate;
    
    // If already initialized, reinitialize with new settings
    if (_initialized) {
        SerialBridge::getInstance().close();
        return initialize();
    }
    
    return true;
}
