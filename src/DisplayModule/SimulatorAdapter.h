#pragma once

#include "DisplayModule.h"
#include "DisplayManager.h"
#include <lvgl.h>
#include <SDL.h>

/**
 * @brief Adapter class to connect the LVGL simulator with the DisplayModule architecture
 * 
 * This class provides a bridge between the LVGL simulator's SDL2 backend and
 * the DisplayModule architecture. It initializes the DisplayManager and routes
 * display operations through the modular architecture.
 */
class SimulatorAdapter {
public:
    /**
     * @brief Initialize the simulator adapter
     * 
     * @param window SDL window pointer
     * @param renderer SDL renderer pointer
     * @param texture SDL texture pointer
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    static bool initialize(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture);
    
    /**
     * @brief Update the simulator adapter
     * 
     * This should be called regularly in the main loop.
     */
    static void update();
    
    /**
     * @brief Get the DisplayManager instance
     * 
     * @return DisplayManager& The DisplayManager instance
     */
    static DisplayManager& getDisplayManager();
    
    /**
     * @brief Set the current screen
     * 
     * @param screen The screen to display
     */
    static void setScreen(ScreenType screen);

private:
    static DisplayManager* _displayManager;
    static SDL_Window* _window;
    static SDL_Renderer* _renderer;
    static SDL_Texture* _texture;
    static bool _initialized;
};
