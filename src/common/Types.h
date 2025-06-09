#pragma once

#ifdef NATIVE_BUILD
#include "../ArduinoCompat.h"
#else
#include <Arduino.h>
#endif

// Maximum number of lanes supported
#define MAX_LANES 8

// Maximum number of laps to track per lane
#define MAX_LAPS 999

// Default debounce time in milliseconds
#define DEFAULT_DEBOUNCE_TIME 1000

// Explicit race modes for the system
enum class RaceMode : uint8_t {
    LAPS = 1,
    TIMER = 2,
    DRAG = 3,
    RALLY = 4,
    PRACTISE = 5
};

/**
 * @brief Race status
 */
enum class RaceStatus {
    IDLE,       // Not started
    READY,      // Ready to start
    RUNNING,    // Race in progress
    PAUSED,     // Race paused
    FINISHED    // Race completed
};

/**
 * @brief Sensor types
 */
enum class SensorType {
    START,      // Start line sensor
    FINISH,     // Finish line sensor
    CHECKPOINT, // Intermediate checkpoint
    LAP         // Combined start/finish for lap counting
};

/**
 * @brief User interface input event types
 */
enum class UIInputEventType {
    START_RACE,     // Start the race
    PAUSE_RACE,     // Pause the race
    RESUME_RACE,    // Resume the race
    STOP_RACE,      // Stop the race
    RESET_RACE,     // Reset the race
    CHANGE_MODE,    // Change race mode
    CHANGE_LANES,   // Change number of active lanes
    SET_LAPS,       // Set target lap count
    SET_TIME,       // Set target race time
    SENSOR_QUANTITY, // Set number of sensors per lane (for DRAG, RALLY, or segmented laps)
    NONE            // No event/invalid event
};

/**
 * @brief Sensor event data
 */
struct SensorEvent {
    uint8_t laneId;             // Lane ID (0-7 for 8 lanes)
    SensorType type;            // Type of sensor triggered
    unsigned long timestamp;    // Time of trigger in milliseconds
    bool isValid;               // Whether this is a valid trigger (debounced)
};

/**
 * @brief User interface input event data
 */
struct UIInputEvent {
    UIInputEventType type;        // Type of input event
    int value;                  // Optional parameter value
    const char* source;         // Source of the event ("touch", "serial", "keyboard")
};

/**
 * @brief Lap data for a single lap
 */
struct LapData {
    unsigned long startTime;    // Start time in milliseconds
    unsigned long endTime;      // End time in milliseconds
    unsigned long lapTime;      // Lap time in milliseconds (endTime - startTime)
};

/**
 * @brief Lane data for a single lane
 */
struct LaneData {
    uint8_t laneId;                 // Lane ID (0-7 for 8 lanes)
    bool isActive;                  // Whether this lane is active
    unsigned long startTime;        // Race start time for this lane
    unsigned long lastTriggerTime;  // Last time a sensor was triggered (for debouncing)
    uint8_t lapCount;               // Number of completed laps
    LapData laps[MAX_LAPS];         // Array of lap data
};

/**
 * @brief Race data for the entire race
 */
struct RaceData {
    RaceMode mode;                  // Race mode
    RaceStatus status;              // Race status
    unsigned long startTime;        // Race start time
    unsigned long pauseTime;        // Time when race was paused
    unsigned long elapsedTime;      // Total elapsed time (accounting for pauses)
    unsigned long endTime;          // Race end time
    uint8_t activeLaneCount;        // Number of active lanes
    uint8_t targetLapCount;         // Target number of laps for LAPS mode
    unsigned long targetRaceTime;   // Target race time for TIMER mode (in ms)
    unsigned long debounceTime;     // Sensor debounce time (in ms)
    LaneData lanes[MAX_LANES];      // Array of lane data
};

/**
 * @brief Error codes for standardized error handling
 */
enum class ErrorCode {
    SUCCESS = 0,                // No error
    NOT_INITIALIZED = 1,        // Module not initialized
    INVALID_STATE = 2,          // Operation invalid in current state
    INVALID_PARAMETER = 3,      // Invalid parameter provided
    TIMEOUT = 4,                // Operation timed out
    HARDWARE_ERROR = 5,         // Hardware-related error
    CONFIGURATION_ERROR = 6,    // Configuration-related error
    COMMUNICATION_ERROR = 7,    // Communication-related error
    RESOURCE_ERROR = 8,         // Resource allocation error
    NOT_IMPLEMENTED = 9,        // Feature not implemented
    UNKNOWN_ERROR = 255         // Unknown error
};

/**
 * @brief Error information structure
 */
struct ErrorInfo {
    ErrorCode code;             // Error code
    const char* message;        // Error message
    const char* module;         // Module where error occurred
    
    ErrorInfo() : code(ErrorCode::SUCCESS), message(""), module("") {}
    
    ErrorInfo(ErrorCode c, const char* msg, const char* mod) 
        : code(c), message(msg), module(mod) {}
        
    // Helper method to check if operation was successful
    bool isSuccess() const { return code == ErrorCode::SUCCESS; }
};

/**
 * @brief Input source identifiers
 */
enum class InputSourceId {
    KEYBOARD,   // Input from keyboard
    TOUCH,     // Input from physical buttons
    SENSOR,     // Input from sensors
    WEB,        // Input from web interface
    UNKNOWN     // Unknown input source
};
