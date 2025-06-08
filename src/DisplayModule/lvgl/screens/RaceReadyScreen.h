#pragma once

#include <lvgl.h>
#include <array>
#include <cstdint>
#include <functional>
#include "../../../InputModule/GT911_TouchInput.h" // For queueSystemEvent
#include "../../../common/TimeManager.h"         // For timestamp
#include "RaceScreen.h"                         // For showing race screen after countdown
#include "BaseScreen.h"

// Callback function type for countdown steps
using CountdownStepCallback = std::function<void(int)>;

// RaceReadyScreen: "Ready..." lights countdown screen for LVGL
class RaceReadyScreen : public BaseScreen {
public:
    enum FinalWaitType {
        RANDOM = 0,
        FIXED_1 = 1,
        FIXED_2 = 2,
        FIXED_3 = 3,
        FIXED_4 = 4,
        FIXED_5 = 5
    };

    explicit RaceReadyScreen();
    ~RaceReadyScreen() override;

    void Show() override;
    void Hide() override;

    void SetInterval(float seconds); // Set interval between red lights (default 0.5)
    void SetFinalWait(FinalWaitType type); // Set final wait type (random or 1-5s)
    
    // Public methods for controlling the light sequence - to be called by SystemController
    void StartRedSequence();  // Start the red lights sequence
    void ResetLights();      // Reset all lights to initial state here
    
    // Set callback for countdown steps
    void SetCountdownStepCallback(CountdownStepCallback callback);

private:
    // Screen elements managed by BaseScreen
    std::array<std::array<lv_obj_t*, 5>, 2> lights_; // 2 rows x 5 columns
    
    // Button UI elements
    lv_obj_t* race_button_;   // Button to start race
    lv_obj_t* return_button_; // Button to return to previous screen
    
    // BaseScreen pure virtual function implementations
    void OnLeftButtonClick() override;
    void OnRightButtonClick() override;
    
    // Get the screen object
    lv_obj_t* getScreen() const { return BaseScreen::getScreen(); }

    float interval_seconds_; // Interval between red lights
    FinalWaitType final_wait_type_;

    lv_timer_t* start_delay_timer_; // Timer for initial 1-second delay
    lv_timer_t* red_timer_;
    lv_timer_t* final_wait_timer_;
    lv_timer_t* green_timer_;
    lv_timer_t* reset_timer_; // Timer for resetting lights after green

    uint8_t current_column_;
    
    // Callback for countdown steps
    CountdownStepCallback countdown_step_callback_ = nullptr;

    static void StartDelayCallback(lv_timer_t* timer);
    static void RedLightStepCallback(lv_timer_t* timer);
    static void FinalWaitCallback(lv_timer_t* timer);
    static void GreenStepCallback(lv_timer_t* timer);
    static void ResetLightsCallback(lv_timer_t* timer);
    
    // Debug method to check if touch is within button bounds (kept for compatibility)
    static bool IsPointInButton(lv_obj_t* btn, lv_coord_t x, lv_coord_t y);
    
    // Button click handlers implemented from BaseScreen
    // void OnLeftButtonClick() override;  // Handles CANCEL button
    // void OnRightButtonClick() override; // Handles START button
    
    void StartFinalWait();
    void ShowGreen();
    void ReturnToPreviousScreen();

    // Utility
    void SetLightColor(uint8_t col, lv_color_t color);
};
