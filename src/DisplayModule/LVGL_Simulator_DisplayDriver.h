#pragma once

#include "DisplayModule.h"       // For IGraphicalDisplay, DisplayType
#include "../common/Types.h"     // For RaceData, ErrorInfo etc.
#include <lvgl.h>
#include "lvgl/screens/ConfigScreen.h"
#include "lvgl/screens/RaceScreen.h"
#include "lvgl/screens/StatsScreen.h"

// Display dimensions - match the simulator dimensions
#define SIM_LCD_WIDTH 480
#define SIM_LCD_HEIGHT 320

/**
 * @brief LVGL Simulator Display Driver
 * 
 * This class implements the IGraphicalDisplay interface using the LVGL simulator
 * with SDL2 backend. It's designed to be a drop-in replacement for the ESP32 display
 * driver during simulation and testing.
 */
class LVGL_Simulator_DisplayDriver : public IGraphicalDisplay {
public:
    LVGL_Simulator_DisplayDriver();
    virtual ~LVGL_Simulator_DisplayDriver();

    // --- IBaseDisplay --- (from DisplayModule.h)
    virtual bool initialize() override;
    virtual void update() override; // Will call lv_timer_handler()
    virtual void clear() override;  // Clears the LVGL active screen
    virtual void print(const String& message, bool newLine = true) override;
    virtual void printf(const char* format, ...) override;
    virtual DisplayType getDisplayType() const override { return DisplayType::LCD; }

    // --- IGraphicalDisplay --- (from DisplayModule.h)
    virtual void setCursor(int x, int y) override;
    virtual void setTextColor(uint32_t color) override;
    virtual void setTextSize(uint8_t size) override;
    virtual void drawRect(int x, int y, int w, int h, uint32_t color) override;
    virtual void fillRect(int x, int y, int w, int h, uint32_t color) override;
    virtual void drawCircle(int x, int y, int r, uint32_t color) override;
    virtual void fillCircle(int x, int y, int r, uint32_t color) override;
    virtual int getWidth() const override { return SIM_LCD_WIDTH; }
    virtual int getHeight() const override { return SIM_LCD_HEIGHT; }

    // UI Screen methods - these will trigger LVGL screen loads
    virtual void drawMain() override;
    virtual void drawRaceReady() override;
    virtual void drawConfig() override;
    virtual void drawRaceActive(RaceMode raceMode) override;
    virtual void startLightSequence() override;
    virtual void updateRaceData(const std::vector<RaceLaneData>& laneData) override;
    virtual void drawStats() override;
    virtual void drawPause() override;
    virtual void drawStop() override;

    // Helper methods for specific screens
    void updateCountdownDisplay(int currentStep);

    // Test mode flag - when true, bypasses automatic race preparation in drawRaceActive
    void setTestMode(bool enabled) { _testMode = enabled; }
    bool getTestMode() const { return _testMode; }

private:
    // LVGL Screen Objects
    lv_obj_t* ui_MainMenuScreen;     // Main menu screen
    StatsScreen* _statsScreen = nullptr;  // Stats screen
    lv_obj_t* ui_RaceReadyScreen;    // Race ready screen (configuration and countdown)
    lv_obj_t* ui_ConfigScreen;       // Configuration menu screen
    lv_obj_t* ui_RaceActiveScreen;   // Active race display screen
    ConfigScreen* config_screen_;    // Config screen implementation
    lv_obj_t* ui_CountdownScreen;    // Countdown overlay (part of RaceReady)
    class RaceReadyScreen* race_ready_screen_; // RaceReadyScreen instance

    // Label for basic print/printf output
    lv_obj_t* _debugLabel;

    // Helper methods for creating LVGL screens
    void createMainMenuScreen();     // Create main menu screen
    void createRaceReadyScreen();    // Create race ready screen
    void createConfigScreen();       // Create configuration screen
    void createRaceActiveScreen();   // Create race active screen
    void createCountdownScreen();    // Create countdown overlay

    // LVGL Event Callbacks
    static void event_cb_main_menu_race_button(lv_event_t * e);
    static void event_cb_main_menu_config_button(lv_event_t * e);
    static void event_cb_race_menu_start_button(lv_event_t* e);
    static void event_cb_race_menu_return_button(lv_event_t * e);
    
    // Test mode flag
    bool _testMode = false;
    
    // Reference to the active race screen for updating race data
    RaceScreen* _activeRaceScreen = nullptr;
};
