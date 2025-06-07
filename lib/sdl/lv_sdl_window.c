#include "lv_sdl_window.h"
#include <SDL2/SDL.h>

static SDL_Window * window = NULL;
static SDL_Renderer * renderer = NULL;
static SDL_Texture * texture = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t * buf = NULL;

void sdl_init(void) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Create window
    window = SDL_CreateWindow("LVGL Simulator",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            800, 480, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Create texture that stores the display content
    texture = SDL_CreateTexture(renderer,
                              SDL_PIXELFORMAT_RGB565,
                              SDL_TEXTUREACCESS_STREAMING,
                              800, 480);
    if (texture == NULL) {
        printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }

    // Allocate display buffer
    buf = (lv_color_t *)malloc(800 * 10 * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, 800 * 10);
}

void sdl_display_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
    if (renderer == NULL || texture == NULL) return;

    // Update the texture with the new pixel data
    SDL_UpdateTexture(texture, NULL, color_p, 800 * sizeof(lv_color_t));

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Copy the texture to the renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    // Update the screen
    SDL_RenderPresent(renderer);

    // Tell LVGL that the flush is ready
    lv_disp_flush_ready(disp_drv);
}

void sdl_mouse_read(lv_indev_drv_t * drv, lv_indev_data_t * data) {
    SDL_Event event;
    static int last_x = 0;
    static int last_y = 0;
    static bool left_button_down = false;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                last_x = event.motion.x;
                last_y = event.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    left_button_down = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    left_button_down = false;
                }
                break;
        }
    }

    data->point.x = last_x;
    data->point.y = last_y;
    data->state = left_button_down ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}
