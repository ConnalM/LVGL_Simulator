#include "RaceReadyScreen.h"
#include <algorithm>
#include "../../../common/DebugUtils.h"
#include <ctime>
#include "../utils/UIUtils.h"
#include "../utils/ColorUtils.h"

// Note: createStandardButton function moved to UIUtils.cpp for reuse across screens

RaceReadyScreen::RaceReadyScreen()
    : BaseScreen("READY"), // Initialize base with title
      interval_seconds_(0.5f), final_wait_type_(RANDOM), start_delay_timer_(nullptr), 
      red_timer_(nullptr), final_wait_timer_(nullptr), green_timer_(nullptr), 
      reset_timer_(nullptr), current_column_(0)
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    
    // Set screen background to black (already done by BaseScreen)
    lv_obj_set_style_bg_color(screen_, ColorUtils::Black(), 0);
    
    // Create navigation buttons with appropriate colors and callbacks
    // BaseScreen will handle the button creation and callbacks through OnLeftButtonClick/OnRightButtonClick
    CreateNavigationButtons("CANCEL", "START", 
                          ColorUtils::Red(), ColorUtils::Green(),
                          lv_color_darken(ColorUtils::Red(), LV_OPA_30), 
                          lv_color_darken(ColorUtils::Green(), LV_OPA_30));

    // Create 2 rows x 5 columns of lights (circles)
    // Using exact positioning values as specified
    int diameter = 70; // Light diameter
    int radius = diameter / 2; // Light radius (35px)
    
    // Exact spacing values as specified:
    // - First column center is 133px from left edge
    // - Each column center is 133px apart
    int first_column_center = 133;
    int column_spacing = 133;
    
    // Position the first light center at the specified position
    int start_x = first_column_center - radius; // Adjust for LV_ALIGN_TOP_LEFT
    
    // Position rows with enough room underneath
    int row_y[2] = {120, 240}; // Good vertical positioning
    
    // Debug info - uncomment if needed
    // char debug[100];
    // sprintf(debug, "Screen: %d, Margin: %d, Light: %d, Space: %d", 
    //         LV_HOR_RES, edge_margin, diameter, space_between);
    // lv_label_set_text(title_label_, debug);
    
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            lv_obj_t* light = lv_obj_create(screen_);
            lv_obj_set_size(light, diameter, diameter);
            lv_obj_set_style_radius(light, LV_RADIUS_CIRCLE, 0);
            lv_obj_align(light, LV_ALIGN_TOP_LEFT, start_x + col * column_spacing, row_y[row]);
            
            // Set greyed out appearance with proper styling to avoid artifacts
            lv_obj_set_style_bg_color(light, ColorUtils::AlmostBlack(), 0); // Off/almost black
            lv_obj_set_style_bg_opa(light, LV_OPA_COVER, 0); // Full opacity
            lv_obj_clear_flag(light, LV_OBJ_FLAG_SCROLLABLE); // Remove scrollable flag
            lv_obj_set_style_border_width(light, 0, 0); // Remove border to avoid artifacts
            
            lights_[row][col] = light;
        }
    }
    
    // We're using the navigation buttons from BaseScreen instead of custom buttons
    // The buttons are already created by BaseScreen with the labels "CANCEL" and "START"
    // The callbacks are handled by OnLeftButtonClick and OnRightButtonClick
    
    // Store references to the base screen buttons for compatibility with existing code
    return_button_ = left_button_;
    race_button_ = right_button_;
    
    // Debug output for button positions
    lv_coord_t race_x = lv_obj_get_x(race_button_);
    lv_coord_t race_y = lv_obj_get_y(race_button_);
    lv_coord_t race_w = lv_obj_get_width(race_button_);
    lv_coord_t race_h = lv_obj_get_height(race_button_);
    
    lv_coord_t return_x = lv_obj_get_x(return_button_);
    lv_coord_t return_y = lv_obj_get_y(return_button_);
    lv_coord_t return_w = lv_obj_get_width(return_button_);
    lv_coord_t return_h = lv_obj_get_height(return_button_);
    
    DPRINTF("Race button position: (%d, %d) size: %dx%d\n", 
           race_x, race_y, race_w, race_h);
    DPRINTF("Return button position: (%d, %d) size: %dx%d\n", 
           return_x, return_y, return_w, return_h);
    
    // No need for screen touch event handler - using proper LVGL button events instead
}

