#pragma once
#include <Arduino.h>
#include <Ticker.h>  // For Timer functionality
#include "InputModule/InputCommand.h"
#include "InputModule/InputManager.h"
#include "common/Types.h"
#include "common/TimeManager.h"
#include "RaceModule/RaceModule.h"
#include "DisplayModule/DisplayManager.h"
#include "LightsModule/LightsModule.h"
#include "ConfigModule/ConfigModule.h"

/**
 * @brief User selection options from the main menu
 */
enum class UserSelection { 
    StartRace,      // Start a race (legacy)
    ChangeConfig,   // Change configuration (legacy)
    ViewStats,      // View statistics (legacy)
    Race,           // Race menu option
    Config,         // Configuration menu option
    Stats           // Statistics menu option
};

/**
 * @brief System state
 */
enum class SystemState {
    Main,   // Showing main menu
    RaceMode,   // In race mode
    ConfigMode, // In configuration mode
    StatsMode   // In statistics mode
};

/**
 * @brief Central controller for the lap counter system
 * 
 * The SystemController acts as the central decision maker for the entire system.
 * It processes commands from various input sources (standardized by the InputModule)
 * and routes them to the appropriate modules (Race or Config).
 * 
 * It also manages the main menu and overall application flow.
 * 
 * This class follows the Singleton pattern to ensure only one instance exists.
 */
class SystemController {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return SystemController& The singleton instance
     */
    static SystemController& getInstance();
    
    /**
     * @brief Initialize the system
     * 
     * This should be called once during system startup.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Update the system state
     * 
     * This should be called regularly in the main loop.
     */
    void update();
    
    /**
     * @brief Show the main menu
     * 
     * Displays the main menu and processes user input.
     */
    void showMain();
    
    /**
     * @brief Process a main menu selection
     * 
     * @param selection The user's selection
     */
    void processMainSelection(UserSelection selection);
    
    /**
     * @brief Show the race menu
     * 
     * Displays the race menu and processes user input.
     */
    void showRaceReady();
    
    /**
     * @brief Show the race active screen
     * 
     * Displays the race active screen with the current race mode.
     */
    void showRaceActive();
    
    /**
     * @brief Start a race with countdown
     * 
     * Initiates the race start sequence with countdown.
     */
    void startRaceWithCountdown();
    
    /**
     * @brief Process an input event
     * 
     * @param event The input event to process
     */
    void processInputEvent(const InputEvent& event);
    
private:
    // Private constructor for singleton pattern
    SystemController();
    
    // Static instance pointer
    static SystemController* _instance;
    
    // System state
    SystemState _systemState;
    
    // Module references
    InputManager& inputManager;
    RaceModule& raceModule;
    DisplayManager& displayManager;
    LightsModule& lightsModule;
    ConfigModule& configModule;
    
    // Initialization flag
    bool _initialized;
    
    // Event handlers for observer pattern
    void onRaceStateChanged(RaceState state);
    void onSecondTick(uint32_t raceTimeMs);
    void onLapRegistered(int lane, uint32_t lapTimeMs);
    
    // Helper methods for formatting time
    String formatTimeMMSS(uint32_t timeMs);
    String formatTimeMMSSmmm(uint32_t timeMs);
    
    /**
     * @brief Create a snapshot of the current race data for display
     * 
     * This method collects race data from RaceModule and formats it for display
     * on the RaceActiveScreen UI. It includes position, lane number, last lap time,
     * and current time for each active lane.
     * 
     * @return std::vector<RaceLaneData> Vector of lane data for display
     */
    std::vector<RaceLaneData> createRaceDataSnapshot();
};
