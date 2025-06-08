#include "StatsScreen.h"
#include "../../../common/DebugUtils.h"
#include "../utils/ColorUtils.h"

// Debug output macros
#ifndef DEBUG_PRINT_METHOD
#define DEBUG_PRINT_METHOD()
#endif

StatsScreen::StatsScreen() : BaseScreen("Statistics") {
    // Initialize any members here
}

StatsScreen::~StatsScreen() {
    // Clean up any resources if needed
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
}

void StatsScreen::Show() {
    if (!is_initialized_) {
        CreateUI();
        is_initialized_ = true;
    }
    lv_scr_load_anim(screen_, LV_SCR_LOAD_ANIM_NONE, 300, 0, false);
    BaseScreen::Show();
}

void StatsScreen::Hide() {
    // Clean up resources if needed
    BaseScreen::Hide();
}

void StatsScreen::CreateUI() {
    // Create the main container
    container_ = lv_obj_create(screen_);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_black(), 0);
    lv_obj_set_style_pad_all(container_, 0, 0);

    // Title is already created by BaseScreen
    // Add message
    message_label_ = lv_label_create(container_);
    lv_label_set_text(message_label_, "Dummy Stats Page\nComing Soon");
    lv_obj_set_style_text_font(message_label_, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(message_label_, ColorUtils::White(), 0);
    lv_obj_set_style_text_align(message_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(message_label_);

    // Create navigation buttons
    CreateNavigationButtons();
}

void StatsScreen::CreateNavigationButtons() {
    // Call the base class method with appropriate button labels and colors
    BaseScreen::CreateNavigationButtons(
        LV_SYMBOL_LEFT " Back", 
        "Next",
        lv_palette_darken(LV_PALETTE_GREY, 2),  // Left button color
        lv_palette_main(LV_PALETTE_BLUE),       // Right button color
        lv_palette_darken(LV_PALETTE_GREY, 3),  // Left pressed color
        lv_palette_darken(LV_PALETTE_BLUE, 2)   // Right pressed color
    );
}

void StatsScreen::OnLeftButtonClick() {
    // Handle left button press (e.g., go to previous screen)
    DEBUG_DETAIL(F("StatsScreen: Left button pressed"));
    // Example: Go back to main menu
    // DisplayManager::getInstance().setScreen(ScreenType::Main);
}

void StatsScreen::OnRightButtonClick() {
    // Handle right button press (e.g., go to next screen)
    DEBUG_DETAIL(F("StatsScreen: Right button pressed"));
}

void StatsScreen::OnCenterButtonClick() {
    // Handle center button press (e.g., select/confirm)
    DEBUG_DETAIL(F("StatsScreen: Center button pressed"));
}
