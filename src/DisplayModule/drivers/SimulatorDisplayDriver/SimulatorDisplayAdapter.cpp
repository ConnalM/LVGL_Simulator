#ifdef SIMULATOR

#include "SimulatorDisplayAdapter.h"
#include "DisplayModule/lvgl/screens/ConfigScreen.h"
#include "DisplayModule/lvgl/screens/RaceScreen.h"
#include "DisplayModule/lvgl/screens/RaceReadyScreen.h"
#include "DisplayModule/lvgl/screens/StatsScreen.h"
#include "DisplayModule/lvgl/screens/PauseScreen.h"
#include "DisplayModule/lvgl/screens/StopScreen.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>

// Initialize static members
SimulatorDisplayAdapter* SimulatorDisplayAdapter::instance_ = nullptr;

// Display dimensions - should match SDL window size
#define DISP_HOR_RES 800
#define DISP_VER_RES 480

SimulatorDisplayAdapter::SimulatorDisplayAdapter() 
    : cursor_x_(0)
    , cursor_y_(0)
    , text_color_(0xFFFFFF)
    , text_size_(1)
{
    // Constructor
    std::cout << "SimulatorDisplayAdapter: Constructor called" << std::endl;
}

SimulatorDisplayAdapter::~SimulatorDisplayAdapter() {
    // Destructor
    std::cout << "SimulatorDisplayAdapter: Destructor called" << std::endl;
}

SimulatorDisplayAdapter& SimulatorDisplayAdapter::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new SimulatorDisplayAdapter();
    }
    return *instance_;
}

bool SimulatorDisplayAdapter::initialize() {
    std::cout << "SimulatorDisplayAdapter: Initializing" << std::endl;
    // The SDL backend should already be initialized in main.cpp
    // We just need to make sure LVGL is ready
    return true;
}

void SimulatorDisplayAdapter::update() {
    // This will be called in the main loop
    // LVGL timer handler is already called in the main loop
    // Nothing to do here
}

void SimulatorDisplayAdapter::clear() {
    // Clear the current screen
    lv_obj_clean(lv_scr_act());
}

void SimulatorDisplayAdapter::print(const String& message, bool newLine) {
    // Print to console for simulator
    if (newLine) {
        std::cout << message.c_str() << std::endl;
    } else {
        std::cout << message.c_str();
    }
    
    // For visual feedback, we could also display this on the LVGL screen
    // but that's probably not necessary for most use cases
}

void SimulatorDisplayAdapter::printf(const char* format, ...) {
    // Format the string
    va_list args;
    va_start(args, format);
    
    // Use vsnprintf to format the string
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    
    // Print to console
    std::cout << buffer;
    
    va_end(args);
}

void SimulatorDisplayAdapter::setCursor(int x, int y) {
    cursor_x_ = x;
    cursor_y_ = y;
}

void SimulatorDisplayAdapter::setTextColor(uint32_t color) {
    text_color_ = color;
}

void SimulatorDisplayAdapter::setTextSize(uint8_t size) {
    text_size_ = size;
}

