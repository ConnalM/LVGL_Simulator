#include "SimulatorRaceScreen.h"
#include <cstdio>
#include <cstring>
#include "common/log_message.h"


// Constructor
SimulatorRaceScreen::SimulatorRaceScreen() :
    screen_(nullptr),
    title_label_(nullptr),
    stop_button_(nullptr),
    pause_button_(nullptr),
    mode_label_(nullptr),
    content_container_(nullptr),
    race_data_table_(nullptr),
    currentMode_(SimRaceMode::LAPS),
    raceTimer_(0),
    paused_(false) {
    
    // Initialize lane data for 4 lanes
    for (int i = 0; i < 4; i++) {
        laneData_.emplace_back(i + 1);
    }
}

// Destructor
SimulatorRaceScreen::~SimulatorRaceScreen() {
    hide();
}

// Show the race screen
void SimulatorRaceScreen::show() {
    log_message("Showing Race Screen");
    
    // Create screen if it doesn't exist
    if (!screen_) {
        screen_ = lv_obj_create(nullptr);
        lv_obj_set_size(screen_, DISP_HOR_RES, DISP_VER_RES);
        
        // Set a background color
        lv_obj_set_style_bg_color(screen_, lv_color_hex(0x2C3E50), LV_PART_MAIN | LV_STATE_DEFAULT);
        
        // Create the UI elements
        createCommonUI();
        createRaceUI();
    }
    
    // Set the screen as active
    lv_scr_load(screen_);
}

// Hide the race screen
void SimulatorRaceScreen::hide() {
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
        title_label_ = nullptr;
        stop_button_ = nullptr;
        pause_button_ = nullptr;
        mode_label_ = nullptr;
        content_container_ = nullptr;
        race_data_table_ = nullptr;
    }
}

// Update the race screen
void SimulatorRaceScreen::update() {
    if (!screen_) return;
    
    // Update race timer if not paused
    if (!paused_) {
        raceTimer_ += 100; // Increment by 100ms
        
        // Update lane data with simulated values
        updateRaceData();
    }
    
    // Update mode label
    const char* modeText;
    switch (currentMode_) {
        case SimRaceMode::LAPS: modeText = "LAPS MODE"; break;
        case SimRaceMode::TIMER: modeText = "TIMER MODE"; break;
        case SimRaceMode::DRAG: modeText = "DRAG MODE"; break;
        case SimRaceMode::RALLY: modeText = "RALLY MODE"; break;
        default: modeText = "UNKNOWN MODE"; break;
    }
    lv_label_set_text(mode_label_, modeText);
    
    // Update race data display
    if (race_data_table_) {
        for (size_t i = 0; i < laneData_.size(); i++) {
            const auto& lane = laneData_[i];
            
            // Position
            lv_table_set_cell_value_fmt(race_data_table_, i + 1, 0, "%d", lane.position);
            
            // Lane number
            lv_table_set_cell_value_fmt(race_data_table_, i + 1, 1, "%d", lane.laneNumber);
            
            // Lap count
            lv_table_set_cell_value_fmt(race_data_table_, i + 1, 2, "%d", lane.lapCount);
            
            // Last lap time
            char lastLapBuffer[16];
            formatTime(lastLapBuffer, sizeof(lastLapBuffer), lane.lastLapTime);
            lv_table_set_cell_value(race_data_table_, i + 1, 3, lastLapBuffer);
            
            // Best lap time
            char bestLapBuffer[16];
            formatTime(bestLapBuffer, sizeof(bestLapBuffer), lane.bestLapTime);
            lv_table_set_cell_value(race_data_table_, i + 1, 4, bestLapBuffer);
            
            // Total time
            char totalTimeBuffer[16];
            formatTime(totalTimeBuffer, sizeof(totalTimeBuffer), lane.totalTime);
            lv_table_set_cell_value(race_data_table_, i + 1, 5, totalTimeBuffer);
        }
    }
}

