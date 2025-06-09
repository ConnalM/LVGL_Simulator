#include "SDLInputHandler.h"
#include <cstdio>

// Forward declaration of log_message function from main.cpp
extern void log_message(const char* format, ...);

bool SDLInputHandler::processEvents() {
    static uint32_t startTime = SDL_GetTicks();
    static bool firstRun = true;
    static int eventCount = 0;
    
    if (firstRun) {
        log_message("SDLInputHandler started at: %u ms", startTime);
        firstRun = false;
    }
    
    SDL_Event e;
    bool quit = false;
    
    // Check SDL initialization status
    if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        log_message("ERROR: SDL video subsystem not initialized in SDLInputHandler");
        return false; // Don't quit, just report the error
    }
    
    // Handle SDL events
    while (SDL_PollEvent(&e)) {
        eventCount++;
        uint32_t currentTime = SDL_GetTicks();
        uint32_t elapsedTime = currentTime - startTime;
        
        // Log event type
        log_message("SDL Event #%d: Type=%d at time %u ms (elapsed: %u ms)", 
               eventCount, e.type, currentTime, elapsedTime);
        
        // Only quit if we receive an actual SDL_QUIT event
        if (e.type == SDL_QUIT) {
            log_message("Received SDL_QUIT event #%d at %u ms (elapsed: %u ms)", 
                   eventCount, currentTime, elapsedTime);
            quit = true; // Set quit flag but continue processing events
        }
        // Handle mouse motion events
        else if (e.type == SDL_MOUSEMOTION) {
            // Just log the event, LVGL will handle it through the mouse_read callback
            int x, y;
            SDL_GetMouseState(&x, &y);
            log_message("Mouse motion event #%d: position x=%d, y=%d", eventCount, x, y);
            // No need to do anything special here, the mouse_read callback will handle it
        }
        // Handle mouse button events
        else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
            log_message("Mouse button %s event #%d received at %u ms", 
                      (e.type == SDL_MOUSEBUTTONDOWN) ? "DOWN" : "UP", 
                      eventCount, currentTime);
            
            // Get mouse position
            int x, y;
            SDL_GetMouseState(&x, &y);
            
            // Just log the event, LVGL will handle it through the mouse_read callback
            log_message("Mouse position for event #%d: x=%d, y=%d", eventCount, x, y);
            
            // CRITICAL: Do not return true here, which would signal to quit
            // Instead, continue processing events
        }
        // Handle keyboard events
        else if (e.type == SDL_KEYDOWN) {
            log_message("Keyboard DOWN event #%d: key=%d at %u ms", 
                      eventCount, e.key.keysym.sym, currentTime);
            
            // Check for ESC key to quit
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                log_message("ESC key pressed, setting quit flag");
                quit = true;
            }
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
    
    // Log if we're returning true (quitting)
    if (quit) {
        log_message("SDLInputHandler::processEvents() returning true (quit) after %u ms", 
               currentTime - startTime);
    }
    
    return quit; // Only return true if we received an SDL_QUIT event or ESC key
}
