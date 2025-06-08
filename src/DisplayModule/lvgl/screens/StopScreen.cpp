#include "StopScreen.h"
#include "../utils/UIUtils.h"
#include "../utils/ColorUtils.h"
#include <cstdlib>
#include <ctime>
#include "../../../common/DebugUtils.h"

StopScreen::StopScreen() 
    : BaseScreen("") {  // Empty string to prevent duplicate title
    
    // Set screen background to black
    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x000000), 0);
    
    // Create navigation buttons using BaseScreen's method
    CreateNavigationButtons("MENU", "NEW RACE", 
                          lv_color_hex(0xAA0000), lv_color_hex(0x00AA00),
                          lv_color_hex(0x880000), lv_color_hex(0x008800));
    
    // Add title label
    lv_obj_t* title_label = lv_label_create(screen_);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_label_set_text(title_label, "RACE RESULTS");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);
    
    // Create 2 rows x 5 columns of lights (circles)
    const int diameter = 70;  // Light diameter
    const int radius = diameter / 2;
    const int first_column_center = 133;  // First column center position
    const int column_spacing = 133;       // Space between column centers
    const int start_x = first_column_center - radius;
    const int row_y[2] = {120, 240};  // Y positions for the two rows
    
    // Initialize lights array
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            // Create a circle for each light
            lights_[row][col] = lv_obj_create(screen_);
            lv_obj_set_size(lights_[row][col], diameter, diameter);
            
            // Set object properties directly (matching RaceReadyScreen approach)
            lv_obj_set_style_radius(lights_[row][col], LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_bg_color(lights_[row][col], ColorUtils::AlmostBlack(), 0);
            lv_obj_set_style_bg_opa(lights_[row][col], LV_OPA_COVER, 0); // Full opacity
            lv_obj_clear_flag(lights_[row][col], LV_OBJ_FLAG_SCROLLABLE); // Remove scrollable flag
            lv_obj_set_style_border_width(lights_[row][col], 0, 0); // Remove border
            
            lv_obj_align(lights_[row][col], LV_ALIGN_TOP_LEFT, 
                        start_x + (col * column_spacing), row_y[row]);
        }
    }
    
    DPRINTLN("StopScreen created");
}

StopScreen::~StopScreen() {
    // Clean up animation timer
    if (animation_timer_) {
        lv_timer_del(animation_timer_);
        animation_timer_ = nullptr;
    }
    DPRINTLN("StopScreen destroyed");
}

void StopScreen::Show() {
    DPRINTLN("Showing StopScreen");
    
    // Call base class Show() to handle basic setup
    BaseScreen::Show();
    
    // Set initial pattern
    current_pattern_ = false;
    UpdatePattern();
    
    // Create animation timer if it doesn't exist
    if (!animation_timer_) {
        animation_timer_ = lv_timer_create(TogglePattern, 500, this); // Toggle every 500ms
    } else {
        lv_timer_resume(animation_timer_);
    }
}

void StopScreen::Hide() {
    DPRINTLN("Hiding StopScreen");
    
    // Pause the animation timer but don't delete it
    if (animation_timer_) {
        lv_timer_pause(animation_timer_);
    }
    
    // Call base class Hide()
    BaseScreen::Hide();
}

void StopScreen::OnLeftButtonClick() {
    DPRINTLN("Menu button clicked");
    ReturnToMenu();
}

void StopScreen::OnRightButtonClick() {
    DPRINTLN("New Race button clicked");
    StartNewRace();
}

void StopScreen::TogglePattern(lv_timer_t* timer) {
    if (!timer || !timer->user_data) return;
    
    StopScreen* screen = static_cast<StopScreen*>(timer->user_data);
    screen->current_pattern_ = !screen->current_pattern_;
    screen->UpdatePattern();
}

void StopScreen::UpdatePattern() {
    // Pattern alternates between two states:
    // - Pattern 0: Odd columns (1,3,5) are red in both rows, Even columns (2,4) are black
    // - Pattern 1: Even columns (2,4) are red in both rows, Odd columns (1,3,5) are black
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (lights_[row][col]) {
                // Columns start at 0, so 0,2,4 are odd positions (1,3,5 in 1-based)
                bool is_odd_column = (col % 2 == 0);
                
                // For pattern 0: Light up odd columns (1,3,5) in both rows
                // For pattern 1: Light up even columns (2,4) in both rows
                bool should_light = (current_pattern_ == 0) ? is_odd_column : !is_odd_column;
                
                lv_obj_set_style_bg_color(lights_[row][col], 
                    should_light ? lv_color_hex(0xFF0000) : ColorUtils::AlmostBlack(), 
                    0);
            }
        }
    }
}

void StopScreen::ReturnToMenu() {
    DPRINTLN("Returning to main menu");
    
    // Queue a ReturnToPrevious command to go back to main menu
    InputEvent event;
    event.command = InputCommand::ReturnToPrevious;
    event.target = InputTarget::Race;
    event.sourceId = 0;
    event.value = 0;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    
    // Queue the event
    GT911_TouchInput::queueSystemInputEvent(event);
}

void StopScreen::StartNewRace() {
    DPRINTLN("Starting new race");
    
    // Queue a EnterRaceReady command to start a new race
    InputEvent event;
    event.command = InputCommand::EnterRaceReady;
    event.target = InputTarget::Race;
    event.sourceId = 0;
    event.value = 0;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    
    // Queue the event
    GT911_TouchInput::queueSystemInputEvent(event);
}
