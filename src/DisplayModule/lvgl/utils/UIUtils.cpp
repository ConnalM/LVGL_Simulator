#include "UIUtils.h"

// Helper function to create a standardized button - can be reused across the program
lv_obj_t* createStandardButton(lv_obj_t* parent, const char* label_text, 
                              lv_coord_t x_pos, lv_coord_t y_pos, 
                              lv_coord_t width, lv_coord_t height, 
                              lv_color_t bg_color, lv_color_t pressed_color,
                              lv_color_t text_color) {
    // Create button object
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    lv_obj_set_pos(btn, x_pos, y_pos);
    
    // Set button styles with custom colors
    lv_obj_set_style_bg_color(btn, bg_color, LV_PART_MAIN | LV_STATE_DEFAULT); // Custom background
    lv_obj_set_style_bg_color(btn, pressed_color, LV_PART_MAIN | LV_STATE_PRESSED); // Darker when pressed
    lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT); // White border
    lv_obj_set_style_radius(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT); // Rounded corners
    lv_obj_set_style_shadow_width(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT); // Add shadow
    lv_obj_set_style_shadow_opa(btn, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Create and configure label
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_color(label, text_color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label);
    
    // Make sure button is clickable
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    
    return btn;
}
