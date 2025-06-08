#include "ConfigScreen.h"
#include "../../../common/DebugUtils.h"
#include "../../../InputModule/GT911_TouchInput.h"
#include "../../../common/TimeManager.h"
#include "../../../common/Types.h"

// Initialize static member
ConfigScreen* ConfigScreen::instance_ = nullptr;

// Standard width for all input boxes
const int ConfigScreen::STD_INPUT_WIDTH = 120;

ConfigScreen::ConfigScreen() 
    : BaseScreen("CONFIGURATION")
    , menu_container_(nullptr)
    , is_initialized_(false)
{
    // Set the static instance
    instance_ = this;
    
    // Initialize the screen object
    if (!screen_) {
        DPRINTLN("ERROR: Failed to create base screen in ConfigScreen constructor");
        return;
    }
    
    // Set black background
    lv_obj_set_style_bg_color(screen_, lv_color_black(), 0);
    
    DPRINTLN("ConfigScreen constructor completed");
}

void ConfigScreen::Show() {
    DPRINTLN("ConfigScreen::Show() - Entering");
    
    // Call base class Show() first to ensure screen is properly set up
    DPRINTLN("Calling BaseScreen::Show()");
    BaseScreen::Show();
    
    // Ensure screen_ is valid
    if (!screen_) {
        DPRINTLN("ERROR: screen_ is null in ConfigScreen::Show()");
        return;
    }
    
    // Initialize on first show
    if (!is_initialized_) {
        DPRINTLN("Initializing ConfigScreen UI components");
        
        // Initialize the container and menu items on first show
        menu_container_ = content_container_;
        if (!menu_container_) {
            DPRINTLN("ERROR: content_container_ is null in ConfigScreen::Show()");
            return;
        }
        
        // Set flex properties specific to ConfigScreen
        lv_obj_set_flex_flow(menu_container_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(menu_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        
        // Create navigation buttons
        DPRINTLN("Creating navigation buttons");
        CreateNavigationButtons(
            "Back",                          // Left button text
            "Start",                         // Right button text
            lv_color_make(204, 0, 0),       // Red for back button
            lv_color_make(0, 204, 0),       // Green for start button
            lv_color_make(153, 0, 0),       // Dark red for pressed
            lv_color_make(0, 153, 0)        // Dark green for right pressed
        );
        
        // Create menu items
        DPRINTLN("Creating ConfigScreen menu items");
        CreateMenuItems();
        
        is_initialized_ = true;
        DPRINTLN("ConfigScreen initialization complete");
    }
    
    // Ensure screen is properly loaded
    lv_obj_t* current_screen = lv_scr_act();
    if (current_screen != screen_) {
        DPRINTLN("WARNING: Current screen is not the ConfigScreen after BaseScreen::Show()");
        DPRINTF("Current screen: %p, ConfigScreen: %p\n", current_screen, screen_);
        
        // Try to force load our screen
        DPRINTLN("Forcing load of ConfigScreen");
        lv_scr_load_anim(screen_, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
    } else {
        DPRINTLN("ConfigScreen is now active");
    }
    
    // Force a full screen refresh
    lv_refr_now(nullptr);
    DPRINTLN("ConfigScreen::Show() - Complete");
}

ConfigScreen::~ConfigScreen() {
    // Clear the static instance if it points to this object
    if (instance_ == this) {
        instance_ = nullptr;
    }
    
    // Clean up any LVGL objects
    if (menu_container_) {
        lv_obj_del(menu_container_);
        menu_container_ = nullptr;
    }
    
    DPRINTLN("ConfigScreen destroyed");
}

lv_obj_t* ConfigScreen::CreateDropdown(const char* label, const std::vector<std::string>& options) {
    lv_obj_t* dropdown = lv_dropdown_create(menu_container_);
    
    // Set dropdown size and style
    lv_obj_set_size(dropdown, 150, LV_SIZE_CONTENT);
    lv_obj_set_style_text_color(dropdown, ColorUtils::White(), 0);
    
    // Combine all options into a single string with \n separator
    std::string options_str;
    for (size_t i = 0; i < options.size(); ++i) {
        options_str += options[i];
        if (i < options.size() - 1) options_str += "\n";
    }
    
    // Set the options
    lv_dropdown_set_options(dropdown, options_str.c_str());
    
    // Style the dropdown list
    lv_obj_t* list = lv_dropdown_get_list(dropdown);
    lv_obj_set_style_bg_color(list, ColorUtils::Black(), LV_PART_MAIN);
    lv_obj_set_style_border_color(list, ColorUtils::White(), LV_PART_MAIN);
    lv_obj_set_style_text_color(list, ColorUtils::White(), LV_PART_MAIN);
    
    return dropdown;
}

void ConfigScreen::CreateMenuItems() {
    // Define menu items and their options
    struct MenuDef {
        const char* label;
        std::vector<std::string> options;
    };
    
    std::vector<MenuItem> items = {
        {"Number of Laps", {}, MenuItemType::Spinbox},
        {"Number of Lanes", {"1", "2", "3", "4", "5", "6", "7", "8"}},
        {"Race Mode", {"LAPS", "TIMER", "DRAG", "RALLY", "PRACTISE"}},
        {"Race Time", {}, MenuItemType::Spinbox},
        {"Reaction Time", {"Before", "After"}},
    };

    // Create each menu item with its dropdown
    menu_items_ = items;
    for (size_t i = 0; i < menu_items_.size(); ++i) {
        auto& item = menu_items_[i];
        
        // Create container for the entire row
        lv_obj_t* row = lv_obj_create(menu_container_);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 50); // Increased height for better touch targets
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
        lv_obj_set_style_pad_bottom(row, 5, 0); // Add some space between rows
        
        // Create and position the label at x=80, with vertical alignment matching input boxes
        item.label = lv_label_create(row);
        lv_label_set_text(item.label, item.label_text.c_str());
        lv_obj_set_style_text_color(item.label, ColorUtils::White(), 0);
        lv_obj_set_style_text_font(item.label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_pad_top(item.label, 5, 0);  // Match input box text vertical position
        lv_obj_set_pos(item.label, 80, 5);  // Move down by 5 pixels
        
        // Create a container for the input control at x=300
        lv_obj_t* input_cont = lv_obj_create(row);
        lv_obj_remove_style_all(input_cont);
        lv_obj_set_size(input_cont, 300, lv_pct(100));
        lv_obj_set_pos(input_cont, 300, 0);
        lv_obj_set_style_pad_all(input_cont, 0, 0);
        lv_obj_set_style_bg_opa(input_cont, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(input_cont, 0, 0);
        lv_obj_set_flex_flow(input_cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(input_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        if (item.type == MenuItemType::Dropdown) {
            // Create dropdown in the input container
            item.dropdown = lv_dropdown_create(input_cont);
            lv_dropdown_clear_options(item.dropdown);
            
            // Style the dropdown
            lv_obj_set_size(item.dropdown, 100, 30);  // Set width to 100px
            lv_obj_align(item.dropdown, LV_ALIGN_LEFT_MID, 0, 0);
            
            // Style the dropdown button
            lv_obj_set_style_text_color(item.dropdown, ColorUtils::White(), 0);
            lv_obj_set_style_text_font(item.dropdown, &lv_font_montserrat_16, 0);
            lv_obj_set_style_bg_color(item.dropdown, ColorUtils::Black(), 0);
            lv_obj_set_style_bg_color(item.dropdown, lv_color_black(), LV_PART_MAIN);
            lv_obj_set_style_border_color(item.dropdown, ColorUtils::White(), 0);
            lv_obj_set_style_border_width(item.dropdown, 1, 0);
            lv_obj_set_style_pad_top(item.dropdown, 5, 0);
            // Center text with arrow on right
            lv_obj_set_style_pad_left(item.dropdown, 20, 0);
            lv_obj_set_style_pad_right(item.dropdown, 30, 0); // Room for arrow
            lv_obj_set_style_text_align(item.dropdown, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_align(item.dropdown, LV_TEXT_ALIGN_RIGHT, LV_PART_INDICATOR);
            // Position arrow on right
            lv_obj_set_style_pad_right(item.dropdown, 5, LV_PART_INDICATOR);
            lv_obj_set_style_width(item.dropdown, 20, LV_PART_INDICATOR);
            
            // Style the dropdown list
            lv_obj_set_style_bg_color(lv_dropdown_get_list(item.dropdown), lv_color_black(), 0);
            lv_obj_set_style_border_color(lv_dropdown_get_list(item.dropdown), ColorUtils::White(), 0);
            lv_obj_set_style_text_color(lv_dropdown_get_list(item.dropdown), ColorUtils::White(), 0);
            lv_obj_set_style_text_font(lv_dropdown_get_list(item.dropdown), &lv_font_montserrat_16, 0);
            
            // Populate dropdown with options from the MenuItem
            for (const auto& option : item.options) {
                lv_dropdown_add_option(item.dropdown, option.c_str(), LV_DROPDOWN_POS_LAST);
            }
            
            // Store the menu item index in the dropdown's user data
            lv_obj_set_user_data(item.dropdown, (void*)i);
            
            // Register event handler for value changes
            lv_obj_add_event_cb(item.dropdown, OnDropdownEvent, LV_EVENT_VALUE_CHANGED, this);
        } else if (item.type == MenuItemType::Spinbox) {
            // Create a numeric spinbox input (e.g., for lap count or race time)
            item.spinbox = lv_spinbox_create(input_cont);
            
            // Configure spinbox numeric properties based on the label
            if (item.label_text == "Race Time") {
                // Race Time: 0-5999 seconds (99:59)
                lv_spinbox_set_range(item.spinbox, 0, 5999);
                lv_spinbox_set_digit_format(item.spinbox, 4, 0); // 4 digits (e.g., 0123)
                lv_spinbox_set_value(item.spinbox, 60); // Default to 1 minute
            } else {
                // Default (Number of Laps): 1-999
                lv_spinbox_set_range(item.spinbox, 1, 999);
                lv_spinbox_set_digit_format(item.spinbox, 3, 0); // 3 digits (e.g., 010)
                lv_spinbox_set_value(item.spinbox, 10); // Default to 10 laps
            }
            
            lv_spinbox_set_step(item.spinbox, 1); // Increment/decrement by 1
            
            // Style the spinbox to match other inputs
            lv_obj_set_size(item.spinbox, 100, 30); // Set fixed height to match dropdowns
            lv_obj_set_style_text_color(item.spinbox, ColorUtils::White(), 0);
            lv_obj_set_style_text_font(item.spinbox, &lv_font_montserrat_16, 0);
            lv_obj_set_style_bg_color(item.spinbox, ColorUtils::Black(), 0);
            lv_obj_set_style_border_color(item.spinbox, ColorUtils::White(), 0);
            lv_obj_set_style_border_width(item.spinbox, 1, 0);
            lv_obj_set_style_text_align(item.spinbox, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            lv_obj_set_style_text_align(item.spinbox, LV_TEXT_ALIGN_CENTER, LV_PART_CURSOR);
            lv_obj_set_style_pad_top(item.spinbox, 5, 0);
            // Center the text by adjusting padding
            lv_obj_set_style_pad_left(item.spinbox, 10, 0);
            lv_obj_set_style_pad_right(item.spinbox, 10, 0);
            
            // Store menu index for event handling
            lv_obj_set_user_data(item.spinbox, (void*)i);
            
            // Register event handler for value changes
            // OnSpinboxEvent will be called when value changes
            lv_obj_add_event_cb(item.spinbox, OnSpinboxEvent, LV_EVENT_VALUE_CHANGED, this);
            
            // Position the spinbox within the input container
            lv_obj_set_width(item.spinbox, 100); // Set a fixed width for the spinbox
            
            // Create a container for the buttons to group them together
            lv_obj_t* btn_cont = lv_obj_create(input_cont);
            lv_obj_remove_style_all(btn_cont);
            lv_obj_set_size(btn_cont, 80, 30); // Width for both buttons + gap
            lv_obj_set_style_bg_opa(btn_cont, LV_OPA_TRANSP, 0);
            lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            
            // Create - button (decrement)
            lv_obj_t* btn_dec = lv_btn_create(btn_cont);
            lv_obj_set_size(btn_dec, 30, 30);
            lv_obj_set_style_bg_color(btn_dec, ColorUtils::Black(), 0);
            lv_obj_set_style_border_color(btn_dec, ColorUtils::White(), 0);
            lv_obj_set_style_border_width(btn_dec, 1, 0);
            lv_obj_set_style_radius(btn_dec, 0, 0);
            
            // Add "-" label to button
            lv_obj_t* label_dec = lv_label_create(btn_dec);
            lv_label_set_text(label_dec, LV_SYMBOL_MINUS);
            lv_obj_center(label_dec);
            lv_obj_set_style_text_font(label_dec, &lv_font_montserrat_16, 0);
            
            // Create + button (increment)
            lv_obj_t* btn_inc = lv_btn_create(btn_cont);
            lv_obj_set_size(btn_inc, 30, 30);
            lv_obj_set_style_bg_color(btn_inc, ColorUtils::Black(), 0);
            lv_obj_set_style_border_color(btn_inc, ColorUtils::White(), 0);
            lv_obj_set_style_border_width(btn_inc, 1, 0);
            lv_obj_set_style_radius(btn_inc, 0, 0);
            
            // Add "+" label to button
            lv_obj_t* label_inc = lv_label_create(btn_inc);
            lv_label_set_text(label_inc, LV_SYMBOL_PLUS);
            lv_obj_center(label_inc);
            lv_obj_set_style_text_font(label_inc, &lv_font_montserrat_16, 0);
            
            // Position the button container to place - at 420px and + at 480px
            lv_obj_align(btn_cont, LV_ALIGN_LEFT_MID, 120, 0); // 300 (input_cont x) + 120 = 420px for -
            // + button will be at 420 + 30 (button width) + 30 (gap) = 480px
            
            // Add event handlers for buttons
            lv_obj_add_event_cb(btn_dec, [](lv_event_t* e) {
                lv_obj_t* spinbox = (lv_obj_t*)lv_event_get_user_data(e);
                lv_spinbox_decrement(spinbox);
            }, LV_EVENT_CLICKED, item.spinbox);
            
            lv_obj_add_event_cb(btn_inc, [](lv_event_t* e) {
                lv_obj_t* spinbox = (lv_obj_t*)lv_event_get_user_data(e);
                lv_spinbox_increment(spinbox);
            }, LV_EVENT_CLICKED, item.spinbox);
        }
    }
}

void ConfigScreen::OnDropdownEvent(lv_event_t* e) {
    ConfigScreen* screen = static_cast<ConfigScreen*>(lv_event_get_user_data(e));
    if (!screen) return;
    
    // Get the dropdown that triggered the event
    lv_obj_t* dropdown = lv_event_get_target(e);
    if (!dropdown) return;
    
    // Get the menu index from user data
    size_t menuIndex = (size_t)lv_obj_get_user_data(dropdown);
    
    // Get selected option index
    uint16_t selectedIndex = lv_dropdown_get_selected(dropdown);
    
    // Handle the selection
    screen->HandleDropdownSelection(menuIndex, selectedIndex);
}

void ConfigScreen::HandleDropdownSelection(size_t menuIndex, uint16_t selectedIndex) {
    // Get the selected option text
    const std::string& selectedOption = menu_items_[menuIndex].options[selectedIndex];
    
    // Create appropriate input event based on selection
    InputEvent event;
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = InputTarget::Race;
    
    switch(menuIndex) {
        case 0: // Number of Laps
            event.command = InputCommand::SetNumLaps;
            event.value = std::stoi(selectedOption);
            break;
            
        case 1: // Number of Lanes
            event.command = InputCommand::SetNumLanes;
            event.value = std::stoi(selectedOption);
            break;
            
        case 2: // Race Mode
            event.command = InputCommand::ChangeMode;
            if (selectedOption == "Standard") event.value = static_cast<int>(RaceMode::LAPS);
            else if (selectedOption == "Time Trial") event.value = static_cast<int>(RaceMode::TIMER);
            else if (selectedOption == "Practice") event.value = static_cast<int>(RaceMode::PRACTISE);
            else if (selectedOption == "Rally") event.value = static_cast<int>(RaceMode::RALLY);
            break;
            
        case 3: // Race Time
            event.command = InputCommand::SetRaceTime;
            // Extract number from "X min" and convert to seconds
            event.value = std::stoi(selectedOption) * 60; // Convert to seconds
            break;
            
        case 4: // Reaction Time
            event.command = InputCommand::ToggleReactionTime;
            event.value = (selectedOption == "On") ? 1 : 0;
            break;
            
        case 5: // Lane Status
            // Use EnableLane/DisableLane commands
            if (selectedOption == "All Enabled") {
                // Enable all lanes
                for (int lane = 0; lane < MAX_LANES; lane++) {
                    InputEvent laneEvent;
                    laneEvent.command = InputCommand::EnableLane;
                    laneEvent.value = lane;
                    laneEvent.sourceId = static_cast<int>(InputSourceId::TOUCH);
                    laneEvent.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
                    laneEvent.target = InputTarget::Config;
                    GT911_TouchInput::queueSystemInputEvent(laneEvent);
                }
                return; // Skip the main event queue
            }
            // Custom will be handled by a separate dialog
            break;
            
        case 6: // Racer Setup
            if (selectedOption == "Add Racer") event.command = InputCommand::AddRacer;
            else if (selectedOption == "Remove Racer") event.command = InputCommand::RemoveRacer;
            // Edit Names not supported in current InputCommand enum
            break;
    }
    
    // Queue the event
    GT911_TouchInput::queueSystemInputEvent(event);
}

void ConfigScreen::OnSpinboxEvent(lv_event_t* e) {
    ConfigScreen* screen = static_cast<ConfigScreen*>(lv_event_get_user_data(e));
    if (!screen) return;
    
    // Get the spinbox that triggered the event
    lv_obj_t* spinbox = lv_event_get_target(e);
    if (!spinbox) return;
    
    // Get the menu index from user data
    size_t menuIndex = (size_t)lv_obj_get_user_data(spinbox);
    if (menuIndex >= screen->menu_items_.size()) return;
    
    // Get the menu item
    const auto& item = screen->menu_items_[menuIndex];
    
    // Get selected value
    int32_t value = lv_spinbox_get_value(spinbox);
    
    // Create input event
    InputEvent event;
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = InputTarget::Config;
    
    // Set the appropriate command based on the menu item
    if (item.label_text == "Number of Laps") {
        event.command = InputCommand::SetNumLaps;
    } else if (item.label_text == "Race Time") {
        event.command = InputCommand::SetRaceTime;
        // Convert seconds to milliseconds for the race module
        value *= 1000;
    } else {
        return; // Unknown spinbox type
    }
    
    event.value = value;
    
    // Queue the event
    GT911_TouchInput::queueSystemInputEvent(event);
}

void ConfigScreen::OnLeftButtonClick() {
    DPRINTLN("Config screen: Back button pressed");
    
    // Use the static instance to ensure we have a valid 'this' pointer
    if (!instance_) {
        DPRINTLN("ERROR: No ConfigScreen instance in OnLeftButtonClick");
        return;
    }
    
    DPRINTLN("Creating return to previous event");
    
    // Create InputEvent for returning to main menu
    InputEvent event;
    event.command = InputCommand::ReturnToPrevious;
    event.value = 0;
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = InputTarget::Race;
    
    DPRINTLN("Queueing input event");
    
    // Queue the input event through the input manager
    GT911_TouchInput::queueSystemInputEvent(event);
    DPRINTLN("Queued ReturnToPrevious event");
}

void ConfigScreen::OnRightButtonClick() {
    DPRINTLN("Config screen: Start button pressed");
    
    // Use the static instance to ensure we have a valid 'this' pointer
    if (!instance_) {
        DPRINTLN("ERROR: No ConfigScreen instance in OnRightButtonClick");
        return;
    }
    
    DPRINTLN("Creating start countdown event");
    
    // Create InputEvent for starting race
    InputEvent event;
    event.command = InputCommand::StartCountdown;
    event.value = 0;
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = InputTarget::Race;
    
    DPRINTLN("Queueing input event");
    
    // Queue the input event through the input manager
    GT911_TouchInput::queueSystemInputEvent(event);
    DPRINTLN("Queued StartCountdown event");
}
