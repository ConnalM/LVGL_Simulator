#include "RaceModule.h"
#include "DisplayModule/DisplayManager.h"
#include <algorithm>

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        static bool firstCall = true; \
        unsigned long now = millis(); \
        if (firstCall || (now - lastDebugPrint > DEBUG_THROTTLE_MS)) { \
            DisplayManager::getInstance().debug(String("[RaceModule] ") + __FUNCTION__, "RaceModule"); \
            lastDebugPrint = now; \
            firstCall = false; \
        } \
    } while(0)

// Initialize static instance pointer
RaceModule* RaceModule::_instance = nullptr;

// Global instance
RaceModule& raceModule = RaceModule::getInstance();

RaceModule& RaceModule::getInstance() {
    if (_instance == nullptr) {
        _instance = new RaceModule();
    }
    return *_instance;
}

RaceModule::RaceModule() 
    : _initialized(false)
    , _raceActive(false)
    , _racePaused(false)
    , _raceMode(RaceMode::LAPS)
    , _raceStartTime(0)
    , _racePauseTime(0)
    , _raceTotalPausedTime(0)
    , _numLanes(0)
    , _numLaps(0)
    , _raceTimeSeconds(0)
    , _lastUpdateTime(0)
    , _updateIntervalMs(100)
    , _countdownTimeMs(0)
    , _countdownStartTime(0)
    , _raceState(RaceState::Idle)
    , _onRaceStateChangedCallback(nullptr)
    , _onSecondTickCallback(nullptr)
    , _onLapRegisteredCallback(nullptr) {
    DEBUG_PRINT_METHOD();
    // Constructor implementation
}

bool RaceModule::initialize() {
    DEBUG_PRINT_METHOD();
    // Check if already initialized
    if (_initialized) {
        DisplayManager::getInstance().info("Already initialized", "RaceModule");
        return true;
    }
    
    DisplayManager::getInstance().info("Initializing...", "RaceModule");
    
    // Ensure TimeManager is initialized
    if (!TimeManager::GetInstance().Initialize()) {
        DisplayManager::getInstance().error("Failed to initialize TimeManager", "RaceModule");
        return false;
    }
    
    // Initialize with default values
    _raceActive = false;
    _racePaused = false;
    _raceMode = RaceMode::LAPS;
    _raceState = RaceState::Idle;
    _raceStartTime = 0;
    _racePauseTime = 0;
    _raceTotalPausedTime = 0;
    _numLanes = 0;
    _numLaps = 0;
    _raceTimeSeconds = 0;
    _lanes.clear();
    
    _initialized = true;
    DisplayManager::getInstance().info("Initialized successfully", "RaceModule");
    return true;
}

void RaceModule::update() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return;
    }
    
    uint32_t currentTime = TimeManager::GetInstance().GetCurrentTimeMs();
    
    // Only update at specified intervals to reduce processing load
    if (currentTime - _lastUpdateTime < _updateIntervalMs) {
        return;
    }
    
    _lastUpdateTime = currentTime;
    
    // Handle different race states
    switch (_raceState) {
        case RaceState::Countdown:
            // Countdown is handled by LightsModule, we just wait for notification
            break;
            
        case RaceState::Starting:
            // This is a transitional state, handled by startRace()
            break;
            
        case RaceState::Active:
            if (!_racePaused) {
                // Calculate race time
                uint32_t raceTimeMs = currentTime - _raceStartTime - _raceTotalPausedTime;
                
                // Update the display with current race data
                std::vector<RaceLaneData> laneData;
                for (const auto& lane : _lanes) {
                    if (lane.enabled) {
                        laneData.push_back(lane);
                    }
                }
                DisplayManager::getInstance().updateRaceData(laneData);
                
                // Check for second tick for clock updates
                static uint32_t lastSecond = 0;
                uint32_t currentSecond = raceTimeMs / 1000;
                if (currentSecond > lastSecond) {
                    lastSecond = currentSecond;
                    if (_onSecondTickCallback) {
                        _onSecondTickCallback(raceTimeMs);
                    }
                }
                
                // Check for race completion in TIMER mode
                if (_raceMode == RaceMode::TIMER && 
                    raceTimeMs >= (uint32_t)_raceTimeSeconds * 1000) {
                    setRaceState(RaceState::Finished);
                }
            }
            break;
            
        case RaceState::Paused:
            // Nothing to update when paused
            break;
            
        case RaceState::Finished:
            // Check if all lanes have finished in LAPS mode
            if (_raceMode == RaceMode::LAPS && !isRaceFinished()) {
                // Race is not actually finished, revert to Active state
                setRaceState(RaceState::Active);
            }
            break;
            
        case RaceState::Idle:
        default:
            // Nothing to update in idle state
            break;
    }
}

