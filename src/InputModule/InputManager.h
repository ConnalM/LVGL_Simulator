#pragma once
#include <vector>
#include "InputModule.h"
#include "common/TimeManager.h"
#include "common/Types.h"

/**
 * @brief Manager for input modules
 * 
 * The InputManager collects and coordinates input from various sources
 * (keyboard, sensors, touchscreen, etc.) through registered InputModule instances.
 * It provides a unified polling interface for the system to receive input events.
 */
class InputManager {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return InputManager& The singleton instance
     */
    static InputManager& getInstance();
    
    /**
     * @brief Initialize the input manager
     * 
     * This should be called once during system startup.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Add an input module to the manager
     * 
     * @param module Pointer to the input module to add
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo addInputModule(InputModule* module);
    
    /**
     * @brief Poll for input events from all registered modules
     * 
     * @param event Reference to store the received event
     * @return bool true if an event was received, false otherwise
     */
    bool poll(InputEvent& event);
    
    /**
     * @brief Update the input manager
     * 
     * This should be called regularly in the main loop.
     * Currently does nothing but included for consistency with other modules.
     */
    void update();
    
private:
    // Private constructor for singleton pattern
    InputManager();
    
    // Prevent copying and assignment
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    // Static instance pointer
    static InputManager* _instance;
    
    std::vector<InputModule*> modules;
    bool _initialized = false;
};
