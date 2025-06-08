#pragma once

#include <SDL.h>
#include <lvgl.h>

/**
 * @brief Class to handle SDL input events for the LVGL simulator
 */
class SDLInputHandler {
public:
    /**
     * @brief Process SDL events
     * 
     * @return true if the application should quit
     * @return false if the application should continue
     */
    static bool processEvents();
    
private:
    // Add any private members as needed
};