ErrorInfo RaceModule::prepareRace(RaceMode mode, int numLanes, int numLaps, int raceTimeSeconds) {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState != RaceState::Idle) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race already in progress", "RaceModule");
    }
    
    // Validate parameters
    if (numLanes <= 0 || numLanes > 8) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Invalid number of lanes", "RaceModule");
    }
    
    if (mode == RaceMode::LAPS && (numLaps <= 0 || numLaps > 100)) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Invalid number of laps", "RaceModule");
    }
    
    if (mode == RaceMode::TIMER && (raceTimeSeconds <= 0 || raceTimeSeconds > 3600)) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Invalid race time", "RaceModule");
    }
    
    // Set race parameters
    _raceMode = mode;
    _numLanes = numLanes;
    _numLaps = numLaps;
    _raceTimeSeconds = raceTimeSeconds;
    
    // Initialize lane data
    _lanes.clear();
    for (int i = 1; i <= numLanes; i++) {
        RaceLaneData lane;
        lane.laneId = i;
        lane.racerName = "Racer " + String(i);
        lane.currentLap = 0;
        lane.totalLaps = numLaps;
        lane.finished = false;
        lane.enabled = true;
        lane.bestLapTime = 0;
        lane.lastLapTime = 0;
        lane.totalTime = 0;
        lane.lastLapTimestamp = 0;
        lane.position = 0;
        _lanes.push_back(lane);
    }
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::startCountdown() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState != RaceState::Idle) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race already in progress", "RaceModule");
    }
    
    // Transition to countdown state
    setRaceState(RaceState::Countdown);
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::startRace() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState != RaceState::Countdown && _raceState != RaceState::Starting) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race not in countdown or starting state", "RaceModule");
    }
    
    // Set race as active
    _raceActive = true;
    _racePaused = false;
    _raceStartTime = TimeManager::GetInstance().GetCurrentTimeMs();
    _raceTotalPausedTime = 0;
    
    // Transition to active state
    setRaceState(RaceState::Active);
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::pauseRace() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState != RaceState::Active) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race not active", "RaceModule");
    }
    
    if (_racePaused) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race already paused", "RaceModule");
    }
    
    // Pause the race
    _racePaused = true;
    _racePauseTime = TimeManager::GetInstance().GetCurrentTimeMs();
    
    // Transition to paused state
    setRaceState(RaceState::Paused);
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::resumeRace() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState != RaceState::Paused) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race not paused", "RaceModule");
    }
    
    // Resume the race
    _racePaused = false;
    _raceTotalPausedTime += TimeManager::GetInstance().GetCurrentTimeMs() - _racePauseTime;
    
    // Transition back to active state
    setRaceState(RaceState::Active);
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::stopRace() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState == RaceState::Idle) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "No race in progress", "RaceModule");
    }
    
    // Stop the race
    _raceActive = false;
    _racePaused = false;
    
    // Transition to idle state
    setRaceState(RaceState::Idle);
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::resetRace() {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    // Reset race parameters
    _raceActive = false;
    _racePaused = false;
    _raceStartTime = 0;
    _racePauseTime = 0;
    _raceTotalPausedTime = 0;
    
    // Reset lane data
    for (auto& lane : _lanes) {
        lane.currentLap = 0;
        lane.finished = false;
        lane.bestLapTime = 0;
        lane.lastLapTime = 0;
        lane.totalTime = 0;
        lane.lastLapTimestamp = 0;
        lane.position = 0;
    }
    
    // Transition to idle state
    setRaceState(RaceState::Idle);
    
    return ErrorInfo(); // Success
}

