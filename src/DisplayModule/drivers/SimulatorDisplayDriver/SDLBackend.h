#pragma once

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <lvgl.h>

// Screen dimensions
#define DISP_HOR_RES 800
#define DISP_VER_RES 480

/**
 * @brief Class to handle SDL2 initialization and management for the LVGL simulator
 */
class SDLBackend {
public:
    /**
     * @brief Initialize SDL2 and create window, renderer, and texture
     * 
     * @param width Width of the display in pixels
     * @param height Height of the display in pixels
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    static bool init(int width, int height);
    
    /**
     * @brief Clean up SDL2 resources
     */
    static void cleanup();
    
    /**
     * @brief Update the texture with new pixel data
     * 
     * @param area Area to update
     * @param color_p Pixel data
     * @return true if successful
     * @return false if failed
     */
    static bool updateTexture(const lv_area_t *area, lv_color_t *color_p);
    
    /**
     * @brief Render the texture to the screen
     */
    static void render();
    
    /**
     * @brief Get the SDL window
     * 
     * @return SDL_Window* The SDL window
     */
    static SDL_Window* getWindow() { return _window; }
    
    /**
     * @brief Get the SDL renderer
     * 
     * @return SDL_Renderer* The SDL renderer
     */
    static SDL_Renderer* getRenderer() { return _renderer; }
    
    /**
     * @brief Get the SDL texture
     * 
     * @return SDL_Texture* The SDL texture
     */
    static SDL_Texture* getTexture() { return _texture; }

private:
    static SDL_Window* _window;
    static SDL_Renderer* _renderer;
    static SDL_Texture* _texture;
    static bool _initialized;
};
