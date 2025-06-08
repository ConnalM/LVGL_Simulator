#include "RaceScreen.h"
#include <Arduino.h>
#include "../../../common/DebugUtils.h"
#include "../../../common/TimeManager.h"
#include "../../../common/Types.h"
#include <cinttypes> // For PRIu32
#include <memory>
#include "../../../InputModule/InputCommand.h"
#include "../../../InputModule/GT911_TouchInput.h"
#include "../utils/UIUtils.h"
#include <cinttypes> // For PRIu32

// ===== RaceModeUI Implementations =====

void LapsRaceUI::CreateUI(lv_obj_t* parent) {
    // Create container with no background or border
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);  // Remove all styles first
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));  // Full screen size
    lv_obj_set_layout(container_, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(container_, 5, 0);
    lv_obj_set_style_pad_row(container_, 2, 0);
    lv_obj_set_style_pad_column(container_, 0, 0);
    lv_obj_set_scrollbar_mode(container_, LV_SCROLLBAR_MODE_AUTO);
    
    // Create the race data table container with no background or border
    race_data_table_ = lv_obj_create(container_);
    lv_obj_remove_style_all(race_data_table_);  // Remove all styles first
    lv_obj_set_size(race_data_table_, lv_pct(100), lv_pct(95));
    lv_obj_set_style_pad_all(race_data_table_, 0, 0);
    lv_obj_set_style_border_width(race_data_table_, 0, 0);
    lv_obj_set_style_bg_opa(race_data_table_, LV_OPA_0, 0);
    lv_obj_set_style_radius(race_data_table_, 0, 0);  // No rounded corners
    lv_obj_set_flex_flow(race_data_table_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(race_data_table_, 1);  // Allow table to grow to fill space
    
    // Create table headers and initial rows
    CreateTableHeaders();
    CreateLaneRows(numLanes_); // Create rows for configured number of lanes
    // Update row heights based on number of lanes
    UpdateRowHeights(numLanes_);
}

void LapsRaceUI::Update() {
    // This method is called periodically to update the UI
    // We don't need to do anything here since the race data is updated via UpdateRaceData
}

void LapsRaceUI::Cleanup() {
    // Reset the row containers array
    for (auto& row : row_containers_) {
        row = nullptr;
    }
    
    // Delete the container which will delete all child objects
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
        race_data_table_ = nullptr; // Will be deleted as part of container_
    }
}