RaceReadyScreen::~RaceReadyScreen() {
    Hide();
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

// Helper method to check if a point is within a button's bounds
bool RaceReadyScreen::IsPointInButton(lv_obj_t* btn, lv_coord_t x, lv_coord_t y) {
    if (!btn) return false;
    
    // Get button coordinates and dimensions
    lv_coord_t btn_x = lv_obj_get_x(btn);
    lv_coord_t btn_y = lv_obj_get_y(btn);
    lv_coord_t btn_w = lv_obj_get_width(btn);
    lv_coord_t btn_h = lv_obj_get_height(btn);
    
    // Print button and touch coordinates for debugging
    Serial.println("\nCHECKING BUTTON TOUCH:");
    Serial.print("Button area: x="); Serial.print(btn_x);
    Serial.print(", y="); Serial.print(btn_y);
    Serial.print(", w="); Serial.print(btn_w);
    Serial.print(", h="); Serial.print(btn_h);
    Serial.print(" | Touch point: x="); Serial.print(x);
    Serial.print(", y="); Serial.println(y);
    
    // For bottom-positioned buttons, we need to check if y is in the bottom area of the screen
    // The buttons are positioned at y=390 with height=60, so they extend from y=390 to y=450
    // But touch coordinates might be inverted or scaled differently
    
    // Check if this is the Start button (right side)
    if (btn_x > 300) { // Assuming Start button is on the right side
        // Check if touch is in the bottom-right quadrant of the screen
        bool is_bottom_right = (x > 400 && y > 400);
        if (is_bottom_right) {
            Serial.println("TOUCH IN BOTTOM-RIGHT QUADRANT - START BUTTON AREA");
            return true;
        }
    }
    // Check if this is the Return button (left side)
    else {
        // Check if touch is in the bottom-left quadrant of the screen
        bool is_bottom_left = (x < 400 && y > 400);
        if (is_bottom_left) {
            Serial.println("TOUCH IN BOTTOM-LEFT QUADRANT - RETURN BUTTON AREA");
            return true;
        }
    }
    
    // Standard bounds check as fallback
    bool is_in = (x >= btn_x && x <= btn_x + btn_w && 
                 y >= btn_y && y <= btn_y + btn_h);
                 
    if (is_in) {
        Serial.println("TOUCH DETECTED INSIDE BUTTON BOUNDS");
    }
    
    return is_in;
}

void RaceReadyScreen::Show() {
    DPRINTLN("RaceReadyScreen::Show() - Entering");
    
    // Ensure screen_ is valid
    if (!screen_) {
        DPRINTLN("ERROR: screen_ is null in RaceReadyScreen::Show()");
        return;
    }
    
    // Call base class Show() to handle basic screen display
    DPRINTLN("Calling BaseScreen::Show()");
    BaseScreen::Show();
    
    // Ensure screen is properly loaded
    lv_obj_t* current_screen = lv_scr_act();
    if (current_screen != screen_) {
        DPRINTLN("WARNING: Current screen is not the RaceReady screen after BaseScreen::Show()");
        DPRINTF("Current screen: %p, RaceReady screen: %p\n", current_screen, screen_);
        
        // Try to force load our screen
        DPRINTLN("Forcing load of RaceReady screen");
        lv_scr_load_anim(screen_, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    } else {
        DPRINTLN("RaceReady screen is now active");
    }
    
    // Debug information
    DPRINTLN("RaceReady: Screen shown - touch buttons should be active");
    
    // Print button positions for debugging
    if (race_button_ && return_button_) {
        lv_coord_t race_x = lv_obj_get_x(race_button_);
        lv_coord_t race_y = lv_obj_get_y(race_button_);
        lv_coord_t race_w = lv_obj_get_width(race_button_);
        lv_coord_t race_h = lv_obj_get_height(race_button_);
        
        lv_coord_t return_x = lv_obj_get_x(return_button_);
        lv_coord_t return_y = lv_obj_get_y(return_button_);
        lv_coord_t return_w = lv_obj_get_width(return_button_);
        lv_coord_t return_h = lv_obj_get_height(return_button_);
        
        DPRINTF("Button positions when showing screen:\n");
        DPRINTF("Start button: x=%d, y=%d, w=%d, h=%d\n", race_x, race_y, race_w, race_h);
        DPRINTF("Return button: x=%d, y=%d, w=%d, h=%d\n", return_x, return_y, return_w, return_h);
        
        // Verify button clickable area
        if (race_w == 0 || race_h == 0) {
            DPRINTLN("WARNING: Start button has zero size!");
        }
        if (return_w == 0 || return_h == 0) {
            DPRINTLN("WARNING: Return button has zero size!");
        }
    } else {
        DPRINTLN("ERROR: Race or Return button is null!");
        if (!race_button_) DPRINTLN("Start button is null");
        if (!return_button_) DPRINTLN("Return button is null");
    }
    
    // Force a full screen refresh
    lv_refr_now(nullptr);
    DPRINTLN("RaceReadyScreen::Show() - Complete");
}

void RaceReadyScreen::Hide() {
    // Clean up any running timers when hiding the screen
    if (start_delay_timer_) {
        lv_timer_del(start_delay_timer_);
        start_delay_timer_ = nullptr;
    }
    if (red_timer_) {
        lv_timer_del(red_timer_);
        red_timer_ = nullptr;
    }
    if (reset_timer_) {
        lv_timer_del(reset_timer_);
        reset_timer_ = nullptr;
    }
    
    // Reset the current column
    current_column_ = 0;
    
    BaseScreen::Hide();  // Call base implementation
}

void RaceReadyScreen::SetInterval(float seconds) {
    interval_seconds_ = seconds;
}

void RaceReadyScreen::SetFinalWait(FinalWaitType type) {
    final_wait_type_ = type;
}

void RaceReadyScreen::SetCountdownStepCallback(CountdownStepCallback callback) {
    countdown_step_callback_ = callback;
}

void RaceReadyScreen::StartRedSequence() {
    current_column_ = 0;
    if (red_timer_) lv_timer_del(red_timer_);
    red_timer_ = lv_timer_create(RedLightStepCallback, static_cast<uint32_t>(interval_seconds_ * 1000), this);
}

void RaceReadyScreen::RedLightStepCallback(lv_timer_t* timer) {
    RaceReadyScreen* self = static_cast<RaceReadyScreen*>(timer->user_data);
    if (self->current_column_ < 5) {
        // Use our corrected Red color for all lights
        self->SetLightColor(self->current_column_, ColorUtils::Red());
        
        // Call the countdown step callback with the current step (5 - current_column)
        // This maps column 0->5, 1->4, 2->3, 3->2, 4->1
        int currentStep = 5 - self->current_column_;
        if (self->countdown_step_callback_) {
            self->countdown_step_callback_(currentStep);
        }
        
        self->current_column_++;
    }
    if (self->current_column_ >= 5) {
        lv_timer_del(self->red_timer_);
        self->red_timer_ = nullptr;
        self->StartFinalWait();
    }
}

void RaceReadyScreen::StartFinalWait() {
    uint32_t wait_ms = 0;
    if (final_wait_type_ == RANDOM) {
        wait_ms = (rand() % 3000) + 1; // 1-3000 ms
    } else {
        wait_ms = static_cast<uint32_t>(final_wait_type_) * 1000;
    }
    final_wait_timer_ = lv_timer_create(FinalWaitCallback, wait_ms, this);
    final_wait_timer_->repeat_count = 1;
}

void RaceReadyScreen::FinalWaitCallback(lv_timer_t* timer) {
    RaceReadyScreen* self = static_cast<RaceReadyScreen*>(timer->user_data);
    lv_timer_del(self->final_wait_timer_);
    self->final_wait_timer_ = nullptr;
    
    // Call the countdown step callback with 0 to indicate the final "GO!" step
    if (self->countdown_step_callback_) {
        self->countdown_step_callback_(0);
    }
    
    self->ShowGreen();
}

void RaceReadyScreen::ShowGreen() {
    // Use our ColorUtils module which handles the BGR color format correctly
    for (int col = 0; col < 5; ++col) {
        SetLightColor(col, ColorUtils::Green()); // Green color with proper BGR handling
    }
    
    // Change the title text to "GO!" in large green letters
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_48, 0); // Larger font
    lv_obj_set_style_text_color(title_label_, ColorUtils::Green(), 0); // Green color with proper BGR handling
    lv_label_set_text(title_label_, "GO!");
    
    green_timer_ = lv_timer_create(GreenStepCallback, 1000, this); // 1s
    green_timer_->repeat_count = 1;
}

void RaceReadyScreen::StartDelayCallback(lv_timer_t* timer) {
    RaceReadyScreen* screen = static_cast<RaceReadyScreen*>(timer->user_data);
    if (screen) {
        screen->ShowGreen();
    }
    lv_timer_del(timer);
}

void RaceReadyScreen::GreenStepCallback(lv_timer_t* timer) {
    RaceReadyScreen* self = static_cast<RaceReadyScreen*>(timer->user_data);
    lv_timer_del(self->green_timer_);
    self->green_timer_ = nullptr;
    
    // Log that green step is complete - SystemController will handle screen transition
    Serial.println("Green light shown - race is now active");
    
    // Send the StartRace command to the SystemController
    // This will trigger the transition to the RaceActive screen
    InputEvent event;
    event.command = InputCommand::StartRace;
    event.target = InputTarget::Race;
    event.value = 0;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    
    // Queue the event through the touch input system
    GT911_TouchInput::queueSystemInputEvent(event);
    
    // Keep the green lights visible for 1 second before hiding this screen
    self->reset_timer_ = lv_timer_create(ResetLightsCallback, 1000, self);
    self->reset_timer_->repeat_count = 1;
}

void RaceReadyScreen::ResetLightsCallback(lv_timer_t* timer) {
    RaceReadyScreen* self = static_cast<RaceReadyScreen*>(timer->user_data);
    lv_timer_del(self->reset_timer_);
    self->reset_timer_ = nullptr;
    self->ResetLights();
    
    // Hide the screen to allow SystemController to show the RaceActive screen
    self->Hide();
}

void RaceReadyScreen::ResetLights() {
    // Reset all lights to off/almost black
    for (auto& row : lights_) {
        for (auto& light : row) {
            lv_obj_set_style_bg_color(light, ColorUtils::AlmostBlack(), 0);
        }
    }
    
    // Reset the title text back to "Ready....."
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_32, 0); // Original font
    lv_obj_set_style_text_color(title_label_, ColorUtils::White(), 0); // White color
    lv_label_set_text(title_label_, "Ready.....");
    
    // Removed ReturnToPreviousScreen() call to prevent going back to previous screen
    // This allows SystemController to handle the screen transitions properly
}

