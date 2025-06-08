#pragma once

#include <lvgl.h>
#include <array>
#include <cstdint>
#include <memory>
#include <map>
#include <vector>
#include "../../../InputModule/GT911_TouchInput.h" // For queueSystemInputEvent
#include "../../../common/TimeManager.h"         // For timestamp
#include "../../../common/Types.h"               // For RaceMode
#include "../../../common/DebugUtils.h"          // For DPRINTLN
#include "../../../RaceModule/RaceModule.h"      // For RaceLaneData

// Forward declarations
class RaceModeUI;

/**
 * @brief Main race screen that manages different race mode UIs
 * 
 * This class provides a container for race mode-specific UIs and handles
 * common functionality like stop/pause controls.
 */
class RaceScreen {
public:
    explicit RaceScreen(uint8_t numLanes = 4);
    ~RaceScreen();
    
    /**
     * @brief Set the number of lanes for the race
     * @param numLanes Number of lanes (1-8)
     */
    void SetNumLanes(uint8_t numLanes);

    /**
     * @brief Show the race screen
     */
    void Show();
    
    /**
     * @brief Hide the race screen
     */
    void Hide();
    
    /**
     * @brief Update the race screen without recreating it
     */
    void Update();
    
    /**
     * @brief Set the active race mode
     * @param mode The race mode to activate
     */
    void SetRaceMode(RaceMode mode);
    
    /**
     * @brief Get the current race mode
     * @return RaceMode The current race mode
     */
    RaceMode GetRaceMode() const { return currentMode_; }
    
    /**
     * @brief Get the active race mode UI instance
     * @return RaceModeUI* Pointer to the active race mode UI, or nullptr if none is active
     */
    RaceModeUI* GetActiveRaceModeUI();

private:
    // UI Elements
    lv_obj_t* screen_;
    lv_obj_t* title_label_;
    lv_obj_t* stop_button_;
    lv_obj_t* pause_button_;
    lv_obj_t* mode_label_;
    
    // Race mode management
    RaceMode currentMode_;
    std::map<RaceMode, std::unique_ptr<RaceModeUI>> modeUIs_;
    uint8_t numLanes_;
    
    // Common UI creation
    void CreateCommonUI();
    
    // Button event callbacks
    static void StopButtonCallback(lv_event_t* e);
    static void PauseButtonCallback(lv_event_t* e);
    
    // Debug method to check if touch is within button bounds
    static bool IsPointInButton(lv_obj_t* btn, lv_coord_t x, lv_coord_t y);
    
    // Race control methods
    void StopRace();
    void PauseRace();
};

/**
 * @brief Base class for race mode-specific UIs
 */
class RaceModeUI {
public:
    virtual ~RaceModeUI() = default;
    
    /**
     * @brief Create the UI elements for this race mode
     * @param parent The parent LVGL object
     */
    virtual void CreateUI(lv_obj_t* parent) = 0;
    
    /**
     * @brief Update the UI elements
     */
    virtual void Update() = 0;
    
    /**
     * @brief Clean up UI resources
     */
    virtual void Cleanup() = 0;
    
    /**
     * @brief Get the race mode this UI is for
     * @return RaceMode The race mode
     */
    virtual RaceMode GetMode() const = 0;
    
    /**
     * @brief Set the number of lanes for the race UI
     * @param numLanes Number of lanes (1-8)
     */
    virtual void SetNumLanes(uint8_t numLanes) = 0;
    
    /**
     * @brief Get the container widget for this UI
     * @return lv_obj_t* The container widget, or nullptr if not created
     */
    virtual lv_obj_t* GetContainer() const = 0;
    
    /**
     * @brief Update the race data display
     * @param laneData Vector of lane data to display
     */
    virtual void UpdateRaceData(const std::vector<RaceLaneData>& laneData) = 0;
};

// Implementations for specific race modes
class LapsRaceUI : public RaceModeUI {
public:
    explicit LapsRaceUI(uint8_t numLanes = 4);
    ~LapsRaceUI() override;
    
    void CreateUI(lv_obj_t* parent) override;
    void Update() override;
    void Cleanup() override;
    void SetNumLanes(uint8_t numLanes) { numLanes_ = numLanes; }
    
private:
    uint8_t numLanes_;
    RaceMode GetMode() const override { return RaceMode::LAPS; }
    
    /**
     * @brief Get the container widget for this UI
     * @return lv_obj_t* The container widget, or nullptr if not created
     */
    lv_obj_t* GetContainer() const override { return container_; }
    
    /**
     * @brief Update the race data display with current lane data
     * 
     * @param laneData Vector of lane data to display
     */
    void UpdateRaceData(const std::vector<RaceLaneData>& laneData) override;
    
private:
    // LAPS mode specific UI elements
    lv_obj_t* container_;
    lv_obj_t* race_data_table_;
    
