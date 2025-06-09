#pragma once

#ifdef SIMULATOR

#include <lvgl.h>

// Define missing fonts for simulator
// These are defined to use existing fonts that are available in LVGL
// In a real implementation, you would include the actual font files

#ifndef LV_FONT_MONTSERRAT_24
extern const lv_font_t lv_font_montserrat_14;
extern const lv_font_t lv_font_montserrat_16;

// Map missing fonts to available ones
#define lv_font_montserrat_24 lv_font_montserrat_16
#define lv_font_montserrat_32 lv_font_montserrat_16
#define lv_font_montserrat_48 lv_font_montserrat_16
#endif

#endif // SIMULATOR
