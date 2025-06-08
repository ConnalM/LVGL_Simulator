#pragma once

#include <lvgl.h>
#include <array>
#include "BaseScreen.h"
#include "../../../InputModule/GT911_TouchInput.h"
#include "../../../common/TimeManager.h"

/**
 * @brief Screen shown when race is stopped
 * 
 * Displays an alternating red light pattern across the light grid
 * to indicate the race is stopped.
 */
class StopScreen : public BaseScreen {
public:
    explicit StopScreen();
    ~StopScreen() override;

    void Show() override;
    void Hide() override;
    
    // Button click handlers
    void OnLeftButtonClick() override;  // BACK
    void OnRightButtonClick() override; // NEW RACE

private:
    // Screen elements
    std::array<std::array<lv_obj_t*, 5>, 2> lights_; // 2 rows x 5 columns
    
    // Animation state
    lv_timer_t* animation_timer_ = nullptr;
    bool current_pattern_ = false;
    
    // Light pattern methods
    static void TogglePattern(lv_timer_t* timer);
    void UpdatePattern();
    
    // Action methods
    void ReturnToMenu();
    void StartNewRace();
};
