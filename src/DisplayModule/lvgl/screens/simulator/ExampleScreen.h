#pragma once

#include <lvgl.h>

/**
 * @brief Class to manage the example screen for the LVGL simulator
 */
class ExampleScreen {
public:
    /**
     * @brief Create and show the example screen
     * 
     * @return true if successful
     * @return false if failed
     */
    static bool show();
    
    /**
     * @brief Hide the example screen
     */
    static void hide();
    
    /**
     * @brief Update the example screen
     */
    static void update();
    
private:
    static lv_obj_t* _label;
    static lv_obj_t* _btn;
    static lv_obj_t* _btnLabel;
    
    /**
     * @brief Button click event handler
     * 
     * @param e Event data
     */
    static void btnClickHandler(lv_event_t* e);
};
