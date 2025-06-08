#pragma once

#include <lvgl.h>
#include <functional>
#include "../../../common/DebugUtils.h"
#include "../utils/UIUtils.h"
#include "../utils/ColorUtils.h"
#include "../../../InputModule/GT911_TouchInput.h"
#include "../../../common/TimeManager.h"

/**
 * @brief Base class for all LVGL screens in the Display4 project
 * 
 * This class provides common functionality and layout for all screens:
 * - Title area at top
 * - Content area in middle
 * - Navigation buttons at bottom
 * - Standard styling and colors
 */
class BaseScreen {
protected:
    static BaseScreen* current_instance_;  // Current active instance for button callbacks
    
public:
    BaseScreen(const char* title);
    virtual ~BaseScreen();

    // Screen lifecycle
    virtual void Show();
    virtual void Hide();
    
    // Screen elements
    void SetTitle(const char* title);
    
    // Get the screen object
    lv_obj_t* getScreen() const { return screen_; }
    
protected:
    // LVGL objects
    lv_obj_t* screen_;        // Main screen object
    lv_obj_t* title_label_;   // Title text
    lv_obj_t* content_area_;  // Middle content area
    lv_obj_t* content_container_; // Container for screen content
    lv_obj_t* left_button_;   // Usually Return/Back
    lv_obj_t* right_button_;  // Action button (context dependent)
    
    // Create the basic screen layout
    void CreateScreenLayout();
    
    // Helper to create and position buttons
    void CreateNavigationButtons(const char* left_text, const char* right_text,
                               lv_color_t left_color, lv_color_t right_color,
                               lv_color_t left_pressed, lv_color_t right_pressed);
    
    // Button event handlers (to be implemented by derived classes)
    virtual void OnLeftButtonClick() = 0;
    virtual void OnRightButtonClick() = 0;
    
private:
    // Static callback handlers for LVGL
    static void LeftButtonCallback(lv_event_t* e);
    static void RightButtonCallback(lv_event_t* e);
    
    /**
     * @brief Creates a debug grid with ticks and labels
     * @param parent The parent object to add the grid to
     */
    void CreateDebugGrid(lv_obj_t* parent);

    // Screen dimensions and layout constants
    static constexpr int TITLE_HEIGHT = 60;
    static constexpr int BUTTON_HEIGHT = 60;
    static constexpr int BUTTON_WIDTH = 150;
    static constexpr int BUTTON_SPACING = 100;
    static constexpr int BOTTOM_MARGIN = 30;
    static constexpr int CONTENT_PADDING = 10;
    static constexpr int CONTENT_ROW_SPACING = 12;
};
