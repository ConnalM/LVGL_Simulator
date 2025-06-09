#include "SDLBackend.h"

// Initialize static members
SDL_Window* SDLBackend::_window = NULL;
SDL_Renderer* SDLBackend::_renderer = NULL;
SDL_Texture* SDLBackend::_texture = NULL;
bool SDLBackend::_initialized = false;

bool SDLBackend::init(int width, int height) {
    if (_initialized) {
        return true;
    }
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Create window
    _window = SDL_CreateWindow("LVGL Simulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_SHOWN);
    
    if (!_window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    // Create renderer
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);
    if (!_renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }

    // Create texture that stores the display content
    printf("Creating texture with size: %dx%d\n", width, height);
    _texture = SDL_CreateTexture(_renderer,
                               SDL_PIXELFORMAT_RGB565,
                               SDL_TEXTUREACCESS_STREAMING,
                               width, height);
    if (!_texture) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(_renderer);
        SDL_DestroyWindow(_window);
        SDL_Quit();
        return false;
    }
    
    // Set texture blend mode
    if (SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND) != 0) {
        printf("Warning: Could not set texture blend mode: %s\n", SDL_GetError());
    }
    
    _initialized = true;
    return true;
}

void SDLBackend::cleanup() {
    if (!_initialized) {
        return;
    }
    
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    SDL_Quit();
    
    _texture = NULL;
    _renderer = NULL;
    _window = NULL;
    _initialized = false;
}

bool SDLBackend::updateTexture(const lv_area_t *area, lv_color_t *color_p) {
    if (!_initialized) {
        return false;
    }
    
    // Calculate the width and height of the area to update
    int32_t w = area->x2 - area->x1 + 1;
    int32_t h = area->y2 - area->y1 + 1;
    
    printf("Flushing area: (%d,%d) to (%d,%d) - Size: %dx%d\n", 
           area->x1, area->y1, area->x2, area->y2, w, h);
    
    // Create a rectangle for the area to update
    SDL_Rect rect;
    rect.x = area->x1;
    rect.y = area->y1;
    rect.w = w;
    rect.h = h;
    
    // Update the texture with the new pixel data for the specific area
    if (SDL_UpdateTexture(_texture, &rect, color_p, DISP_HOR_RES * sizeof(lv_color_t)) != 0) {
        printf("SDL_UpdateTexture error: %s\n", SDL_GetError());
        return false;
    }
    
    return true;
}

void SDLBackend::render() {
    if (!_initialized) {
        return;
    }
    
    // Set the blend mode to none for better performance
    SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_NONE);
    
    // Clear the renderer with white background
    SDL_SetRenderDrawColor(_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(_renderer);
    
    // Copy the entire texture to the renderer
    if (SDL_RenderCopy(_renderer, _texture, NULL, NULL) != 0) {
        printf("SDL_RenderCopy error: %s\n", SDL_GetError());
    }
    
    // Update the screen
    SDL_RenderPresent(_renderer);
}
