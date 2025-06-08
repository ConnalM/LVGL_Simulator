#pragma once

#include "DisplayModule.h"       // For IGraphicalDisplay, DisplayType
#include "../common/Types.h"        // For RaceData, ErrorInfo etc.
#include "../common/TimeManager.h"  // For TimeManager

// Forward declaration for RaceLaneData
struct RaceLaneData;
#include <lvgl.h>
#include "lvgl/screens/ConfigScreen.h"
#include <Arduino_GFX_Library.h> // For Arduino_GFX
#include <esp_heap_caps.h>       // For heap_caps_malloc (PSRAM)
#include "lvgl/screens/ConfigScreen.h"  // Include ConfigScreen header
#include "lvgl/screens/RaceScreen.h"    // Include RaceScreen header
#include "lvgl/screens/StatsScreen.h"   // Include StatsScreen header

// Display dimensions
#define LCD_WIDTH 800
#define LCD_HEIGHT 480

// GFX Backlight Pin (GFX_BL is often used by Arduino_GFX)
// Ensure this matches your board's actual backlight pin if controlled by GFX lib
// If backlight is always on or controlled differently, this might not be needed or set to -1
#define GFX_BL 2 // Backlight control pin for ESP32-8048S070C
#define LCD_BACKLIGHT_PIN GFX_BL 

// ESP32-8048S070 RGB Pin Definitions (from Display5_TouchTest memory)
#define LCD_DE_PIN 41
#define LCD_VSYNC_PIN 40
#define LCD_HSYNC_PIN 39
#define LCD_PCLK_PIN 42
#define LCD_R0_PIN 14
#define LCD_R1_PIN 21
#define LCD_R2_PIN 47
#define LCD_R3_PIN 48
#define LCD_R4_PIN 45
#define LCD_G0_PIN 9
#define LCD_G1_PIN 46
#define LCD_G2_PIN 3
#define LCD_G3_PIN 8
#define LCD_G4_PIN 16
#define LCD_G5_PIN 1
#define LCD_B0_PIN 15
#define LCD_B1_PIN 7
#define LCD_B2_PIN 6
#define LCD_B3_PIN 5
#define LCD_B4_PIN 4

// Timing parameters (from Display5_TouchTest memory/ESP32_Replication_Guide.md)
#define HSYNC_POLARITY 0
#define HSYNC_FRONT_PORCH 210 
#define HSYNC_PULSE_WIDTH 30 
#define HSYNC_BACK_PORCH 16 

#define VSYNC_POLARITY 0
#define VSYNC_FRONT_PORCH 22 
#define VSYNC_PULSE_WIDTH 13 
#define VSYNC_BACK_PORCH 10 

#define PCLK_ACTIVE_NEG 1
#define PREFER_SPEED_HZ 16000000 // Adjusted from 12MHz to 16MHz; matches working Arduino_GFX config in Display5_TouchTest

// Backlight Control

class ESP32_8048S070_Lvgl_DisplayDriver : public IGraphicalDisplay {
public:
    ESP32_8048S070_Lvgl_DisplayDriver();
    virtual ~ESP32_8048S070_Lvgl_DisplayDriver();

    // --- IBaseDisplay --- (from DisplayModule.h)
    virtual bool initialize() override;
    virtual void update() override; // Will call lv_timer_handler()
    virtual void clear() override;  // Clears the LVGL active screen
    // Basic print methods might be less relevant with full LVGL UI
    virtual void print(const String& message, bool newLine = true) override;
    virtual void printf(const char* format, ...) override;
    virtual DisplayType getDisplayType() const override { return DisplayType::LCD; }

