#ifndef LV_SDL_WINDOW_H
#define LV_SDL_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

void sdl_init(void);
void sdl_display_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void sdl_mouse_read(lv_indev_drv_t * drv, lv_indev_data_t * data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_SDL_WINDOW_H */
