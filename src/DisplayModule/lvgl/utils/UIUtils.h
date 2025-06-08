#pragma once

#include <lvgl.h>

// Helper function to create a standardized button - can be reused across the program
lv_obj_t* createStandardButton(lv_obj_t* parent, const char* label_text, 
                              lv_coord_t x_pos, lv_coord_t y_pos, 
                              lv_coord_t width, lv_coord_t height, 
                              lv_color_t bg_color, lv_color_t pressed_color,
                              lv_color_t text_color = lv_color_black());