void SimulatorDisplayAdapter::drawPixel(int16_t x, int16_t y, uint16_t color) {
    // In LVGL, we could draw this on a canvas if needed
    // For now, we'll just log it
    std::cout << "SimulatorDisplayAdapter: drawPixel(" << x << ", " << y << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    // In LVGL, we could draw this on a canvas if needed
    // For now, we'll just log it
    std::cout << "SimulatorDisplayAdapter: drawLine(" << x0 << ", " << y0 << ", " << x1 << ", " << y1 << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::drawRect(int x, int y, int w, int h, uint32_t color) {
    // Create a rectangle object using LVGL
    lv_obj_t* rect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_bg_color(rect, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(rect, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(rect, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Log the operation
    std::cout << "SimulatorDisplayAdapter: drawRect(" << x << ", " << y << ", " << w << ", " << h << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::fillRect(int x, int y, int w, int h, uint32_t color) {
    // Create a filled rectangle object using LVGL
    lv_obj_t* rect = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_bg_color(rect, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Log the operation
    std::cout << "SimulatorDisplayAdapter: fillRect(" << x << ", " << y << ", " << w << ", " << h << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::drawCircle(int x, int y, int r, uint32_t color) {
    // Create a circle object using LVGL
    lv_obj_t* circle = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(circle, x - r, y - r);
    lv_obj_set_size(circle, 2 * r, 2 * r);
    lv_obj_set_style_radius(circle, r, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(circle, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(circle, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Log the operation
    std::cout << "SimulatorDisplayAdapter: drawCircle(" << x << ", " << y << ", " << r << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::fillCircle(int x, int y, int r, uint32_t color) {
    // In LVGL, we could draw this on a canvas if needed
    // For now, we'll just log it
    std::cout << "SimulatorDisplayAdapter: fillCircle(" << x << ", " << y << ", " << r << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // In LVGL, we could draw this on a canvas if needed
    // For now, we'll just log it
    std::cout << "SimulatorDisplayAdapter: drawTriangle(" << x0 << ", " << y0 << ", " << x1 << ", " << y1 << ", " << x2 << ", " << y2 << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    // In LVGL, we could draw this on a canvas if needed
    // For now, we'll just log it
    std::cout << "SimulatorDisplayAdapter: fillTriangle(" << x0 << ", " << y0 << ", " << x1 << ", " << y1 << ", " << x2 << ", " << y2 << ", " << color << ")" << std::endl;
}

void SimulatorDisplayAdapter::drawText(int16_t x, int16_t y, const String& text, uint16_t color, uint8_t size) {
    // In LVGL, we could draw this on a canvas if needed
    // For now, we'll just log it
    std::cout << "SimulatorDisplayAdapter: drawText(" << x << ", " << y << ", \"" << text.c_str() << "\", " << color << ", " << (int)size << ")" << std::endl;
}

int SimulatorDisplayAdapter::getWidth() const {
    return DISP_HOR_RES;
}

int SimulatorDisplayAdapter::getHeight() const {
    return DISP_VER_RES;
}

void SimulatorDisplayAdapter::drawMain() {
    std::cout << "SimulatorDisplayAdapter: Drawing main screen" << std::endl;
    
    // Create and show the main screen
    // For now, we'll use a placeholder implementation
    // In a real implementation, you would create a MainScreen class
    
    // Clear the screen
    lv_obj_clean(lv_scr_act());
    
    // Create a simple main screen with buttons
    lv_obj_t* screen = lv_scr_act();
    
    // Set black background
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    
    // Create title
    lv_obj_t* title = lv_label_create(screen);
    lv_label_set_text(title, "MAIN MENU");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    
    // Create buttons
    lv_obj_t* race_btn = lv_btn_create(screen);
    lv_obj_set_size(race_btn, 200, 80);
    lv_obj_align(race_btn, LV_ALIGN_CENTER, 0, -100);
    lv_obj_t* race_label = lv_label_create(race_btn);
    lv_label_set_text(race_label, "RACE");
    lv_obj_center(race_label);
    
    lv_obj_t* config_btn = lv_btn_create(screen);
    lv_obj_set_size(config_btn, 200, 80);
    lv_obj_align(config_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t* config_label = lv_label_create(config_btn);
    lv_label_set_text(config_label, "CONFIG");
    lv_obj_center(config_label);
    
    lv_obj_t* stats_btn = lv_btn_create(screen);
    lv_obj_set_size(stats_btn, 200, 80);
    lv_obj_align(stats_btn, LV_ALIGN_CENTER, 0, 100);
    lv_obj_t* stats_label = lv_label_create(stats_btn);
    lv_label_set_text(stats_label, "STATS");
    lv_obj_center(stats_label);
}

void SimulatorDisplayAdapter::drawRaceReady() {
    std::cout << "SimulatorDisplayAdapter: Drawing race ready screen" << std::endl;
    
    // Create and show the race ready screen
    static RaceReadyScreen raceReadyScreen;
    raceReadyScreen.Show();
}

void SimulatorDisplayAdapter::drawConfig() {
    std::cout << "SimulatorDisplayAdapter: Drawing config screen" << std::endl;
    
    // Create and show the config screen
    static ConfigScreen configScreen;
    configScreen.Show();
}

void SimulatorDisplayAdapter::drawRaceActive(RaceMode raceMode) {
    std::cout << "SimulatorDisplayAdapter: Drawing race active screen with mode " << static_cast<int>(raceMode) << std::endl;
    
    // Create and show the race screen
    static RaceScreen raceScreen;
    // Just show the race screen without passing the race mode
    // The race mode would need to be set separately or handled differently
    raceScreen.Show();
}

void SimulatorDisplayAdapter::startLightSequence() {
    std::cout << "SimulatorDisplayAdapter: Starting light sequence" << std::endl;
    
    // Delegate to the race ready screen
    // This would need to be implemented in RaceReadyScreen
    // For now, we'll just log it
}

void SimulatorDisplayAdapter::updateRaceData(const std::vector<RaceLaneData>& laneData) {
    std::cout << "SimulatorDisplayAdapter: Updating race data with " << laneData.size() << " lanes" << std::endl;
    
    // Delegate to the race screen
    // This would need to be implemented in RaceScreen
    // For now, we'll just log it
}

void SimulatorDisplayAdapter::drawStats() {
    std::cout << "SimulatorDisplayAdapter: Drawing stats screen" << std::endl;
    
    // Create and show the stats screen
    static StatsScreen statsScreen;
    statsScreen.Show();
}

void SimulatorDisplayAdapter::drawPause() {
    std::cout << "SimulatorDisplayAdapter: Drawing pause screen" << std::endl;
    
    // Create and show the pause screen
    static PauseScreen pauseScreen;
    pauseScreen.Show();
}

void SimulatorDisplayAdapter::drawStop() {
    std::cout << "SimulatorDisplayAdapter: Drawing stop screen" << std::endl;
    
    // Create and show the stop screen
    static StopScreen stopScreen;
    stopScreen.Show();
}

#endif // SIMULATOR