    // --- IGraphicalDisplay --- (from DisplayModule.h)
    // Basic GFX functions: map to LVGL drawing on a canvas or specific objects if needed.
    // Or, can be stubbed if direct GFX drawing is not the primary use case with LVGL.
    virtual void setCursor(int x, int y) override; 
    virtual void setTextColor(uint32_t color) override;
    virtual void setTextSize(uint8_t size) override;
    virtual void drawRect(int x, int y, int w, int h, uint32_t color) override;
    virtual void fillRect(int x, int y, int w, int h, uint32_t color) override;
    virtual void drawCircle(int x, int y, int r, uint32_t color) override;
    virtual void fillCircle(int x, int y, int r, uint32_t color) override;
    virtual int getWidth() const override { return LCD_WIDTH; }
    virtual int getHeight() const override { return LCD_HEIGHT; }

    // UI Screen methods - these will trigger LVGL screen loads
    virtual void drawMain() override;
    virtual void drawRaceReady() override;
    virtual void drawConfig() override;
    virtual void drawRaceActive(RaceMode raceMode) override;
    virtual void startLightSequence() override;
    virtual void updateRaceData(const std::vector<RaceLaneData>& laneData) override;
    
    /**
     * @brief Draw the statistics screen
     * 
     * Implements the IGraphicalDisplay interface to show the statistics screen.
     */
    virtual void drawStats() override;
    
    // Helper methods for specific screens
    void updateCountdownDisplay(int currentStep);
    
    // Deprecated methods kept for backward compatibility
    void drawRaceScreenMenu(); // Deprecated: use drawRaceReady instead

    // LVGL specific methods
    static void lvgl_display_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
    
    // Draw the pause screen
    virtual void drawPause() override;
    
    // Draw the stop screen
    virtual void drawStop() override;
    
    // Test mode flag - when true, bypasses automatic race preparation in drawRaceActive
    void setTestMode(bool enabled) { _testMode = enabled; }
    bool getTestMode() const { return _testMode; }

private:
    Arduino_ESP32RGBPanel* _rgbPanel;    // GFX panel object
    Arduino_RGB_Display* _gfx;          // GFX display object (wrapper around panel)
    lv_color_t* _psram_lvglBuffer1;     // Dynamically allocated LVGL draw buffer in PSRAM

    lv_disp_drv_t _lvglDisplayDriver;   // LVGL display driver

    // LVGL Screen Objects (add more as you define more screens)
    lv_obj_t* ui_MainMenuScreen;     // Main menu screen
    StatsScreen* _statsScreen = nullptr;  // Stats screen
    lv_obj_t* ui_RaceReadyScreen;    // Race ready screen (configuration and countdown)
    lv_obj_t* ui_ConfigScreen;       // Configuration menu screen
    lv_obj_t* ui_RaceActiveScreen;   // Active race display screen
    ConfigScreen* config_screen_;    // Config screen implementation
    lv_obj_t* ui_CountdownScreen;    // Countdown overlay (part of RaceReady)
    class RaceReadyScreen* race_ready_screen_; // RaceReadyScreen instance

    // Label for basic print/printf output (optional)
    lv_obj_t* _debugLabel; 

    // Helper methods for creating LVGL screens
    void createMainMenuScreen();     // Create main menu screen
    void createRaceReadyScreen();    // Create race ready screen
    void createConfigScreen();       // Create configuration screen
    void createRaceActiveScreen();   // Create race active screen
    void createCountdownScreen();    // Create countdown overlay

    // LVGL Event Callbacks (static or global, or use lv_event_set_user_data to pass instance)
    // These will construct InputEvent and call GT911_TouchInput::queueSystemInputEvent(...)
    static void event_cb_main_menu_race_button(lv_event_t * e);
    static void event_cb_main_menu_config_button(lv_event_t * e);
    static void event_cb_race_menu_start_button(lv_event_t* e);
    static void event_cb_race_menu_return_button(lv_event_t * e);
    // ... more event callbacks for other interactive elements
    
    // Test mode flag
    bool _testMode = false;
    
    // Reference to the active race screen for updating race data
    RaceScreen* _activeRaceScreen = nullptr;
};