ErrorInfo RaceModule::registerLap(int lane) {
    DEBUG_PRINT_METHOD();
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    if (_raceState != RaceState::Active && _raceState != RaceState::Paused) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Race not active or paused", "RaceModule");
    }
    
    if (lane < 1 || lane > _numLanes) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Invalid lane number", "RaceModule");
    }
    
    // Find the lane data
    auto it = std::find_if(_lanes.begin(), _lanes.end(), 
                          [lane](const RaceLaneData& data) { return data.laneId == lane; });
    
    if (it == _lanes.end()) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Lane not found", "RaceModule");
    }
    
    // Check if lane is enabled
    if (!it->enabled) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Lane is disabled", "RaceModule");
    }
    
    // Check if lane has already finished
    if (it->finished) {
        return ErrorInfo(ErrorCode::INVALID_STATE, "Lane has already finished", "RaceModule");
    }
    
    // Get current time
    uint32_t currentTime = TimeManager::GetInstance().GetCurrentTimeMs();
    uint32_t raceTimeMs = currentTime - _raceStartTime - _raceTotalPausedTime;
    
    // Calculate lap time
    uint32_t lapTime;
    if (it->currentLap == 0) {
        lapTime = raceTimeMs;
    } else {
        lapTime = currentTime - it->lastLapTimestamp;
    }
    
    // Update lap data
    it->currentLap++;
    it->lastLapTime = lapTime;
    it->lastLapTimestamp = currentTime;
    it->totalTime = raceTimeMs;
    
    // Update best lap time
    if (it->bestLapTime == 0 || lapTime < it->bestLapTime) {
        it->bestLapTime = lapTime;
    }
    
    // Check if lane has finished the race
    if (_raceMode == RaceMode::LAPS && it->currentLap >= it->totalLaps) {
        it->finished = true;
        
        // Update positions
        updatePositions();
        
        // Check if all lanes have finished
        if (isRaceFinished()) {
            setRaceState(RaceState::Finished);
        }
    }
    
    // Notify observers
    if (_onLapRegisteredCallback) {
        _onLapRegisteredCallback(lane, lapTime);
    }
    
    return ErrorInfo(); // Success
}

RaceState RaceModule::getRaceState() const {
    DEBUG_PRINT_METHOD();
    return _raceState;
}

const RaceLaneData& RaceModule::getLaneData(int laneId) const {
    DEBUG_PRINT_METHOD();
    // Find the lane data with the specified ID
    for (const auto& lane : _lanes) {
        if (lane.laneId == laneId) {
            return lane;
        }
    }
    
    // If not found, return the first lane as a fallback
    // This should never happen if the caller validates the lane ID first
    static RaceLaneData emptyLane = {};
    return emptyLane;
}

const std::vector<RaceLaneData>& RaceModule::getAllLaneData() const {
    DEBUG_PRINT_METHOD();
    return _lanes;
}

RaceLaneData& RaceModule::getLaneDataRef(int laneId) {
    DEBUG_PRINT_METHOD();
    // Find the lane data with the specified ID
    for (auto& lane : _lanes) {
        if (lane.laneId == laneId) {
            return lane;
        }
    }
    
    // If not found, return the first lane as a fallback
    // This should never happen if the caller validates the lane ID first
    static RaceLaneData emptyLane = {};
    return emptyLane;
}

bool RaceModule::isValidLaneId(int laneId) const {
    DEBUG_PRINT_METHOD();
    return laneId >= 1 && laneId <= _numLanes;
}

bool RaceModule::isLaneEnabled(int laneId) const {
    DEBUG_PRINT_METHOD();
    // Find the lane data
    for (const auto& lane : _lanes) {
        if (lane.laneId == laneId) {
            return lane.enabled;
        }
    }
    
    // Lane not found
    return false;
}

bool RaceModule::isLaneFinished(int laneId) const {
    DEBUG_PRINT_METHOD();
    // Find the lane data
    for (const auto& lane : _lanes) {
        if (lane.laneId == laneId) {
            return lane.finished;
        }
    }
    
    // Lane not found
    return false;
}

uint32_t RaceModule::getRaceTimeMs() const {
    DEBUG_PRINT_METHOD();
    if (!_raceActive) {
        return 0;
    }
    
    uint32_t currentTime = TimeManager::GetInstance().GetCurrentTimeMs();
    return currentTime - _raceStartTime - _raceTotalPausedTime;
}

bool RaceModule::isRaceActive() const {
    DEBUG_PRINT_METHOD();
    return _raceState == RaceState::Active || _raceState == RaceState::Paused;
}

bool RaceModule::isRacePaused() const {
    DEBUG_PRINT_METHOD();
    return _raceState == RaceState::Paused;
}

