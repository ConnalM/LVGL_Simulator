#pragma once

#include "BaseScreen.h"
#include "../../../common/Types.h"
#include "../../../common/DebugUtils.h"
#include <array>
#include <vector>
#include <string>

// Forward declaration for MenuItemType
enum class MenuItemType {
    Dropdown,
    Toggle,
    Button,
    Label
};

/**
 * @brief Statistics screen for LVGL
 * 
 */
class StatsScreen : public BaseScreen {
private:
    static const int STD_INPUT_WIDTH = 200;  // Standard width for all input boxes
    
    // Screen elements
    lv_obj_t* container_ = nullptr;
    lv_obj_t* message_label_ = nullptr;
    bool is_initialized_ = false;

public:
    StatsScreen();
    virtual ~StatsScreen();

    // Override Show to handle lazy initialization
    virtual void Show() override;
    virtual void Hide() override;

    // Button event handlers (inherited from BaseScreen)
    virtual void OnLeftButtonClick() override;  // Return to main menu
    virtual void OnRightButtonClick() override; // Not used in config screen
    virtual void OnCenterButtonClick(); // Handle center button press

private:
    // Create the UI elements
    void CreateUI();
    
    // Create navigation buttons
    void CreateNavigationButtons();
};
