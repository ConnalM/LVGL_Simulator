#pragma once

#include <Arduino.h>
#include <vector>
#include <functional>
#include "common/TimeManager.h"
#include "common/Types.h"

/**
 * @brief Race state enumeration
 */
enum class RaceState {
    Idle,       // No race in progress
    Countdown,  // Countdown in progress
    Starting,   // About to start the race
    Active,     // Race is active
    Paused,     // Race is paused
    Finished    // Race is finished
};

/**
 * @brief Data structure for tracking each lane's race progress
 */
struct RaceLaneData {
    int laneId;                 // Lane identifier (1-8)
    String racerName;           // Name of the racer (optional)
    int currentLap;             // Current lap count
    int totalLaps;              // Total laps to complete
    bool finished;              // Whether the lane has finished the race
    bool enabled;               // Whether the lane is active in the race
    uint32_t bestLapTime;       // Best lap time in milliseconds
    uint32_t lastLapTime;       // Last lap time in milliseconds
    uint32_t totalTime;         // Total race time in milliseconds
    uint32_t lastLapTimestamp;  // Timestamp of the last lap
    int position;               // Current race position
};

// Callback function types for observer pattern
using RaceStateChangedCallback = std::function<void(RaceState)>;
using SecondTickCallback = std::function<void(uint32_t)>;
using LapRegisteredCallback = std::function<void(int, uint32_t)>;

/**
 * @brief Race module for managing race state and lap tracking
 * 
 * The RaceModule handles all race-related functionality including:
 * - Race start/stop/pause/resume
 * - Lap counting and timing
 * - Race completion detection
 * - Racer position tracking
 */
class RaceModule {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return RaceModule& The singleton instance
     */
    static RaceModule& getInstance();
    
    /**
     * @brief Initialize the module
     * 
     * This should be called once during system startup.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Update the module state
     * 
     * This should be called regularly in the main loop.
     * Handles race timing and state updates.
     */
    void update();
    
    /**
     * @brief Prepare a new race
     * 
     * This initializes the race with the given parameters but doesn't start it yet.
     * 
     * @param mode Race mode (LAPS, TIMER, DRAG, RALLY)
     * @param numLanes Number of lanes in the race
     * @param numLaps Number of laps in the race (for LAPS mode)
     * @param raceTimeSeconds Race time in seconds (for TIMER mode)
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo prepareRace(RaceMode mode, int numLanes, int numLaps, int raceTimeSeconds = 0);
    
    /**
     * @brief Start the countdown for a race
     * 
     * This transitions the race state to Countdown.
     * 
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo startCountdown();
    
    /**
     * @brief Start the race timing
     * 
     * This is called when the countdown is complete and the race should actually start.
     * 
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo startRace();
    
    /**
     * @brief Pause the current race
     * 
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo pauseRace();
    
    /**
     * @brief Resume a paused race
     * 
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo resumeRace();
    
    /**
     * @brief Stop the current race
     * 
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo stopRace();
    
    /**
     * @brief Reset the current race
     * 
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo resetRace();
    
    /**
     * @brief Register a lap for a lane
     * 
     * @param lane Lane number (1-8)
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo registerLap(int lane);
    
    /**
     * @brief Get the current race state
     * 
     * @return RaceState The current race state
     */
    RaceState getRaceState() const;
    
    /**
     * @brief Get the current race time in milliseconds
     * 
     * @return uint32_t Race time in milliseconds
     */
    uint32_t getRaceTimeMs() const;
    
    /**
     * @brief Check if the race is active
     * 
     * @return bool true if race is active, false otherwise
     */
    bool isRaceActive() const;
    
    /**
     * @brief Check if the race is paused
     * 
     * @return bool true if race is paused, false otherwise
     */
    bool isRacePaused() const;
    
    /**
     * @brief Set callback for race state changes
     * 
     * @param callback Function to call when race state changes
     */
    void setOnRaceStateChangedCallback(RaceStateChangedCallback callback);
    
    /**
     * @brief Set callback for second tick
     * 
     * @param callback Function to call every second during active race
     */
    void setOnSecondTickCallback(SecondTickCallback callback);
    
