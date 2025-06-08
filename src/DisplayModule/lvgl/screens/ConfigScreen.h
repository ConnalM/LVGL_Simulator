#pragma once

#include "BaseScreen.h"
#include "../../../common/Types.h"
#include <array>
#include <vector>
#include <string>

/**
 * @brief Configuration menu screen for LVGL
 * 
 * Provides a menu interface for configuring race settings:
 * - Number of laps
 * - Number of lanes
 * - Race mode
 * - Race time
 * - Reaction time toggle
 * - Lane enable/disable
 * - Racer management
 */
class ConfigScreen : public BaseScreen {
private:
    static const int STD_INPUT_WIDTH;  // Standard width for all input boxes
    static ConfigScreen* instance_;    // Static instance pointer for button callbacks
    
public:
    ConfigScreen();
    virtual ~ConfigScreen();

    // Override Show to handle lazy initialization
    virtual void Show() override;

    // Button event handlers (inherited from BaseScreen)
    virtual void OnLeftButtonClick() override;  // Return to main menu
    virtual void OnRightButtonClick() override; // Not used in config screen

private:
    // Menu items and dropdowns
    lv_obj_t* menu_container_;
    bool is_initialized_;
    enum class MenuItemType {
        Dropdown,
        Spinbox
    };

    struct MenuItem {
        std::string label_text;
        std::vector<std::string> options;
        MenuItemType type;
        lv_obj_t* label = nullptr;
        union {
            lv_obj_t* dropdown;
            lv_obj_t* spinbox;
        };

        MenuItem(const std::string& text, const std::vector<std::string>& opts, MenuItemType t = MenuItemType::Dropdown)
            : label_text(text), options(opts), type(t) {
            dropdown = nullptr;
        }
    };

    static void OnSpinboxEvent(lv_event_t* e);
    std::vector<MenuItem> menu_items_;

public:
    lv_obj_t* getScreen() { return screen_; }
    
    // Create the configuration menu items
    void CreateMenuItems();
    
    // Dropdown event handler
    static void OnDropdownEvent(lv_event_t* e);
    void HandleDropdownSelection(size_t menuIndex, uint16_t selectedIndex);
    
    // Helper to create a dropdown with options
    lv_obj_t* CreateDropdown(const char* label, const std::vector<std::string>& options);
};