    // Table column headers
    static constexpr int COL_POSITION = 0;
    static constexpr int COL_LANE = 1;
    static constexpr int COL_LAPS = 2;
    static constexpr int COL_LAST_LAP = 3;
    static constexpr int COL_BEST_LAP = 4;
    static constexpr int COL_TOTAL_TIME = 5;
    static constexpr int NUM_COLS = 6;
    
    // Array to store row objects for easy access
    std::array<lv_obj_t*, 8> row_containers_;
    
    // Helper methods
    void CreateTableHeaders();
    void CreateLaneRows(int numLanes);
    void UpdateRowHeights(int numLanes);
    static void FormatTime(char* buffer, size_t bufferSize, uint32_t timeMs);
};

class TimerRaceUI : public RaceModeUI {
public:
    explicit TimerRaceUI(uint8_t numLanes = 4) : numLanes_(numLanes) {
        // Initialize all row containers to nullptr
        for (auto& row : row_containers_) {
            row = nullptr;
        }
    }
    ~TimerRaceUI() override;
    
    void CreateUI(lv_obj_t* parent) override;
    void Update() override;
    void Cleanup() override;
    void SetNumLanes(uint8_t numLanes);
    RaceMode GetMode() const override { return RaceMode::TIMER; }
    
private:
    uint8_t numLanes_;
    
    /**
     * @brief Get the container widget for this UI
     * @return lv_obj_t* The container widget, or nullptr if not created
     */
    lv_obj_t* GetContainer() const override { return container_; }
    
    /**
     * @brief Update the race data display with current lane data
     * 
     * @param laneData Vector of lane data to display
     */
    void UpdateRaceData(const std::vector<RaceLaneData>& laneData) override;

private:
    // TIMER mode specific UI elements
    lv_obj_t* container_;
    lv_obj_t* race_data_table_;
    
    // Table column headers
    static constexpr int COL_POSITION = 0;
    static constexpr int COL_LANE = 1;
    static constexpr int COL_LAP = 2;
    static constexpr int COL_LAST_LAP = 3;
    static constexpr int COL_BEST_LAP = 4;
    static constexpr int COL_CURRENT_TIME = 5;
    static constexpr int NUM_COLS = 6;
    
    // Array to store row objects for easy access
    std::array<lv_obj_t*, 8> row_containers_;
    
    // Helper methods
    void CreateTableHeaders();
    void CreateLaneRows(int numLanes);
};

class DragRaceUI : public RaceModeUI {
public:
    explicit DragRaceUI(uint8_t numLanes = 4) : numLanes_(numLanes) {}
    ~DragRaceUI() override = default;
    
    void CreateUI(lv_obj_t* parent) override;
    void Update() override;
    void Cleanup() override;
    RaceMode GetMode() const override { return RaceMode::DRAG; }
    void SetNumLanes(uint8_t numLanes) { numLanes_ = numLanes; }
    
private:
    uint8_t numLanes_;
    
    /**
     * @brief Get the container widget for this UI
     * @return lv_obj_t* The container widget, or nullptr if not created
     */
    lv_obj_t* GetContainer() const override { return container_; }
    
    void UpdateRaceData(const std::vector<RaceLaneData>& laneData) override {
        // TODO: Implement DRAG mode specific race data update
        DPRINTLN("DragRaceUI::UpdateRaceData - Updating race data display");
    }

private:
    // DRAG mode specific UI elements
    lv_obj_t* container_;
    // Add DRAG-specific UI elements here
};

class RallyRaceUI : public RaceModeUI {
public:
    explicit RallyRaceUI(uint8_t numLanes = 4) : numLanes_(numLanes) {}
    ~RallyRaceUI() override = default;
    
    void CreateUI(lv_obj_t* parent) override;
    void Update() override;
    void Cleanup() override;
    RaceMode GetMode() const override { return RaceMode::RALLY; }
    void SetNumLanes(uint8_t numLanes) { numLanes_ = numLanes; }
    
private:
    uint8_t numLanes_;
    
    /**
     * @brief Get the container widget for this UI
     * @return lv_obj_t* The container widget, or nullptr if not created
     */
    lv_obj_t* GetContainer() const override { return container_; }
    
    void UpdateRaceData(const std::vector<RaceLaneData>& laneData) override {
        // TODO: Implement RALLY mode specific race data update
        DPRINTLN("RallyRaceUI::UpdateRaceData - Updating race data display");
    }

private:
    // RALLY mode specific UI elements
    lv_obj_t* container_;
    // Add RALLY-specific UI elements here
};