    /**
     * @brief Set callback for lap registration
     * 
     * @param callback Function to call when a lap is registered
     */
    void setOnLapRegisteredCallback(LapRegisteredCallback callback);
    
    /**
     * @brief Get the current race mode
     * 
     * @return RaceMode The current race mode
     */
    RaceMode getRaceMode() const { return _raceMode; }
    
    /**
     * @brief Get the number of lanes in the race
     * 
     * @return int Number of lanes
     */
    int getNumLanes() const { return _numLanes; }
    
    /**
     * @brief Get the number of laps in the race
     * 
     * @return int Number of laps
     */
    int getNumLaps() const { return _numLaps; }
    
    /**
     * @brief Get the lane data for a specific lane
     * 
     * @param laneId Lane identifier (1-based)
     * @return const RaceLaneData& Lane data
     */
    const RaceLaneData& getLaneData(int laneId) const;
    
    /**
     * @brief Get all lane data
     * 
     * @return const std::vector<RaceLaneData>& All lane data
     */
    const std::vector<RaceLaneData>& getAllLaneData() const;
    
    /**
     * @brief Compare two lanes for position sorting
     * 
     * @param a First lane
     * @param b Second lane
     * @return bool true if lane a is ahead of lane b
     */
    static bool compareLanes(const RaceLaneData& a, const RaceLaneData& b);
    
    /**
     * @brief Enable a lane for the race
     * 
     * @param laneId Lane identifier (1-based)
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo enableLane(int laneId);
    
    /**
     * @brief Disable a lane for the race
     * 
     * @param laneId Lane identifier (1-based)
     * @return ErrorInfo Error information (success or failure)
     */
    ErrorInfo disableLane(int laneId);
    
    /**
     * @brief Check if a lane is enabled
     * 
     * @param laneId Lane identifier (1-based)
     * @return bool true if the lane is enabled, false otherwise
     */
    bool isLaneEnabled(int laneId) const;
    
    /**
     * @brief Check if a lane has finished the race
     * 
     * @param laneId Lane identifier (1-based)
     * @return bool true if the lane has finished, false otherwise
     */
    bool isLaneFinished(int laneId) const;
    
    /**
     * @brief Check if the race is finished
     * 
     * @return bool true if all enabled lanes have finished, false otherwise
     */
    bool isRaceFinished() const;
    
    /**
     * @brief Get the race elapsed time
     * 
     * @return uint32_t Race elapsed time in milliseconds
     */
    uint32_t getRaceElapsedTime() const;
    
private:
    // Private constructor for singleton pattern
    RaceModule();
    
    // Prevent copying and assignment
    RaceModule(const RaceModule&) = delete;
    RaceModule& operator=(const RaceModule&) = delete;
    
    // Static instance pointer
    static RaceModule* _instance;
    
    // Private methods
    void updateRacePositions();
    bool isValidLaneId(int laneId) const;
    RaceLaneData& getLaneDataRef(int laneId);
    
    /**
     * @brief Set the race state and notify observers
     * 
     * @param newState The new race state
     */
    void setRaceState(RaceState newState);
    
    /**
     * @brief Update race positions based on current progress
     */
    void updatePositions();
    
    // Member variables
    bool _initialized;
    bool _raceActive;
    bool _racePaused;
    RaceMode _raceMode;
    uint32_t _raceStartTime;
    uint32_t _racePauseTime;
    uint32_t _raceTotalPausedTime;
    int _numLanes;
    int _numLaps;
    int _raceTimeSeconds;
    uint32_t _lastUpdateTime;
    uint32_t _updateIntervalMs;
    uint32_t _countdownTimeMs;
    uint32_t _countdownStartTime;
    RaceState _raceState;
    RaceStateChangedCallback _onRaceStateChangedCallback;
    SecondTickCallback _onSecondTickCallback;
    LapRegisteredCallback _onLapRegisteredCallback;
    
    // Lane data
    std::vector<RaceLaneData> _lanes;
};

// Global instance declaration
extern RaceModule& raceModule;