void RaceReadyScreen::SetLightColor(uint8_t col, lv_color_t color) {
    // Simple implementation - directly set the color for both rows of lights
    for (int row = 0; row < 2; ++row) {
        lv_obj_set_style_bg_color(lights_[row][col], color, 0);
    }
}

void RaceReadyScreen::ReturnToPreviousScreen() {
    // Create a return to previous screen event
    InputEvent event;
    event.command = InputCommand::ReturnToPrevious;
    event.target = InputTarget::Race;
    event.sourceId = 0;
    event.value = 0;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    
    GT911_TouchInput::queueSystemInputEvent(event);
}

void RaceReadyScreen::OnLeftButtonClick()
{
    // Handle left button (CANCEL) click
    DPRINTLN("===== RaceReady: CANCEL BUTTON CLICKED =====");
    
    // First, cancel any active timers
    if (red_timer_) {
        lv_timer_del(red_timer_);
        red_timer_ = nullptr;
    }
    if (final_wait_timer_) {
        lv_timer_del(final_wait_timer_);
        final_wait_timer_ = nullptr;
    }
    if (green_timer_) {
        lv_timer_del(green_timer_);
        green_timer_ = nullptr;
    }
    
    // Call the original return to previous screen logic
    ReturnToPreviousScreen();
}

void RaceReadyScreen::OnRightButtonClick()
{
    // Handle right button (START) click
    DPRINTLN("===== RaceReady: START BUTTON CLICKED =====");
    StartRedSequence();
}
