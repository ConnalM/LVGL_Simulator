#include "PauseScreen.h"
#include "../utils/UIUtils.h"
#include "../utils/ColorUtils.h"
#include <cstdlib>
#include <ctime>
#include "../../../common/DebugUtils.h"

PauseScreen::PauseScreen() 
    : BaseScreen("PAUSED") {
    
    // Set screen background to black
    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x000000), 0);
    
    // Create navigation buttons using BaseScreen's method
    CreateNavigationButtons("RESUME", "STOP", 
                          lv_color_hex(0x00AA00), lv_color_hex(0xAA0000),
                          lv_color_hex(0x008800), lv_color_hex(0x880000));
    
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
    
    DPRINTLN("PauseScreen created");
}

PauseScreen::~PauseScreen() {
    // Clean up animation timer
    if (animation_timer_) {
        lv_timer_del(animation_timer_);
        animation_timer_ = nullptr;
    }
    DPRINTLN("PauseScreen destroyed");
}

void PauseScreen::Show() {
    DPRINTLN("Showing PauseScreen");
    
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

void PauseScreen::Hide() {
    DPRINTLN("Hiding PauseScreen");
    
    // Pause the animation timer but don't delete it
    if (animation_timer_) {
        lv_timer_pause(animation_timer_);
    }
    
    // Call base class Hide()
    BaseScreen::Hide();
}

void PauseScreen::OnLeftButtonClick() {
    DPRINTLN("Resume button clicked");
    ResumeRace();
}

void PauseScreen::OnRightButtonClick() {
    DPRINTLN("Stop button clicked");
    StopRace();
}

void PauseScreen::TogglePattern(lv_timer_t* timer) {
    if (!timer || !timer->user_data) return;
    
    PauseScreen* screen = static_cast<PauseScreen*>(timer->user_data);
    screen->current_pattern_ = !screen->current_pattern_;
    screen->UpdatePattern();
}

void PauseScreen::UpdatePattern() {
    // Pattern 1: Row 1 - odd lights, Row 2 - even lights
    // Pattern 2: Row 1 - even lights, Row 2 - odd lights
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 5; ++col) {
            if (lights_[row][col]) {
                bool should_light = (col % 2 == 0) ? 
                    (current_pattern_ ? (row == 1) : (row == 0)) : 
                    (current_pattern_ ? (row == 0) : (row == 1));
                
                lv_obj_set_style_bg_color(lights_[row][col], 
                    should_light ? lv_color_hex(0xFFFF00) : ColorUtils::AlmostBlack(), 
                    0);
            }
        }
    }
}

void PauseScreen::ResumeRace() {
    DPRINTLN("Resuming race");
    
    // Queue a ResumeRace command
    InputEvent event;
    event.command = InputCommand::ResumeRace;
    event.target = InputTarget::Race;
    event.sourceId = 0;
    event.value = 0;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    
    // Queue the event
    GT911_TouchInput::queueSystemInputEvent(event);
    
    // This screen will be hidden when the race resumes
}

void PauseScreen::StopRace() {
    DPRINTLN("Stopping race from pause screen");
    
    // Queue a StopRace command
    InputEvent event;
    event.command = InputCommand::StopRace;
    event.target = InputTarget::Race;
    event.sourceId = 0;
    event.value = 0;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    
    // Queue the event
    GT911_TouchInput::queueSystemInputEvent(event);
    
    // This screen will be hidden when the race stops
}
