#include "SDLInputHandler.h"
#include <cstdio>

// Forward declaration of log_message function from main.cpp
extern void log_message(const char* format, ...);

bool SDLInputHandler::processEvents() {
    static uint32_t startTime = SDL_GetTicks();
    static bool firstRun = true;
    
    if (firstRun) {
        log_message("SDLInputHandler started at: %u ms", startTime);
        firstRun = false;
    }
    
    SDL_Event e;
    bool quit = false;
    
    // Handle SDL events
    while (SDL_PollEvent(&e)) {
        uint32_t currentTime = SDL_GetTicks();
        uint32_t elapsedTime = currentTime - startTime;
        
        // Log event type
        log_message("SDL Event: Type=%d at time %u ms (elapsed: %u ms)", 
               e.type, currentTime, elapsedTime);
        
        // Only quit if we receive an actual SDL_QUIT event
        if (e.type == SDL_QUIT) {
            log_message("Received SDL_QUIT event at %u ms (elapsed: %u ms)", 
                   currentTime, elapsedTime);
            quit = true; // Set quit flag but continue processing events
        }
        // Handle mouse motion events
        else if (e.type == SDL_MOUSEMOTION) {
            // Just log the event, LVGL will handle it through the mouse_read callback
            int x, y;
            SDL_GetMouseState(&x, &y);
            // No need to do anything special here, the mouse_read callback will handle it
        }
        // Handle mouse button events
        else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            log_message("Mouse button %s event received at %u ms", 
                      (e.type == SDL_MOUSEBUTTONDOWN) ? "DOWN" : "UP", 
                      currentTime);
            
            // Get mouse position
            int x, y;
            SDL_GetMouseState(&x, &y);
            
            // Just log the event, LVGL will handle it through the mouse_read callback
            log_message("Mouse position: x=%d, y=%d", x, y);
            
            // CRITICAL: Do not return true here, which would signal to quit
            // Instead, continue processing events
        }
    }
    
    // Check for timeout (if simulator has been running for more than 15 seconds)
    uint32_t currentTime = SDL_GetTicks();
    if (currentTime - startTime > 15000) {
        static bool timeoutWarningPrinted = false;
        if (!timeoutWarningPrinted) {
            log_message("WARNING: Simulator has been running for %u ms", 
                   currentTime - startTime);
            timeoutWarningPrinted = true;
        }
    }
    
    return quit; // Only return true if we received an SDL_QUIT event
}