void RaceModule::setOnRaceStateChangedCallback(RaceStateChangedCallback callback) {
    DEBUG_PRINT_METHOD();
    _onRaceStateChangedCallback = callback;
}

void RaceModule::setOnSecondTickCallback(SecondTickCallback callback) {
    DEBUG_PRINT_METHOD();
    _onSecondTickCallback = callback;
}

void RaceModule::setOnLapRegisteredCallback(LapRegisteredCallback callback) {
    DEBUG_PRINT_METHOD();
    _onLapRegisteredCallback = callback;
}

// Private helper method to set race state and notify observers
void RaceModule::setRaceState(RaceState newState) {
    DEBUG_PRINT_METHOD();
    if (_raceState != newState) {
        _raceState = newState;
        
        // Notify observers of state change
        if (_onRaceStateChangedCallback) {
            _onRaceStateChangedCallback(newState);
        }
    }
}

// Helper method to update race positions
void RaceModule::updatePositions() {
    DEBUG_PRINT_METHOD();
    // Create a copy of the lanes for sorting
    std::vector<RaceLaneData> sortedLanes = _lanes;
    
    // Sort the lanes by position
    std::sort(sortedLanes.begin(), sortedLanes.end(), compareLanes);
    
    // Update the position of each lane
    for (size_t i = 0; i < sortedLanes.size(); i++) {
        // Find the lane in the original vector and update its position
        for (auto& lane : _lanes) {
            if (lane.laneId == sortedLanes[i].laneId) {
                lane.position = i + 1;
                break;
            }
        }
    }
}

bool RaceModule::isRaceFinished() const {
    DEBUG_PRINT_METHOD();
    // Check if all enabled lanes have finished
    for (const auto& lane : _lanes) {
        if (lane.enabled && !lane.finished) {
            return false;
        }
    }
    return true;
}

uint32_t RaceModule::getRaceElapsedTime() const {
    DEBUG_PRINT_METHOD();
    return getRaceTimeMs();
}

bool RaceModule::compareLanes(const RaceLaneData& a, const RaceLaneData& b) {
    // No debug print to avoid recursion and performance impact
    // If one lane is finished and the other isn't, the finished lane is ahead
    if (a.finished && !b.finished) return true;
    if (!a.finished && b.finished) return false;
    
    // If both lanes are finished, compare by total time
    if (a.finished && b.finished) {
        return a.totalTime < b.totalTime;
    }
    
    // If neither lane is finished, compare by lap count first
    if (a.currentLap != b.currentLap) {
        return a.currentLap > b.currentLap;
    }
    
    // If lap counts are equal, compare by total time
    return a.totalTime < b.totalTime;
}

ErrorInfo RaceModule::enableLane(int laneId) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    // Validate lane ID
    if (!isValidLaneId(laneId)) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Invalid lane ID", "RaceModule");
    }
    
    // Find the lane data with the specified ID
    for (auto& lane : _lanes) {
        if (lane.laneId == laneId) {
            // Already enabled? No problem, just return success
            if (lane.enabled) {
                return ErrorInfo(); // Success
            }
            
            // Enable the lane
            lane.enabled = true;
            DisplayManager::getInstance().debug("Lane " + String(laneId) + " enabled", "RaceModule");
            return ErrorInfo(); // Success
        }
    }
    
    // Lane not found (shouldn't happen if isValidLaneId passed)
    return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Lane not found", "RaceModule");
}

ErrorInfo RaceModule::disableLane(int laneId) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return ErrorInfo(ErrorCode::NOT_INITIALIZED, "RaceModule not initialized", "RaceModule");
    }
    
    // Validate lane ID
    if (!isValidLaneId(laneId)) {
        return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Invalid lane ID", "RaceModule");
    }
    
    // Find the lane data with the specified ID
    for (auto& lane : _lanes) {
        if (lane.laneId == laneId) {
            // Already disabled? No problem, just return success
            if (!lane.enabled) {
                return ErrorInfo(); // Success
            }
            
            // Disable the lane
            lane.enabled = false;
            DisplayManager::getInstance().debug("Lane " + String(laneId) + " disabled", "RaceModule");
            return ErrorInfo(); // Success
        }
    }
    
    // Lane not found (shouldn't happen if isValidLaneId passed)
    return ErrorInfo(ErrorCode::INVALID_PARAMETER, "Lane not found", "RaceModule");
}
