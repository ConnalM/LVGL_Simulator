#pragma once

#include <lvgl.h>

// Color utilities for ESP32 display with proper color format handling
namespace ColorUtils {
    // Standard colors with proper format for ESP32 display
    // These handle any necessary color format conversions
    
    // Direct RGB565 color values for ESP32-8048S070 display
    // Using direct RGB565 values that are known to work on this display
    // Standard RGB colors using LVGL's color functions
    // With LV_COLOR_16_SWAP=1, LVGL will handle the byte swapping for us
    inline lv_color_t Red() {
        return lv_color_make(255, 0, 0); // Standard RGB red
    }
    
    inline lv_color_t Green() { 
        return lv_color_make(0, 255, 0); // Standard RGB green
    }
    
    inline lv_color_t Blue() { 
        return lv_color_make(0, 0, 255); // Standard RGB blue
    }
    
    inline lv_color_t Yellow() { 
        return lv_color_make(255, 255, 0); // Standard RGB yellow
    }
    
    // Additional colors using standard RGB values
    inline lv_color_t Magenta() {
        // Magenta is red + blue
        return lv_color_make(255, 0, 255);
    }
    
    inline lv_color_t Cyan() {
        // Cyan is green + blue
        return lv_color_make(0, 255, 255);
    }
    
    inline lv_color_t Orange() {
        // Orange is a mix of red and yellow
        return lv_color_make(255, 128, 0);
    }
    
    inline lv_color_t Purple() {
        // A darker magenta/violet color
        return lv_color_make(128, 0, 128);
    }
    inline lv_color_t White() { 
        // Pure white (255, 255, 255)
        return lv_color_make(255, 255, 255);
    }
    
    inline lv_color_t Black() { 
        // Pure black (0, 0, 0)
        return lv_color_make(0, 0, 0);
    }
    
    inline lv_color_t AlmostBlack() { 
        // Very dark grey (17, 17, 17) - nearly black, for inactive lights
        return lv_color_make(17, 17, 17);
    }
    
    inline lv_color_t LightGrey() {
        // Slightly lighter dark grey (34, 34, 34) - for subtle highlights
        return lv_color_make(34, 34, 34);
    }
    
    // Function to convert standard RGB to the display's color format
    // Use this for any custom colors not defined above
    inline lv_color_t RGB(uint8_t r, uint8_t g, uint8_t b) {
        // With LV_COLOR_16_SWAP=1, LVGL will handle the byte swapping for us
        return lv_color_make(r, g, b);
    }
    
    // Function to convert hex color to the display's color format
    inline lv_color_t FromHex(uint32_t hex) {
        // Extract RGB components from hex
        uint8_t r = (hex >> 16) & 0xFF;
        uint8_t g = (hex >> 8) & 0xFF;
        uint8_t b = hex & 0xFF;
        
        // With LV_COLOR_16_SWAP=1, LVGL will handle the byte swapping for us
        return lv_color_make(r, g, b);
    }
}
