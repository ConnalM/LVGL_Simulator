#include "ExampleScreen.h"

// Initialize static members
lv_obj_t* ExampleScreen::_label = NULL;
lv_obj_t* ExampleScreen::_btn = NULL;
lv_obj_t* ExampleScreen::_btnLabel = NULL;

bool ExampleScreen::show() {
    // Create a simple label
    _label = lv_label_create(lv_scr_act());
    lv_label_set_text(_label, "Hello LVGL Simulator!");
    lv_obj_align(_label, LV_ALIGN_CENTER, 0, 0);

    // Create a button
    _btn = lv_btn_create(lv_scr_act());
    lv_obj_align(_btn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(_btn, btnClickHandler, LV_EVENT_CLICKED, _label);

    // Add label to button
    _btnLabel = lv_label_create(_btn);
    lv_label_set_text(_btnLabel, "Click Me!");
    lv_obj_center(_btnLabel);
    
    return true;
}

void ExampleScreen::hide() {
    // Delete objects
    if (_btn) {
        lv_obj_del(_btn);
        _btn = NULL;
        _btnLabel = NULL; // Deleted automatically as child of _btn
    }
    
    if (_label) {
        lv_obj_del(_label);
        _label = NULL;
    }
}

void ExampleScreen::btnClickHandler(lv_event_t* e) {
    static int count = 0;
    count++;
    lv_obj_t* label = (lv_obj_t*)e->user_data;
    lv_label_set_text_fmt(label, "Clicked: %d", count);
}
