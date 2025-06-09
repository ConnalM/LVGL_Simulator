#pragma once

#ifdef SIMULATOR
#include <string>
#else
#include <Arduino.h>
#endif

#include "common/Types.h"
#include "RaceModule/RaceModule.h"
#include "DisplayModule/DisplayModule.h"
#include "DisplayModule/DisplayFactory.h"

/**
 * @brief Screen types for the display
 */
enum class ScreenType {
    Main,       // Main menu screen
    RaceReady,  // Race ready screen (configuration and countdown)
    Config,     // Configuration menu
    RaceActive, // Active race display
    Stats,      // Statistics screen
    Pause,      // Pause screen
    Stop        // Stop screen
};

/**
 * @brief Manager for handling display content formatting
 * 
 * This module sits between the SystemController and display implementations.
 * It formats data for different displays and sends it to the appropriate display.
 */
class DisplayManager {
public:
    /**
     * @brief Log level enumeration for system messages
     */
    enum class LogLevel {
        Debug,    // Detailed debug information
        Info,     // General information messages
        Warning,  // Warning messages
        Error     // Error messages
    };

    /**
     * @brief Get the singleton instance
     * 
     * @return DisplayManager& The singleton instance
     */
    static DisplayManager& getInstance();
    
    /**
     * @brief Initialize the display manager
     * 
     * @param displayTypes Array of display types to initialize
     * @param count Number of display types in the array
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    bool initialize(DisplayType* displayTypes, int count);
    
    /**
     * @brief Initialize a single display type
     * 
     * @param displayType The display type to initialize
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    bool initializeDisplay(DisplayType displayType);
    
    /**
     * @brief Update the display manager
     * 
     * This should be called regularly in the main loop.
     */
    void update();
    
    /**
     * @brief Set the current screen
     * 
     * @param screen The screen to display
     */
    void setScreen(ScreenType screen);
    
    /**
     * @brief Show the main screen
     */
    void showMain();
    
    /**
     * @brief Show the race ready screen
     * 
     * @param raceMode The current race mode
     * @param numLaps The number of laps
     * @param numLanes The number of lanes
     * @param countdownInterval The countdown interval in ms
     */
    void showRaceReady(RaceMode raceMode, int numLaps, int numLanes, uint32_t countdownInterval);
    
    /**
     * @brief Show the config screen
     */
    void showConfig();
    
    /**
     * @brief Show the countdown (part of RaceReady screen)
     * 
     * @param currentStep The current countdown step
     * @param isComplete Whether the countdown is complete
     */
    void showCountdown(int currentStep, bool isComplete = false);
    
    /**
     * @brief Start the light sequence on the RaceReadyScreen
     */
    void startLightSequence();
    
    /**
     * @brief Show the active race screen
     * 
     * @param raceMode The current race mode
     */
    void showRaceActive(RaceMode raceMode);
    
    /**
     * @brief Show the statistics screen
     */
    void showStats();
    
    /**
     * @brief Update the race data display with current lane data
     * 
     * @param laneData Vector of lane data to display
     */
    void updateRaceData(const std::vector<RaceLaneData>& laneData);
    
    /**
     * @brief Show the race status
     * 
     * @param raceModule Reference to the race module
     * @param isPaused Whether the race is paused
     */
    void showRaceStatus(const RaceModule& raceModule, bool isPaused);
    
    // Removed showRaceScreenMenu as it's now part of showRaceReady
    
    /**
     * @brief Show a message
     * 
     * @param message The message to show
     */
    void showMessage(const String& message);
    
    /**
     * @brief Get the current screen type
     * 
     * @return ScreenType The current screen
     */
    ScreenType getCurrentScreen() const;
    
    /**
     * @brief Get the active display types
     * 
     * @param displayTypes Array to store the active display types
     * @param maxCount Maximum number of display types to store
     * @return int Number of active display types
     */
    int getActiveDisplayTypes(DisplayType* displayTypes, int maxCount) const;

    /**
     * @brief Log a message with the specified log level
     * 
     * @param level The log level
     * @param message The message to log
     * @param moduleName Optional module name for context
     */
    void log(LogLevel level, const String& message, const String& moduleName = "");
    
    /**
     * @brief Log a debug message
     * 
     * @param message The message to log
     * @param moduleName Optional module name for context
     */
    void debug(const String& message, const String& moduleName = "");
    
    /**
     * @brief Log an info message
     * 
     * @param message The message to log
     * @param moduleName Optional module name for context
     */
    void info(const String& message, const String& moduleName = "");
    
    /**
     * @brief Log a warning message
     * 
     * @param message The message to log
     * @param moduleName Optional module name for context
     */
    void warning(const String& message, const String& moduleName = "");
    
    /**
     * @brief Log an error message
     * 
     * @param message The message to log
     * @param moduleName Optional module name for context
     */
    void error(const String& message, const String& moduleName = "");

    /**
     * @brief Log race status messages to Serial with "LC: " prefix
     * 
     * @param message The race status message
     */
    void raceLog(const String& message);

    /**
     * @brief Format the race status display for logging
     * 
     * @param raceModule Reference to the race module
     * @param isPaused   Whether the race is paused
     * @return String    Formatted race status for logs
     */
    String formatRaceStatus(const RaceModule& raceModule, bool isPaused);

private:
    /**
     * @brief Private constructor (Singleton pattern)
     */
    DisplayManager();
    
    /**
     * @brief Private copy constructor (Singleton pattern)
     */
    DisplayManager(const DisplayManager&) = delete;
    
    /**
     * @brief Private assignment operator (Singleton pattern)
     */
    DisplayManager& operator=(const DisplayManager&) = delete;
    
    /**
     * @brief Format the countdown display
     * 
     * @param currentStep The current countdown step
     * @param isComplete Whether the countdown is complete
     * @return String The formatted countdown display
     */
    String formatCountdown(int currentStep, bool isComplete);
    
    /**
     * @brief Format the time in MM:SS.mmm format
     * 
     * @param timeMs The time in milliseconds
     * @return String The formatted time
     */
    String formatTimeMMSSmmm(uint32_t timeMs);

    // Static instance for singleton pattern
    static DisplayManager* _instance;
    
    // Active displays (can have multiple active at once)
    IBaseDisplay* _activeDisplays[3]; // Serial, LCD, Web
    DisplayType _activeDisplayTypes[3]; // Types of active displays
    int _activeDisplayCount;
    
    // Current screen
    ScreenType _currentScreen = ScreenType::Main;
    
    // Flag to track if initialization has been performed
    bool _initialized = false;
    
    // Countdown display state
    String _countdownDisplay;
};
