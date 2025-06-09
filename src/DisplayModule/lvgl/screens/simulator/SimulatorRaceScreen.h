#pragma once

#include <lvgl.h>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>

// Include the display resolution definitions
#include "DisplayModule/drivers/SimulatorDisplayDriver/SDLBackend.h"

// Simple enum to replace the hardware-dependent RaceMode
enum class SimRaceMode {
    LAPS,
    TIMER,
    DRAG,
    RALLY
};

// Simple struct to replace the hardware-dependent RaceLaneData
struct SimRaceLaneData {
    int laneNumber;
    int position;
    int lapCount;
    uint32_t lastLapTime;
    uint32_t bestLapTime;
    uint32_t totalTime;
    bool finished;
    
    SimRaceLaneData(int lane) : 
        laneNumber(lane), 
        position(lane), 
        lapCount(0), 
        lastLapTime(0), 
        bestLapTime(0), 
        totalTime(0), 
        finished(false) {}
};

/**
 * @brief Simplified Race Screen for the simulator
 * 
 * This class provides a basic race screen UI without hardware dependencies
 */
class SimulatorRaceScreen {
public:
    SimulatorRaceScreen();
    ~SimulatorRaceScreen();
    
    /**
     * @brief Show the race screen
     */
    void show();
    
    /**
     * @brief Hide the race screen
     */
    void hide();
    
    /**
     * @brief Update the race data with simulated values
     */
    void update();
    
    /**
     * @brief Set the active race mode
     * @param mode The race mode to activate
     */
    void setRaceMode(SimRaceMode mode);
    
private:
    // UI Elements
    lv_obj_t* screen_;
    lv_obj_t* title_label_;
    lv_obj_t* stop_button_;
    lv_obj_t* pause_button_;
    lv_obj_t* mode_label_;
    lv_obj_t* content_container_;
    lv_obj_t* race_data_table_;
    
    // Race data
    SimRaceMode currentMode_;
    std::vector<SimRaceLaneData> laneData_;
    uint32_t raceTimer_;
    bool paused_;
    
    // UI creation methods
    void createCommonUI();
    void createRaceUI();
    void createTableHeaders();
    void createLaneRows();
    
    // Button event callbacks
    static void stopButtonCallback(lv_event_t* e);
    static void pauseButtonCallback(lv_event_t* e);
    
    // Helper methods
    void updateRaceData();
    static void formatTime(char* buffer, size_t bufferSize, uint32_t timeMs);
};
