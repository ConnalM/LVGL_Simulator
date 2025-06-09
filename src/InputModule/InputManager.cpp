#include "InputManager.h"
#include "DisplayModule/DisplayManager.h"

// Initialize static instance pointer
InputManager* InputManager::_instance = nullptr;

InputManager& InputManager::getInstance() {
    if (_instance == nullptr) {
        _instance = new InputManager();
    }
    return *_instance;
}

InputManager::InputManager() 
    : _initialized(false) {
    // Constructor implementation
}

bool InputManager::initialize() {
    // Check if already initialized
    if (_initialized) {
        DisplayManager::getInstance().info("Already initialized", "InputManager");
        return true;
    }
    
    DisplayManager::getInstance().info("Initializing...", "InputManager");
    
    // No special initialization required for this module
    // but we follow the standard pattern for consistency
    
    _initialized = true;
    DisplayManager::getInstance().info("Initialized successfully", "InputManager");
    return true;
}

ErrorInfo InputManager::addInputModule(InputModule* module) {
    // Skip if not initialized
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "InputManager not initialized", "InputManager");
    }
    
    // Check for null module
    if (module == nullptr) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Null input module", "InputManager");
    }
    
    // Add the module to our list
    modules.push_back(module);
    return ErrorInfo(); // Success
}

bool InputManager::poll(InputEvent& event) {
    // Skip if not initialized
    if (!_initialized) {
        return false;
    }
    
    // Poll each module in order
    for (auto module : modules) {
        if (module->poll(event)) {
            // Set timestamp using TimeManager
            event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
            
            // If target is not set, use the default target for this command
            if ((int)event.target < 0) {
                event.target = getDefaultTargetForCommand(event.command);
            }
            
            return true;
        }
    }
    return false;
}

void InputManager::update() {
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Currently no periodic updates needed
    // This method is included for consistency with other modules
}