// Set the race mode
void SimulatorRaceScreen::setRaceMode(SimRaceMode mode) {
    currentMode_ = mode;
    
    // Reset race data
    raceTimer_ = 0;
    paused_ = false;
    
    for (auto& lane : laneData_) {
        lane.lapCount = 0;
        lane.lastLapTime = 0;
        lane.bestLapTime = 0;
        lane.totalTime = 0;
        lane.finished = false;
    }
    
    // Update the UI
    update();
}

// Create common UI elements
void SimulatorRaceScreen::createCommonUI() {
    // Create title label
    title_label_ = lv_label_create(screen_);
    lv_label_set_text(title_label_, "Race Active");
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title_label_, LV_ALIGN_TOP_MID, 0, 10);
    
    // Create mode label
    mode_label_ = lv_label_create(screen_);
    lv_label_set_text(mode_label_, "LAPS MODE");
    lv_obj_set_style_text_font(mode_label_, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(mode_label_, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(mode_label_, LV_ALIGN_TOP_MID, 0, 40);
    
    // Create stop button
    stop_button_ = lv_btn_create(screen_);
    lv_obj_set_size(stop_button_, 120, 50);
    lv_obj_set_style_bg_color(stop_button_, lv_color_hex(0xE74C3C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(stop_button_, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_event_cb(stop_button_, stopButtonCallback, LV_EVENT_CLICKED, this);
    
    // Add label to stop button
    lv_obj_t* stop_label = lv_label_create(stop_button_);
    lv_label_set_text(stop_label, "STOP");
    lv_obj_center(stop_label);
    
    // Create pause button
    pause_button_ = lv_btn_create(screen_);
    lv_obj_set_size(pause_button_, 120, 50);
    lv_obj_set_style_bg_color(pause_button_, lv_color_hex(0xF39C12), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(pause_button_, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_event_cb(pause_button_, pauseButtonCallback, LV_EVENT_CLICKED, this);
    
    // Add label to pause button
    lv_obj_t* pause_label = lv_label_create(pause_button_);
    lv_label_set_text(pause_label, "PAUSE");
    lv_obj_center(pause_label);
}

// Create race UI elements
void SimulatorRaceScreen::createRaceUI() {
    // Create content container
    content_container_ = lv_obj_create(screen_);
    lv_obj_set_size(content_container_, DISP_HOR_RES - 40, DISP_VER_RES - 150);
    lv_obj_align(content_container_, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_bg_color(content_container_, lv_color_hex(0x34495E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(content_container_, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    
    // Create race data table
    race_data_table_ = lv_table_create(content_container_);
    lv_obj_set_size(race_data_table_, DISP_HOR_RES - 60, DISP_VER_RES - 200);
    lv_obj_align(race_data_table_, LV_ALIGN_TOP_MID, 0, 0);
    
    // Set column widths
    lv_table_set_col_width(race_data_table_, 0, 60);  // Position
    lv_table_set_col_width(race_data_table_, 1, 60);  // Lane
    lv_table_set_col_width(race_data_table_, 2, 60);  // Lap
    lv_table_set_col_width(race_data_table_, 3, 120); // Last Lap
    lv_table_set_col_width(race_data_table_, 4, 120); // Best Lap
    lv_table_set_col_width(race_data_table_, 5, 120); // Total Time
    
    // Create table headers
    createTableHeaders();
    
    // Create lane rows
    createLaneRows();
}

// Create table headers
void SimulatorRaceScreen::createTableHeaders() {
    lv_table_set_cell_value(race_data_table_, 0, 0, "Pos");
    lv_table_set_cell_value(race_data_table_, 0, 1, "Lane");
    lv_table_set_cell_value(race_data_table_, 0, 2, "Lap");
    lv_table_set_cell_value(race_data_table_, 0, 3, "Last Lap");
    lv_table_set_cell_value(race_data_table_, 0, 4, "Best Lap");
    lv_table_set_cell_value(race_data_table_, 0, 5, "Total");
    
    // Style the header row
    lv_obj_set_style_bg_color(race_data_table_, lv_color_hex(0x2980B9), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(race_data_table_, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_DEFAULT);
}

// Create lane rows
void SimulatorRaceScreen::createLaneRows() {
    for (size_t i = 0; i < laneData_.size(); i++) {
        // Position
        lv_table_set_cell_value_fmt(race_data_table_, i + 1, 0, "%d", laneData_[i].position);
        
        // Lane number
        lv_table_set_cell_value_fmt(race_data_table_, i + 1, 1, "%d", laneData_[i].laneNumber);
        
        // Lap count
        lv_table_set_cell_value_fmt(race_data_table_, i + 1, 2, "%d", laneData_[i].lapCount);
        
        // Last lap time
        lv_table_set_cell_value(race_data_table_, i + 1, 3, "--:--:--");
        
        // Best lap time
        lv_table_set_cell_value(race_data_table_, i + 1, 4, "--:--:--");
        
        // Total time
        lv_table_set_cell_value(race_data_table_, i + 1, 5, "--:--:--");
        
        // Style the row based on position
        lv_obj_set_style_bg_color(race_data_table_, lv_color_hex(0x3498DB), LV_PART_ITEMS | LV_STATE_DEFAULT);
    }
}

// Update race data with simulated values
void SimulatorRaceScreen::updateRaceData() {
    // Simulate race progress
    for (auto& lane : laneData_) {
        // Update total time
        lane.totalTime = raceTimer_;
        
        // Simulate lap completion every 5 seconds
        if (raceTimer_ > 0 && raceTimer_ % 5000 == 0) {
            lane.lapCount++;
            lane.lastLapTime = 5000 + (rand() % 1000) - 500; // 4.5 to 5.5 seconds
            
            // Update best lap time
            if (lane.bestLapTime == 0 || lane.lastLapTime < lane.bestLapTime) {
                lane.bestLapTime = lane.lastLapTime;
            }
        }
    }
    
    // Sort lanes by lap count (descending) and then by total time (ascending)
    std::sort(laneData_.begin(), laneData_.end(), [](const SimRaceLaneData& a, const SimRaceLaneData& b) {
        if (a.lapCount != b.lapCount) {
            return a.lapCount > b.lapCount;
        }
        return a.totalTime < b.totalTime;
    });
    
    // Update positions
    for (size_t i = 0; i < laneData_.size(); i++) {
        laneData_[i].position = i + 1;
    }
}

// Format time as MM:SS:ms
void SimulatorRaceScreen::formatTime(char* buffer, size_t bufferSize, uint32_t timeMs) {
    if (timeMs == 0) {
        snprintf(buffer, bufferSize, "--:--:--");
        return;
    }
    
    uint32_t minutes = (timeMs / 60000) % 60;
    uint32_t seconds = (timeMs / 1000) % 60;
    uint32_t ms = timeMs % 1000;
    
    snprintf(buffer, bufferSize, "%02lu:%02lu:%03lu", minutes, seconds, ms);
}

// Stop button callback
void SimulatorRaceScreen::stopButtonCallback(lv_event_t* e) {
    SimulatorRaceScreen* screen = static_cast<SimulatorRaceScreen*>(e->user_data);
    if (screen) {
        log_message("Stop button clicked");
        // Reset race data
        screen->raceTimer_ = 0;
        screen->paused_ = false;
        
        for (auto& lane : screen->laneData_) {
            lane.lapCount = 0;
            lane.lastLapTime = 0;
            lane.bestLapTime = 0;
            lane.totalTime = 0;
            lane.finished = false;
        }
        
        // Update the UI
        screen->update();
    }
}

// Pause button callback
void SimulatorRaceScreen::pauseButtonCallback(lv_event_t* e) {
    SimulatorRaceScreen* screen = static_cast<SimulatorRaceScreen*>(e->user_data);
    if (screen) {
        screen->paused_ = !screen->paused_;
        log_message("Pause button clicked, paused: %s", screen->paused_ ? "true" : "false");
        
        // Update button label
        lv_obj_t* label = lv_obj_get_child(screen->pause_button_, 0);
        if (label) {
            lv_label_set_text(label, screen->paused_ ? "RESUME" : "PAUSE");
        }
    }
}