void LapsRaceUI::CreateTableHeaders() {
    // Create a header row container
    lv_obj_t* header_row = lv_obj_create(race_data_table_);
    lv_obj_remove_style_all(header_row);  // Remove default styles
    lv_obj_set_size(header_row, lv_pct(100), LV_SIZE_CONTENT);  // Height will be set by UpdateRowHeights
    lv_obj_set_style_bg_color(header_row, lv_color_hex(0x2C3E50), 0); // Dark blue header
    lv_obj_set_style_bg_opa(header_row, LV_OPA_100, 0);
    lv_obj_set_style_border_width(header_row, 0, 0);
    lv_obj_set_style_pad_all(header_row, 2, 0);
    lv_obj_set_style_pad_row(header_row, 0, 0);
    lv_obj_set_style_pad_column(header_row, 2, 0);
    lv_obj_set_flex_flow(header_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Column definitions with flexible widths
    struct ColumnDef {
        const char* header;
        lv_coord_t flex_grow;
        lv_coord_t min_width;
    };
    
    const ColumnDef columns[] = {
        {"Pos", 1, 40},    // Position
        {"Lane", 1, 50},   // Lane number
        {"Laps", 1, 60},   // Laps count
        {"Last Lap", 2, 80}, // Last lap time
        {"Best Lap", 2, 80}, // Best lap time
        {"Time", 1, 80}     // Total time
    };
    
    for (int i = 0; i < NUM_COLS; i++) {
        const auto& col_def = columns[i];
        
        // Create column container
        lv_obj_t* col = lv_obj_create(header_row);
        lv_obj_remove_style_all(col);
        lv_obj_set_style_pad_all(col, 2, 0);
        lv_obj_set_style_bg_opa(col, LV_OPA_0, 0);
        lv_obj_set_style_border_width(col, 0, 0);
        lv_obj_set_flex_grow(col, col_def.flex_grow);
        lv_obj_set_width(col, LV_SIZE_CONTENT);
        lv_obj_set_height(col, lv_pct(100));
        lv_obj_set_style_min_width(col, col_def.min_width, 0);
        
        // Center content in column
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Create and style label
        lv_obj_t* label = lv_label_create(col);
        lv_label_set_text(label, col_def.header);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    }
}

#include <algorithm>  // For std::max

void LapsRaceUI::UpdateRowHeights(int numLanes) {
    if (numLanes <= 0) return;
    
    // Base height for 1-3 lanes
    int baseHeaderHeight = 50;
    int baseRowHeight = 100;
    
    if (numLanes > 3) {
        // Scale down for 4+ lanes
        float scale = 3.0f / numLanes;
        baseHeaderHeight = static_cast<int>(baseHeaderHeight * scale);
        baseRowHeight = static_cast<int>(baseRowHeight * scale);
    }
    
    // Ensure minimum sizes
    const int minHeaderHeight = 30;
    const int minRowHeight = 30;
    baseHeaderHeight = (baseHeaderHeight > minHeaderHeight) ? baseHeaderHeight : minHeaderHeight;
    baseRowHeight = (baseRowHeight > minRowHeight) ? baseRowHeight : minRowHeight;
    
    // Update header height if race_data_table_ exists
    if (race_data_table_ && lv_obj_get_child_cnt(race_data_table_) > 0) {
        lv_obj_set_height(lv_obj_get_child(race_data_table_, 0), baseHeaderHeight);
    }
    
    // Update row heights
    for (size_t i = 0; i < row_containers_.size(); i++) {
        if (row_containers_[i]) {
            lv_obj_set_height(row_containers_[i], baseRowHeight);
        }
    }
}

void LapsRaceUI::CreateLaneRows(int numLanes) {
    DPRINTF("LapsRaceUI::CreateLaneRows - Creating %d lane rows\n", numLanes);
    
    // Clear existing row containers
    for (size_t i = 0; i < row_containers_.size(); i++) {
        if (row_containers_[i]) {
            lv_obj_del(row_containers_[i]);
            row_containers_[i] = nullptr;
        }
    }
    
    // Limit to configured number of lanes
    if (numLanes > numLanes_) numLanes = numLanes_;
    
    // Column definitions matching the header
    struct ColumnDef {
        lv_coord_t flex_grow;
        lv_coord_t min_width;
    };
    
    const ColumnDef columns[] = {
        {1, 40},   // Position
        {1, 50},   // Lane number
        {1, 60},   // Laps count
        {2, 80},   // Last lap time
        {2, 80},   // Best lap time
        {1, 80}    // Total time
    };
    
    // Create rows for each lane
    for (int i = 0; i < numLanes; i++) {
        // Create a container for the row
        lv_obj_t* row = lv_obj_create(race_data_table_);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);  // Height will be set by UpdateRowHeights
        lv_obj_set_style_min_height(row, 30, 0);  // Absolute minimum row height
        lv_obj_set_style_bg_color(row, lv_color_hex(i % 2 ? 0x2C3E50 : 0x34495E), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_30, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 2, 0);
        lv_obj_set_style_pad_row(row, 0, 0);
        lv_obj_set_style_pad_column(row, 2, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        for (int j = 0; j < NUM_COLS; j++) {
            const auto& col_def = columns[j];
            
            // Create cell container
            lv_obj_t* cell = lv_obj_create(row);
            lv_obj_remove_style_all(cell);
            lv_obj_set_style_pad_all(cell, 2, 0);
            lv_obj_set_style_bg_opa(cell, LV_OPA_0, 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_set_flex_grow(cell, col_def.flex_grow);
            lv_obj_set_width(cell, LV_SIZE_CONTENT);
            lv_obj_set_height(cell, lv_pct(100));
            lv_obj_set_style_min_width(cell, col_def.min_width, 0);
            
            // Center content in cell
            lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            
            // Create and style label
            lv_obj_t* label = lv_label_create(cell);
            lv_label_set_text(label, "-");
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        }
        
        // Store the row container for later updates
        row_containers_[i] = row;
        DPRINTF("Created row container %d at %p\n", i, row);
    }
}

void LapsRaceUI::UpdateRaceData(const std::vector<RaceLaneData>& laneData) {
    DPRINTLN("LapsRaceUI::UpdateRaceData - Updating race data display");
    DPRINTF("Number of lanes to update: %d\n", laneData.size());
    
    // Debug: Print lane data
    for (size_t i = 0; i < laneData.size(); i++) {
        const auto& lane = laneData[i];
        DPRINTF("  Lane %d: enabled=%d, pos=%d, lap=%d/%d, last=%.3fs, best=%.3fs, total=%.3fs\n",
               lane.laneId, lane.enabled, lane.position, lane.currentLap, lane.totalLaps,
               lane.lastLapTime/1000.0f, lane.bestLapTime/1000.0f, lane.totalTime/1000.0f);
    }
    
    // Update each lane's data
    for (size_t i = 0; i < laneData.size() && i < row_containers_.size(); i++) {
        const auto& lane = laneData[i];
        lv_obj_t* row = row_containers_[i];
        
        if (!row) {
            DPRINTF("  Warning: Row %d is null\n", i);
            continue;
        }
        
        // Buffer for formatted values
        char position[16] = "-";
        char laneStr[16] = "-";
        char lapsStr[16] = "-";
        char lastLapStr[16] = "-";
        char bestLapStr[16] = "-";
        char totalTimeStr[16] = "-";
        
        // Format values
        snprintf(position, sizeof(position), "%d", lane.position);
        snprintf(laneStr, sizeof(laneStr), "%d", lane.laneId);
        snprintf(lapsStr, sizeof(lapsStr), "%d/%d", lane.currentLap, lane.totalLaps);
        
        if (lane.lastLapTime > 0) {
            FormatTime(lastLapStr, sizeof(lastLapStr), lane.lastLapTime);
        }
        
        if (lane.bestLapTime > 0) {
            FormatTime(bestLapStr, sizeof(bestLapStr), lane.bestLapTime);
        }
        
        if (lane.totalTime > 0) {
            FormatTime(totalTimeStr, sizeof(totalTimeStr), lane.totalTime);
        }
        
        const char* values[] = {
            position,
            laneStr,
            lapsStr,
            lastLapStr,
            bestLapStr,
            totalTimeStr
        };
        
        DPRINTF("Updating row for lane %d with values: %s | %s | %s | %s | %s | %s\n",
               lane.laneId, values[0], values[1], values[2], values[3], values[4], values[5]);
        
        // Update each cell in the row
        for (int j = 0; j < NUM_COLS && j < lv_obj_get_child_cnt(row); j++) {
            lv_obj_t* cell = lv_obj_get_child(row, j);
            if (!cell) {
                DPRINTF("Cell %d is null in row for lane %d\n", j, lane.laneId);
                continue;
            }
            
            // Get the label from the cell
            lv_obj_t* label = lv_obj_get_child(cell, 0);
            if (!label) {
                DPRINTF("Label in cell %d is null for lane %d\n", j, lane.laneId);
                continue;
            }
            
            // Update the label text
            lv_label_set_text(label, values[j]);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
            
            // Set text opacity based on lane state
            lv_obj_set_style_text_opa(label, lane.enabled ? LV_OPA_100 : LV_OPA_50, 0);
            
            // Debug output
            DPRINTF("  Updated cell %d: %s\n", j, values[j]);
        }
    }
}

/**
 * @brief Format a time value in milliseconds to a MM:SS.mmm string
 * @param buffer Output buffer for the formatted string
 * @param bufferSize Size of the output buffer
 * @param timeMs Time in milliseconds
 */
void LapsRaceUI::FormatTime(char* buffer, size_t bufferSize, uint32_t timeMs) {
    if (buffer == nullptr || bufferSize == 0) {
        return;
    }
    
    // Handle zero time specially
    if (timeMs == 0) {
        strncpy(buffer, "00:00.000", bufferSize);
        buffer[bufferSize - 1] = '\0'; // Ensure null termination
        return;
    }
    
    uint32_t minutes = (timeMs / 60000) % 60;
    uint32_t seconds = (timeMs / 1000) % 60;
    uint32_t milliseconds = timeMs % 1000;
    
    // Ensure we have enough space for the formatted string ("00:00.000" + null terminator)
    if (bufferSize < 10) {
        strncpy(buffer, "--:--.---", bufferSize);
        buffer[bufferSize - 1] = '\0'; // Ensure null termination
        return;
    }
    
    snprintf(buffer, bufferSize, "%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32, 
             minutes, seconds, milliseconds);
    
    // Ensure null termination in case of truncation
    buffer[bufferSize - 1] = '\0';
}

LapsRaceUI::LapsRaceUI(uint8_t numLanes) : numLanes_(numLanes) {
    // Initialize all row containers to nullptr
    for (auto& row : row_containers_) {
        row = nullptr;
    }
}

LapsRaceUI::~LapsRaceUI() {
    // Cleanup is handled by the Cleanup() method
}

TimerRaceUI::~TimerRaceUI() {
    // Cleanup is handled by the Cleanup() method
}

void TimerRaceUI::CreateUI(lv_obj_t* parent) {
    DPRINTF("TimerRaceUI::CreateUI - Creating UI with %d lanes\n", numLanes_);
    // Create container with no background or border
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    
    // Create the race data table first
    race_data_table_ = lv_obj_create(container_);
    lv_obj_remove_style_all(race_data_table_);  // Remove all styles first
    lv_obj_set_size(race_data_table_, lv_pct(98), lv_pct(75)); // Slightly smaller width and height for better margins
    lv_obj_align(race_data_table_, LV_ALIGN_TOP_MID, 0, 120);  // Start 120px from top to make room for title
    lv_obj_set_style_pad_all(race_data_table_, 0, 0);
    lv_obj_set_style_border_width(race_data_table_, 0, 0);
    lv_obj_set_style_bg_opa(race_data_table_, LV_OPA_0, 0);
    lv_obj_set_style_radius(race_data_table_, 0, 0);  // No rounded corners
    lv_obj_set_flex_flow(race_data_table_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(race_data_table_, 2, 0); // Add small gap between rows
    lv_obj_set_style_pad_column(race_data_table_, 0, 0); // No horizontal padding between columns
    
    // Create table headers and initial rows
    DPRINTF("TimerRaceUI::CreateUI - Creating table headers and %d lane rows\n", numLanes_);
    CreateTableHeaders();
    CreateLaneRows(numLanes_); // Create rows for configured number of lanes
}

void TimerRaceUI::Update() {
    // This method is called periodically to update the UI
    // We don't need to do anything here since the race data is updated via UpdateRaceData
}

void TimerRaceUI::SetNumLanes(uint8_t numLanes) { 
    DPRINTF("TimerRaceUI::SetNumLanes - Setting numLanes_ to %d\n", numLanes);
    numLanes_ = numLanes; 
}

void TimerRaceUI::Cleanup() {
    // Reset the row containers array
    for (auto& row : row_containers_) {
        row = nullptr;
    }
    
    // Delete the container which will delete all child objects
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
        race_data_table_ = nullptr; // Will be deleted as part of container_
    }
}

void TimerRaceUI::CreateTableHeaders() {
    // Create header row
    lv_obj_t* header_row = lv_obj_create(race_data_table_);
    lv_obj_remove_style_all(header_row);
    lv_obj_set_size(header_row, lv_pct(100), 70); // Same height as data rows
    lv_obj_set_style_bg_color(header_row, lv_color_hex(0x0a0a0a), 0); // Darker header background
    lv_obj_set_style_bg_opa(header_row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header_row, 0, 0);
    lv_obj_set_style_pad_all(header_row, 0, 0);
    lv_obj_set_style_pad_hor(header_row, 0, 0);
    lv_obj_set_style_pad_ver(header_row, 0, 0);
    lv_obj_set_flex_flow(header_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    // Add header labels with updated widths
    const char* headers[] = {"Pos", "Lane", "Lap", "Last Lap", "Best Lap", "Current"};
    const lv_coord_t widths[] = {10, 10, 15, 25, 25, 15}; // Percentages
    
    for (int i = 0; i < 6; i++) {
        lv_obj_t* col = lv_obj_create(header_row);
        lv_obj_remove_style_all(col);
        lv_obj_set_size(col, lv_pct(widths[i]), lv_pct(100));
        lv_obj_set_style_bg_opa(col, LV_OPA_0, 0);
        lv_obj_set_style_border_width(col, 0, 0);
        lv_obj_set_style_pad_all(col, 0, 0);
        lv_obj_set_style_pad_hor(col, 2, 0);
        lv_obj_set_style_pad_ver(col, 0, 0);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        lv_obj_t* label = lv_label_create(col);
        lv_label_set_text(label, headers[i]);
        lv_obj_set_style_text_color(label, lv_color_hex(0x4fc3f7), 0); // Light blue header text
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_letter_space(label, 1, 0);
        lv_obj_center(label);
    }
}

void TimerRaceUI::CreateLaneRows(int numLanes) {
    DPRINTF("TimerRaceUI::CreateLaneRows - Requested %d lanes, numLanes_ = %d\n", numLanes, numLanes_);
    // Limit to configured number of lanes
    if (numLanes > numLanes_) {
        numLanes = numLanes_;
        DPRINTF("TimerRaceUI::CreateLaneRows - Limited to %d lanes\n", numLanes);
    }
    
    // Create a row for each lane
    for (int i = 0; i < numLanes; i++) {
        // Create row container
        lv_obj_t* row = lv_obj_create(race_data_table_);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 70); // Increased row height to 70px
        lv_obj_set_style_bg_color(row, lv_color_hex(i % 2 ? 0x111111 : 0x1a1a1a), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_hor(row, 0, 0);
        lv_obj_set_style_pad_ver(row, 0, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        // Store the row in our array for easy access later
        row_containers_[i] = row;
        
        // Create cells for each column with updated widths
        const lv_coord_t widths[] = {10, 10, 15, 25, 25, 15}; // Same as headers
        
        // Placeholders for each column: Pos, Lane, Lap, Last Lap, Best Lap, Current
        const char* placeholders[] = {
            "-",                  // Position
            String(i+1).c_str(),  // Lane number
            "0/0",               // Lap count/total
            "--:--:---",         // Last lap time
            "--:--:---",         // Best lap time
            "--:--"              // Current time
        };
        
        for (int j = 0; j < NUM_COLS; j++) {
            lv_obj_t* cell = lv_obj_create(row);
            lv_obj_remove_style_all(cell);
            lv_obj_set_size(cell, lv_pct(widths[j]), lv_pct(100));
            lv_obj_set_style_bg_opa(cell, LV_OPA_0, 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_set_style_pad_all(cell, 0, 0);
            lv_obj_set_style_pad_hor(cell, 2, 0);
            lv_obj_set_style_pad_ver(cell, 0, 0);
            lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            
            lv_obj_t* label = lv_label_create(cell);
            lv_label_set_text(label, placeholders[j]);
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
            lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_center(label);
        }
    }
}

void TimerRaceUI::UpdateRaceData(const std::vector<RaceLaneData>& laneData) {
    DPRINTLN("TimerRaceUI::UpdateRaceData - Updating race data display");
    DPRINTF("Number of lanes to update: %d\n", laneData.size());
    
    // First, hide all rows
    for (int i = 0; i < numLanes_; i++) {
        if (row_containers_[i]) {
            lv_obj_add_flag(row_containers_[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    // Update each lane's data
    for (const auto& lane : laneData) {
        // Skip if lane is not enabled or has invalid lane ID
        if (!lane.enabled || lane.laneId < 1 || lane.laneId > numLanes_) {
            DPRINTF("Skipping lane %d: %s\n", lane.laneId, 
                   !lane.enabled ? "not enabled" : "invalid lane ID");
            continue;
        }
        
        // Find the row container for this lane (lane IDs are 1-based)
        int rowIndex = lane.laneId - 1;
        if (rowIndex < 0 || rowIndex >= static_cast<int>(row_containers_.size()) || !row_containers_[rowIndex]) {
            DPRINTF("Invalid row container for lane %d at index %d\n", lane.laneId, rowIndex);
            continue;
        }
        
        // Make the row visible
        lv_obj_clear_flag(row_containers_[rowIndex], LV_OBJ_FLAG_HIDDEN);
        lv_obj_t* row = row_containers_[rowIndex];
        if (!row) continue;
        
        // Make the row visible
        lv_obj_clear_flag(row, LV_OBJ_FLAG_HIDDEN);
        
        // Format the data for display
        String posText = String(lane.position);
        String laneText = String(lane.laneId);
        
        // Format lap count as x/y
        String lapText = String(lane.currentLap) + "/" + String(lane.totalLaps);
        
        // Format last lap time (MM:SS:mmm)
        char lastLapBuffer[16];
        if (lane.lastLapTime > 0) {
            uint32_t lastLapMs = lane.lastLapTime;
            uint32_t lastLapMin = (lastLapMs / 60000);
            uint32_t lastLapSec = (lastLapMs / 1000) % 60;
            uint32_t lastLapMillis = lastLapMs % 1000;
            snprintf(lastLapBuffer, sizeof(lastLapBuffer), "%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32, 
                     lastLapMin, lastLapSec, lastLapMillis);
        } else {
            strncpy(lastLapBuffer, "--:--.---", sizeof(lastLapBuffer));
        }
        
        // Format best lap time (MM:SS:mmm)
        char bestLapBuffer[16];
        if (lane.bestLapTime > 0) {
            uint32_t bestLapMs = lane.bestLapTime;
            uint32_t bestLapMin = (bestLapMs / 60000);
            uint32_t bestLapSec = (bestLapMs / 1000) % 60;
            uint32_t bestLapMillis = bestLapMs % 1000;
            snprintf(bestLapBuffer, sizeof(bestLapBuffer), "%02" PRIu32 ":%02" PRIu32 ".%03" PRIu32, 
                     bestLapMin, bestLapSec, bestLapMillis);
        } else {
            strncpy(bestLapBuffer, "--:--.---", sizeof(bestLapBuffer));
        }
        
        // Format current time (MM:SS)
        char currentTimeBuffer[16];
        if (lane.totalTime > 0) {
            uint32_t totalMs = lane.totalTime;
            uint32_t totalMin = (totalMs / 60000);
            uint32_t totalSec = (totalMs / 1000) % 60;
            snprintf(currentTimeBuffer, sizeof(currentTimeBuffer), "%02" PRIu32 ":%02" PRIu32, 
                     totalMin, totalSec);
        } else {
            strncpy(currentTimeBuffer, "00:00", sizeof(currentTimeBuffer));
        }
        
        // Update the cell labels with all required fields
        const char* values[] = {
            posText.c_str(),       // Position
            laneText.c_str(),      // Lane number
            lapText.c_str(),       // Lap count/total
            lastLapBuffer,         // Last lap time
            bestLapBuffer,         // Best lap time
            currentTimeBuffer      // Current time
        };
        
        // Get each cell and update its label
        for (int j = 0; j < NUM_COLS; j++) {
            // Each cell is a child of the row, and each label is a child of the cell
            lv_obj_t* cell = lv_obj_get_child(row, j);
            if (!cell) continue;
            
            lv_obj_t* label = lv_obj_get_child(cell, 0);
            if (!label) continue;
            
            lv_label_set_text(label, values[j]);
        }
    }
}

void DragRaceUI::CreateUI(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(70));
    lv_obj_align(container_, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_bg_opa(container_, LV_OPA_0, 0);
    
    // Add DRAG-specific UI elements here
    lv_obj_t* label = lv_label_create(container_);
    lv_label_set_text(label, "DRAG RACE MODE");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}

void DragRaceUI::Update() {
    // Update DRAG-specific UI elements
}

void DragRaceUI::Cleanup() {
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
}

void RallyRaceUI::CreateUI(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(70));
    lv_obj_align(container_, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_bg_opa(container_, LV_OPA_0, 0);
    
    // Add RALLY-specific UI elements here
    lv_obj_t* label = lv_label_create(container_);
    lv_label_set_text(label, "RALLY MODE");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}

void RallyRaceUI::Update() {
    // Update RALLY-specific UI elements
}

void RallyRaceUI::Cleanup() {
    if (container_) {
        lv_obj_del(container_);
        container_ = nullptr;
    }
}

// ===== RaceScreen Implementation =====

RaceScreen::RaceScreen(uint8_t numLanes)
    : screen_(nullptr),
      title_label_(nullptr),
      stop_button_(nullptr),
      pause_button_(nullptr),
      mode_label_(nullptr),
      currentMode_(RaceMode::LAPS),
      numLanes_(numLanes) {
    
    DPRINTF("RaceScreen::RaceScreen - Constructor called with %d lanes\n", numLanes);
    
    // Validate number of lanes
    if (numLanes_ < 1) numLanes_ = 1;
    if (numLanes_ > 8) numLanes_ = 8;
    
    DPRINTF("RaceScreen::RaceScreen - Validated to %d lanes\n", numLanes_);
    
    // Create screen
    screen_ = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_, lv_color_hex(0x000000), 0); // Black background
    
    // Initialize race mode UIs with the configured number of lanes
    DPRINTF("RaceScreen::RaceScreen - Creating mode UIs with %d lanes\n", numLanes_);
    modeUIs_[RaceMode::LAPS].reset(new LapsRaceUI(numLanes_));
    modeUIs_[RaceMode::TIMER].reset(new TimerRaceUI(numLanes_));
    modeUIs_[RaceMode::DRAG].reset(new DragRaceUI(numLanes_));
    modeUIs_[RaceMode::RALLY].reset(new RallyRaceUI(numLanes_));
    
    // Create common UI elements
    CreateCommonUI();
    
    // Set default mode
    DPRINTLN("RaceScreen::RaceScreen - Setting default race mode to LAPS");
    SetRaceMode(RaceMode::LAPS);
}

void RaceScreen::SetNumLanes(uint8_t numLanes) {
    DPRINTF("RaceScreen::SetNumLanes - Setting numLanes_ to %d (current: %d)\n", numLanes, numLanes_);
    
    // Validate input
    if (numLanes < 1) numLanes = 1;
    if (numLanes > 8) numLanes = 8;
    
    // If no change, just return
    if (numLanes == numLanes_) {
        DPRINTLN("RaceScreen::SetNumLanes - No change in number of lanes");
        return;
    }
    
    // Update the number of lanes
    numLanes_ = numLanes;
    
    // Update all mode UIs with the new number of lanes
    for (auto& pair : modeUIs_) {
        if (pair.second) {
            DPRINTF("RaceScreen::SetNumLanes - Updating mode %d to %d lanes\n", 
                  static_cast<int>(pair.first), numLanes_);
            pair.second->SetNumLanes(numLanes_);
        }
    }
    
    // If we have a current mode UI, recreate its UI with the new number of lanes
    auto it = modeUIs_.find(currentMode_);
    if (it != modeUIs_.end() && it->second) {
        DPRINTF("RaceScreen::SetNumLanes - Recreating UI for current mode %d with %d lanes\n", 
              static_cast<int>(currentMode_), numLanes_);
        it->second->Cleanup();
        it->second->CreateUI(screen_);
    }
}

RaceScreen::~RaceScreen() {
    // Clean up all race mode UIs first
    for (auto& pair : modeUIs_) {
        if (pair.second) {
            pair.second->Cleanup();
        }
    }
    modeUIs_.clear();
    
    // Clean up screen
    if (screen_) {
        lv_obj_del(screen_);
        screen_ = nullptr;
    }
}

bool RaceScreen::IsPointInButton(lv_obj_t* btn, lv_coord_t x, lv_coord_t y) {
    if (!btn) return false;
    
    lv_area_t area;
    lv_obj_get_coords(btn, &area);
    
    // Check if point is within button area
    if (x >= area.x1 && x <= area.x2 && y >= area.y1 && y <= area.y2) {
        return true;
    }
    return false;
}

void RaceScreen::CreateCommonUI() {
    // Create title label - will show race mode (e.g., "LAPS MODE")
    title_label_ = lv_label_create(screen_);
    lv_label_set_text(title_label_, "RACE MODE"); // Default text, will be updated in SetRaceMode
    lv_obj_set_style_text_font(title_label_, &lv_font_montserrat_32, 0); // Even larger font for better visibility
    lv_obj_set_style_text_color(title_label_, lv_color_white(), 0);
    lv_obj_set_style_text_align(title_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title_label_, LV_PCT(100)); // Full width for centered text
    lv_obj_align(title_label_, LV_ALIGN_TOP_MID, 0, 20); // Increased top margin to 20px
    lv_obj_set_style_text_opa(title_label_, LV_OPA_COVER, 0); // Ensure text is fully opaque
    lv_obj_set_style_pad_bottom(title_label_, 15, 0); // Add padding below the title
    
    // Create mode label - not needed anymore as we'll show mode in title
    mode_label_ = nullptr; // We'll use the title to show the mode
    
    // Button dimensions and positions - these must remain the same across all race modes
    const int button_width = 150;
    const int button_height = 50;
    const int button_spacing = 20;
    const int bottom_margin = 20;
    
    // Define button colors
    lv_color_t red_color = lv_color_hex(0xCC0000);
    lv_color_t dark_red = lv_color_hex(0x990000);
    lv_color_t yellow_color = lv_color_hex(0xCCCC00);
    lv_color_t dark_yellow = lv_color_hex(0x999900);
    lv_color_t black_text = lv_color_black();
    
    // Calculate button positions
    int stop_x = button_spacing;
    int stop_y = LV_VER_RES - button_height - bottom_margin;
    int pause_x = LV_HOR_RES - button_width - button_spacing;
    int pause_y = LV_VER_RES - button_height - bottom_margin;
    
    // Create stop button (left side)
    stop_button_ = createStandardButton(screen_, "STOP", 
                                      stop_x, stop_y,
                                      button_width, button_height,
                                      red_color, dark_red, lv_color_white());
    lv_obj_add_event_cb(stop_button_, StopButtonCallback, LV_EVENT_CLICKED, this);
    
    // Create pause button (right side)
    pause_button_ = createStandardButton(screen_, "PAUSE",
                                       pause_x, pause_y,
                                       button_width, button_height,
                                       yellow_color, dark_yellow, black_text);
    lv_obj_add_event_cb(pause_button_, PauseButtonCallback, LV_EVENT_CLICKED, this);
}

void RaceScreen::SetRaceMode(RaceMode mode) {
    DPRINTF("RaceScreen::SetRaceMode - Requested mode: %d, current mode: %d, numLanes_: %d\n", 
           static_cast<int>(mode), static_cast<int>(currentMode_), numLanes_);
    
    // If already in the requested mode, just return
    if (mode == currentMode_) {
        DPRINTLN("RaceScreen::SetRaceMode - Already in requested mode, no change needed");
        return;
    }
    
    // Clean up current mode UI if it exists
    auto it = modeUIs_.find(currentMode_);
    if (it != modeUIs_.end() && it->second) {
        DPRINTLN("RaceScreen::SetRaceMode - Cleaning up current mode UI");
        it->second->Cleanup();
    } else {
        DPRINTLN("RaceScreen::SetRaceMode - No current mode UI to clean up");
    }
    
    // Set new mode
    currentMode_ = mode;
    
    // Update title label with mode text
    const char* modeText = "";
    switch (mode) {
        case RaceMode::LAPS: modeText = "LAPS MODE"; break;
        case RaceMode::TIMER: modeText = "TIMER MODE"; break;
        case RaceMode::DRAG: modeText = "DRAG RACE"; break;
        case RaceMode::RALLY: modeText = "RALLY MODE"; break;
        case RaceMode::PRACTISE: modeText = "PRACTISE"; break;
        default: modeText = "UNKNOWN"; break;
    }
    DPRINTF("RaceScreen::SetRaceMode - Setting mode text to: %s\n", modeText);
    if (title_label_) {
        lv_label_set_text(title_label_, modeText);
    }
    
    // Create new mode UI
    it = modeUIs_.find(mode);
    if (it != modeUIs_.end() && it->second) {
        DPRINTF("RaceScreen::SetRaceMode - Creating UI for mode %s with %d lanes\n", modeText, numLanes_);
        // Set the number of lanes before creating the UI
        it->second->SetNumLanes(numLanes_);
        it->second->CreateUI(screen_);
    } else {
        DPRINTF("RaceScreen::SetRaceMode - No UI found for mode %d\n", static_cast<int>(mode));
    }
}

void RaceScreen::Show() {
    if (!screen_) {
        DPRINTLN("RaceScreen::Show - Screen is null, cannot show");
        return;
    }
    
    DPRINTF("RaceScreen::Show - Showing screen (current mode: %d)\n", static_cast<int>(currentMode_));
    
    // Load the screen
    lv_scr_load(screen_);
    DPRINTLN("RaceScreen::Show - Screen loaded");
    
    // Make sure the current mode UI is created
    auto it = modeUIs_.find(currentMode_);
    if (it != modeUIs_.end() && it->second) {
        DPRINTF("RaceScreen::Show - Ensuring UI is created for mode %d\n", static_cast<int>(currentMode_));
        if (!lv_obj_get_parent(it->second->GetContainer())) {
            DPRINTLN("RaceScreen::Show - Recreating UI for current mode");
            it->second->CreateUI(screen_);
        } else {
            DPRINTLN("RaceScreen::Show - UI already exists, not recreating");
        }
    } else {
        DPRINTF("RaceScreen::Show - No UI found for current mode %d\n", static_cast<int>(currentMode_));
    }
}

void RaceScreen::Hide() {
    // Clean up the current mode UI when hiding
    auto it = modeUIs_.find(currentMode_);
    if (it != modeUIs_.end() && it->second) {
        it->second->Cleanup();
    }
}

void RaceScreen::Update() {
    // Update the current mode UI if it exists
    auto it = modeUIs_.find(currentMode_);
    if (it != modeUIs_.end() && it->second) {
        it->second->Update();
    }
    if (!screen_) return;
    
    DPRINTLN("RaceScreen: Update called - refreshing display");
    
    // Ensure the screen is in front
    if (lv_scr_act() != screen_) {
        lv_scr_load(screen_);
    }
    
    // Any additional updates to the race screen can be done here
    // For example, updating race time, status, etc.
}

void RaceScreen::StopRace() {
    // Queue an event to stop the race - SystemController will handle navigation to StopScreen
    // This method is used by all race modes (LAPS, TIMER, DRAG, RALLY, PRACTISE)
    InputEvent event;
    event.command = InputCommand::StopRace;
    event.value = 0;
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = getDefaultTargetForCommand(event.command);
    
    // Queue the event to the system
    GT911_TouchInput::queueSystemInputEvent(event);
    
    DPRINTLN("RaceScreen: Stop race event queued");
    
    // Note: We don't need to queue a ReturnToPrevious event here
    // The SystemController should handle navigation to the StopScreen
    // when it receives the StopRace command
}

void RaceScreen::PauseRace() {
    // Queue an event to pause the race
    // This method is used by all race modes (LAPS, TIMER, DRAG, RALLY, PRACTISE)
    InputEvent event;
    event.command = InputCommand::PauseRace;
    event.value = 0;
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = getDefaultTargetForCommand(event.command);
    
    // Queue the event to the system
    GT911_TouchInput::queueSystemInputEvent(event);
    
    DPRINTLN("RaceScreen: Pause race event queued");
    // The SystemController will handle navigation to the PauseScreen
    // and ensure race data is preserved when returning from pause
}

void RaceScreen::StopButtonCallback(lv_event_t* e) {
    if (e->code != LV_EVENT_CLICKED) return;
    
    // Get the RaceScreen instance
    RaceScreen* self = static_cast<RaceScreen*>(lv_event_get_user_data(e));
    if (!self) {
        DPRINTLN("ERROR: RaceScreen instance is null");
        return;
    }
    
    // Stop the race - this works the same for all race modes
    self->StopRace();
    
    DPRINTLN("===== RaceScreen: STOP BUTTON CLICKED =====");
}

void RaceScreen::PauseButtonCallback(lv_event_t* e) {
    if (e->code != LV_EVENT_CLICKED) return;
    
    // Get the RaceScreen instance
    RaceScreen* self = static_cast<RaceScreen*>(lv_event_get_user_data(e));
    if (!self) {
        DPRINTLN("ERROR: RaceScreen instance is null");
        return;
    }
    
    // Pause the race - this works the same for all race modes
    self->PauseRace();
    
    DPRINTLN("===== RaceScreen: PAUSE BUTTON CLICKED =====");
}

RaceModeUI* RaceScreen::GetActiveRaceModeUI() {
    // Find the active race mode UI based on the current mode
    auto it = modeUIs_.find(currentMode_);
    if (it != modeUIs_.end()) {
        return it->second.get();
    }
    return nullptr;
}
