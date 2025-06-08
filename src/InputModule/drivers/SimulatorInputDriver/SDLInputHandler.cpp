#include "SDLInputHandler.h"

bool SDLInputHandler::processEvents() {
    SDL_Event e;
    
    // Handle SDL events
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return true; // Quit
        }
        
        // Handle mouse events for LVGL
        if (e.type == SDL_MOUSEMOTION) {
            lv_disp_t* disp = lv_disp_get_default();
            if (disp) {
                lv_disp_drv_t* disp_drv = disp->driver;
                if (disp_drv->rotated == LV_DISP_ROT_NONE) {
                    lv_indev_set_cursor(lv_indev_get_next(NULL), NULL);
                }
            }
        }
    }
    
    return false; // Continue
}
