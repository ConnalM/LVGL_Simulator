#pragma once

#ifdef NATIVE_BUILD
#include "../ArduinoCompat.h"
#else
#include <Arduino.h>
#endif

#include "common/TimeManager.h"
#include "common/Types.h"

/**
 * @brief Template for standardized module implementation
 * 
 * This template demonstrates the standardized module pattern with:
 * - Consistent initialization method
 * - Singleton pattern
 * - Standardized error handling
 * - Proper use of TimeManager for timing
 * 
 * When implementing a new module, copy this template and replace
 * "ModuleTemplate" with your module name.
 */
class ModuleTemplate {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return ModuleTemplate& The singleton instance
     */
    static ModuleTemplate& getInstance() {
        if (_instance == nullptr) {
            _instance = new ModuleTemplate();
        }
        return *_instance;
    }
    
    /**
     * @brief Initialize the module
     * 
     * This should be called once during system startup.
     * Initializes all required resources and dependencies.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize() {
        // Check if already initialized
        if (_initialized) {
            Serial.println(F("ModuleTemplate: Already initialized"));
            return true;
        }
        
        Serial.println(F("ModuleTemplate: Initializing..."));
        
        // Initialize module-specific resources
        // ...
        
        // Example of error handling - replace 'false' with your actual condition
        // For example: if (!myHardware.begin()) or if (someValue != expectedValue)
        if (false /* Replace with actual initialization failure condition */) {
            Serial.println(F("ModuleTemplate: Initialization failed"));
            return false;
        }
        
        _initialized = true;
        Serial.println(F("ModuleTemplate: Initialized successfully"));
        return true;
    }
    
    /**
     * @brief Update the module state
     * 
     * This should be called regularly in the main loop.
     * Handles regular processing and state updates.
     */
    void update() {
        // Skip if not initialized
        if (!_initialized) {
            return;
        }
        
        // Example of using TimeManager for timing
        unsigned long currentTime = TimeManager::GetInstance().GetCurrentTimeMs();
        
        // Only perform certain actions at specific intervals
        if (currentTime - _lastUpdateTime >= _updateIntervalMs) {
            _lastUpdateTime = currentTime;
            
            // Perform periodic updates
            // ...
        }
        
        // Regular processing
        // ...
    }
    
    /**
     * @brief Process a command or event
     * 
     * Example of standardized command processing.
     * 
     * @param command The command to process
     * @param value Optional parameter value
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo processCommand(int command, int value = 0) {
        // Skip if not initialized
        if (!_initialized) {
            return ErrorInfo(ErrorCode::NOT_INITIALIZED, "Module not initialized", "ModuleTemplate");
        }
        
        // Process command
        switch (command) {
            case 1:
                // Handle command 1
                break;
                
            case 2:
                // Handle command 2
                break;
                
            default:
                return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Unknown command", "ModuleTemplate");
        }
        
        return ErrorInfo(); // Success
    }
    
    // Module-specific methods
    // ...

private:
    /**
     * @brief Private constructor (Singleton pattern)
     */
    ModuleTemplate() 
        : _initialized(false)
        , _lastUpdateTime(0)
        , _updateIntervalMs(100) {
        // Constructor implementation
    }
    
    /**
     * @brief Private copy constructor (Singleton pattern)
     */
    ModuleTemplate(const ModuleTemplate&) = delete;
    
    /**
     * @brief Private assignment operator (Singleton pattern)
     */
    ModuleTemplate& operator=(const ModuleTemplate&) = delete;
    
    // Static instance for singleton pattern
    static ModuleTemplate* _instance;
    
    // Flag to track if initialization has been performed
    bool _initialized;
    
    // Timing variables
    unsigned long _lastUpdateTime;
    unsigned long _updateIntervalMs;
    
    // Module-specific private members
    // ...
};

// Initialize static instance pointer
ModuleTemplate* ModuleTemplate::_instance = nullptr;

