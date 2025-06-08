#include "BaseScreen.h"
#include "../utils/ColorUtils.h"
#include <lvgl.h>
#include <string>

// Initialize static member
BaseScreen* BaseScreen::current_instance_ = nullptr;

BaseScreen::BaseScreen(const char* title)
    : screen_(nullptr)
    , title_label_(nullptr)
    , content_area_(nullptr)
    , left_button_(nullptr)
    , right_button_(nullptr)
{
    // Create the basic screen layout
    CreateScreenLayout();
    
    // Set the title
    SetTitle(title);
}

BaseScreen::~BaseScreen() {
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

void BaseScreen::CreateScreenLayout() {
    // Create main screen
    screen_ = lv_obj_create(NULL);
    lv_obj_set_size(screen_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_, ColorUtils::Black(), 0);

    // Create title label - centered in 70px header
    title_label_ = lv_label_create(screen_);
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_32, 0);
    lv_obj_set_style_text_color(title_label_, ColorUtils::White(), 0);
    
    // Get the height of the text to center it properly
    lv_point_t text_size;
    lv_txt_get_size(&text_size, "CONFIGURATION", &lv_font_montserrat_32, 0, 0, LV_COORD_MAX, 0);
    
    // Center the title in the 70px header
    int title_y = (70 - text_size.y) / 2;
    lv_obj_align(title_label_, LV_ALIGN_TOP_MID, 0, title_y);

    // Create content area (acts as a frame)
    content_area_ = lv_obj_create(screen_);
    lv_obj_remove_style_all(content_area_);  // Start with clean styling
    
    // Position content area below title and above buttons
    lv_obj_set_size(content_area_, LV_HOR_RES, 
                    LV_VER_RES - TITLE_HEIGHT - BUTTON_HEIGHT - BOTTOM_MARGIN);
    lv_obj_align_to(content_area_, title_label_, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    
    // Set content area styling (just a frame)
    lv_obj_set_style_bg_opa(content_area_, LV_OPA_0, 0);  // Transparent background
    lv_obj_set_style_border_width(content_area_, 0, 0);   // No border for the frame
    
    // Create content container that will hold the actual screen content
    content_container_ = lv_obj_create(content_area_);
    lv_obj_remove_style_all(content_container_);
    lv_obj_set_size(content_container_, lv_pct(100), lv_pct(100));
    lv_obj_align(content_container_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_opa(content_container_, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(content_container_, CONTENT_PADDING, LV_PART_MAIN);
    lv_obj_set_style_pad_row(content_container_, CONTENT_ROW_SPACING, LV_PART_MAIN);
    lv_obj_set_flex_flow(content_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scrollbar_mode(content_container_, LV_SCROLLBAR_MODE_AUTO);

    // Add debug borders for layout visualization
    // Header border (0-70)
    lv_obj_t* header_border = lv_obj_create(screen_);
    lv_obj_set_size(header_border, LV_HOR_RES, 70);
    lv_obj_align(header_border, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_border_width(header_border, 2, 0);
    lv_obj_set_style_border_color(header_border, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(header_border, LV_OPA_0, 0);  // Transparent background

    // Content border (70-400)
    lv_obj_t* content_border = lv_obj_create(screen_);
    lv_obj_set_size(content_border, LV_HOR_RES, 330);  // 400-70=330
    lv_obj_align(content_border, LV_ALIGN_TOP_LEFT, 0, 70);
    lv_obj_set_style_border_width(content_border, 2, 0);
    lv_obj_set_style_border_color(content_border, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(content_border, LV_OPA_0, 0);  // Transparent background

    // Footer border (400-480)
    lv_obj_t* footer_border = lv_obj_create(screen_);
    lv_obj_set_size(footer_border, LV_HOR_RES, 80);  // 480-400=80
    lv_obj_align(footer_border, LV_ALIGN_TOP_LEFT, 0, 400);
    lv_obj_set_style_border_width(footer_border, 2, 0);
    lv_obj_set_style_border_color(footer_border, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(footer_border, LV_OPA_0, 0);  // Transparent background
    
    // Add debug grid
    CreateDebugGrid(screen_);
}

void BaseScreen::CreateDebugGrid(lv_obj_t* parent) {
    lv_color_t grid_color = lv_color_make(255, 0, 0);  // Red
    lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_color(&style_line, grid_color);
    lv_style_set_line_width(&style_line, 1);
    lv_style_set_line_rounded(&style_line, false);

    // Vertical lines at x=2 and x=798
    for (int x : {2, 798}) {
        // Main line
        static lv_point_t line_points[2];
        line_points[0] = {static_cast<lv_coord_t>(x), 0};
        line_points[1] = {static_cast<lv_coord_t>(x), 480};
        
        lv_obj_t* line = lv_line_create(parent);
        lv_line_set_points(line, line_points, 2);
        lv_obj_add_style(line, &style_line, 0);
        
        // Add ticks and labels every 50px
        for (int y = 0; y <= 480; y += 50) {
            // Tick mark (small horizontal line)
            static lv_point_t tick_points[2];
            tick_points[0] = {static_cast<lv_coord_t>(x - 5), static_cast<lv_coord_t>(y)};
            tick_points[1] = {static_cast<lv_coord_t>(x + 5), static_cast<lv_coord_t>(y)};
            
            lv_obj_t* tick = lv_line_create(parent);
            lv_line_set_points(tick, tick_points, 2);
            lv_obj_add_style(tick, &style_line, 0);
            
            // Label
            if (x == 2) {  // Only label on left side to avoid clutter
                lv_obj_t* label = lv_label_create(parent);
                lv_label_set_text_fmt(label, "%d", y);
                lv_obj_set_style_text_color(label, grid_color, 0);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
                lv_obj_align(label, LV_ALIGN_TOP_LEFT, x + 10, y - 6);
            }
        }
    }
    
    // Horizontal lines at y=2 and y=478 (near bottom)
    for (int y : {2, 478}) {
        // Main line
        static lv_point_t line_points[2];
        line_points[0] = {0, static_cast<lv_coord_t>(y)};
        line_points[1] = {800, static_cast<lv_coord_t>(y)};
        
        lv_obj_t* line = lv_line_create(parent);
        lv_line_set_points(line, line_points, 2);
        lv_obj_add_style(line, &style_line, 0);
        
        // Add ticks and labels every 50px
        for (int x = 0; x <= 800; x += 50) {
            // Tick mark (small vertical line)
            static lv_point_t tick_points[2];
            tick_points[0] = {static_cast<lv_coord_t>(x), static_cast<lv_coord_t>(y - 5)};
            tick_points[1] = {static_cast<lv_coord_t>(x), static_cast<lv_coord_t>(y + 5)};
            
            lv_obj_t* tick = lv_line_create(parent);
            lv_line_set_points(tick, tick_points, 2);
            lv_obj_add_style(tick, &style_line, 0);
            
            // Label
            if (y == 2) {  // Only label on top to avoid clutter
                lv_obj_t* label = lv_label_create(parent);
                lv_label_set_text_fmt(label, "%d", x);
                lv_obj_set_style_text_color(label, grid_color, 0);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
                lv_obj_align(label, LV_ALIGN_TOP_LEFT, x - 10, y + 12);
            }
        }
    }
}

void BaseScreen::CreateNavigationButtons(const char* left_text, const char* right_text,
                                      lv_color_t left_color, lv_color_t right_color,
                                      lv_color_t left_pressed, lv_color_t right_pressed) {
    // Calculate button positions
    const int FOOTER_TOP = 400;  // Top of footer area (from our border)
    const int FOOTER_HEIGHT = 80; // Footer height
    
    // Calculate vertical position to center buttons in footer
    int btn_y = FOOTER_TOP + (FOOTER_HEIGHT - BUTTON_HEIGHT) / 2;
    
    // Horizontal positions (keeping existing spacing from edges)
    int btn_left_x = BUTTON_SPACING;
    int btn_right_x = LV_HOR_RES - BUTTON_WIDTH - BUTTON_SPACING;
    
    // Create left button (usually Return)
    left_button_ = createStandardButton(screen_, left_text,
                                      btn_left_x, btn_y,
                                      BUTTON_WIDTH, BUTTON_HEIGHT,
                                      left_color, left_pressed);
                                      
    // Create right button (usually action)
    right_button_ = createStandardButton(screen_, right_text,
                                       btn_right_x, btn_y,
                                       BUTTON_WIDTH, BUTTON_HEIGHT,
                                       right_color, right_pressed);
                                       
    // Set up button callbacks
    lv_obj_add_event_cb(left_button_, LeftButtonCallback, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(right_button_, RightButtonCallback, LV_EVENT_CLICKED, this);
}

void BaseScreen::Show() {
    DPRINTF("BaseScreen::Show() - Entering, screen_=%p\n", screen_);
    
    if (screen_) {
        lv_obj_t* current_screen = lv_scr_act();
        DPRINTF("Current screen before load: %p, loading screen: %p\n", current_screen, screen_);
        
        // Load the screen
        lv_scr_load_anim(screen_, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        
        // Verify the screen was loaded
        current_screen = lv_scr_act();
        DPRINTF("Current screen after load: %p, expected: %p\n", current_screen, screen_);
        
        if (current_screen != screen_) {
            DPRINTLN("WARNING: Screen load may have failed, trying direct load");
            lv_scr_load(screen_);
        }
        
        // Force a full refresh
        lv_refr_now(nullptr);
    } else {
        DPRINTLN("ERROR: Cannot show screen - screen_ is null");
    }
    
    DPRINTLN("BaseScreen::Show() - Complete");
}

void BaseScreen::Hide() {
    // Base implementation just removes the screen
    // Derived classes might need to do cleanup before hiding
}

void BaseScreen::SetTitle(const char* title) {
    if (title_label_) {
        lv_label_set_text(title_label_, title);
    }
}

void BaseScreen::LeftButtonCallback(lv_event_t* e) {
    BaseScreen* screen = static_cast<BaseScreen*>(lv_event_get_user_data(e));
    if (screen) {
        screen->OnLeftButtonClick();
    }
}

void BaseScreen::RightButtonCallback(lv_event_t* e) {
    BaseScreen* screen = static_cast<BaseScreen*>(lv_event_get_user_data(e));
    if (screen) {
        screen->OnRightButtonClick();
    }
}
