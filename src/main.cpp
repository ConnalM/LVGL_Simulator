#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif
#include <lvgl.h>
#include <../.pio/libdeps/native/lvgl/src/misc/lv_timer.h>
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

// Function declarations
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

// Screen dimensions
#define DISP_HOR_RES 480
#define DISP_VER_RES 320

// SDL2 variables
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[DISP_HOR_RES * 10];
static lv_color_t buf2[DISP_HOR_RES * 10];

int main(int argc, char** argv) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // Create window
    window = SDL_CreateWindow("LVGL Simulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISP_HOR_RES, DISP_VER_RES, SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create texture that stores the display content
    printf("Creating texture with size: %dx%d\n", DISP_HOR_RES, DISP_VER_RES);
    texture = SDL_CreateTexture(renderer,
                               SDL_PIXELFORMAT_RGB565,
                               SDL_TEXTUREACCESS_STREAMING,
                               DISP_HOR_RES, DISP_VER_RES);
    if (!texture) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Set texture blend mode
    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
        printf("Warning: Could not set texture blend mode: %s\n", SDL_GetError());
    }

    // Initialize LVGL
    lv_init();

    // Initialize the display buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_HOR_RES * 10);

    // Initialize the display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;  // Enable full screen refresh
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    
    // Set the display to the default screen
    lv_disp_set_default(disp);

    // Create a simple label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello LVGL Simulator!");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Create a button
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(btn, [](lv_event_t *e) {
        static int count = 0;
        count++;
        lv_label_set_text_fmt((lv_obj_t*)e->user_data, "Clicked: %d", count);
    }, LV_EVENT_CLICKED, label);

    // Add label to button
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Click Me!");
    lv_obj_center(btn_label);
    
    // Main loop
    bool quit = false;
    SDL_Event e;
    
    while (!quit) {
        // Handle SDL events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        // Call LVGL task handler
        lv_timer_handler();
        
        // Small delay to prevent high CPU usage
        SDL_Delay(5);
    }
    
    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

// Display flushing callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
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
    
    // Set the blend mode to none for better performance
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    
    // Update the texture with the new pixel data for the specific area
    if (SDL_UpdateTexture(texture, &rect, color_p, DISP_HOR_RES * sizeof(lv_color_t)) != 0) {
        printf("SDL_UpdateTexture error: %s\n", SDL_GetError());
    }
    
    // Clear the renderer with white background
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);
    
    // Copy the entire texture to the renderer
    if (SDL_RenderCopy(renderer, texture, NULL, NULL) != 0) {
        printf("SDL_RenderCopy error: %s\n", SDL_GetError());
    }
    
    // Update the screen
    SDL_RenderPresent(renderer);
    
    // Inform LVGL that flushing is done
    lv_disp_flush_ready(disp);
}