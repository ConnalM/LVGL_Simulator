#include "LVGL_Simulator_DisplayDriver.h"
#include <cstdarg>
#include <stdio.h>

// External SDL variables from main.cpp
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;

// Constructor
LVGL_Simulator_DisplayDriver::LVGL_Simulator_DisplayDriver() 
    : ui_MainMenuScreen(nullptr),
      ui_RaceReadyScreen(nullptr),
      ui_ConfigScreen(nullptr),
      ui_RaceActiveScreen(nullptr),
      ui_CountdownScreen(nullptr),
      _debugLabel(nullptr),
      race_ready_screen_(nullptr),
      config_screen_(nullptr) {
}

// Destructor
LVGL_Simulator_DisplayDriver::~LVGL_Simulator_DisplayDriver() {
    // Cleanup any resources if needed
}

// Initialize the display
bool LVGL_Simulator_DisplayDriver::initialize() {
    // The SDL initialization is handled in main.cpp
    // Here we just create our UI components
    
    // Create debug label for print/printf output
    _debugLabel = lv_label_create(lv_scr_act());
    lv_obj_align(_debugLabel, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_label_set_text(_debugLabel, "");
    
    // Create screens
    createMainMenuScreen();
    
    return true;
}

// Update the display
void LVGL_Simulator_DisplayDriver::update() {
    // LVGL timer handler is called in the main loop in main.cpp
    // Nothing to do here
}

// Clear the display
void LVGL_Simulator_DisplayDriver::clear() {
    // Clear the active screen
    lv_obj_clean(lv_scr_act());
}

// Print a message to the display
void LVGL_Simulator_DisplayDriver::print(const String& message, bool newLine) {
    if (_debugLabel) {
        String currentText = lv_label_get_text(_debugLabel);
        String newText = currentText + message;
        if (newLine) {
            newText += "\n";
        }
        lv_label_set_text(_debugLabel, newText.c_str());
    }
    
    // Also print to console for debugging
    if (newLine) {
        printf("%s\n", message.c_str());
    } else {
        printf("%s", message.c_str());
    }
}

// Print a formatted message to the display
void LVGL_Simulator_DisplayDriver::printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    print(buffer);
}

// Set cursor position
void LVGL_Simulator_DisplayDriver::setCursor(int x, int y) {
    // Not directly applicable in LVGL, but we can store the position
    // for future text operations if needed
}

// Set text color
void LVGL_Simulator_DisplayDriver::setTextColor(uint32_t color) {
    // Not directly applicable in LVGL, but we can store the color
    // for future text operations if needed
}

// Set text size
void LVGL_Simulator_DisplayDriver::setTextSize(uint8_t size) {
    // Not directly applicable in LVGL, but we can store the size
    // for future text operations if needed
}

// Draw a rectangle
void LVGL_Simulator_DisplayDriver::drawRect(int x, int y, int w, int h, uint32_t color) {
    // Create a rectangle object
    lv_obj_t* rect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_bg_color(rect, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(rect, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(rect, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
}

// Fill a rectangle
void LVGL_Simulator_DisplayDriver::fillRect(int x, int y, int w, int h, uint32_t color) {
    // Create a filled rectangle object
    lv_obj_t* rect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_bg_color(rect, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
}

// Draw a circle
void LVGL_Simulator_DisplayDriver::drawCircle(int x, int y, int r, uint32_t color) {
    // Create a circle object
    lv_obj_t* circle = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(circle, x - r, y - r);
    lv_obj_set_size(circle, 2 * r, 2 * r);
    lv_obj_set_style_radius(circle, r, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(circle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(circle, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
}
