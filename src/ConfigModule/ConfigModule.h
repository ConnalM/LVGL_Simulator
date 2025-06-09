#pragma once
#include <Arduino.h>
#include "common/Types.h"
#include "common/TimeManager.h"
#include "ConfigSettings.h"

/**
 * @brief Module for managing system configuration settings
 * 
 * The ConfigModule handles all configuration settings for the system,
 * including race parameters, lane settings, and mode selection.
 * It provides methods for updating settings and persisting them to storage.
 */
class ConfigModule {
public:
    /**
     * @brief Constructor
     */
    ConfigModule();
    
    /**
     * @brief Initialize the module
     * 
     * This should be called once during system startup.
     * Loads settings from storage and initializes the configuration.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Update the module state
     * 
     * This should be called regularly in the main loop.
     * Currently does nothing but included for consistency with other modules.
     */
    void update();
    
    /**
     * @brief Handle setting the number of laps
     * 
     * @param laps Number of laps to set (1-99)
     */
    void handleSetLaps(uint8_t laps);
    
    /**
     * @brief Handle setting the number of lanes
     * 
     * @param lanes Number of lanes to set (1-8)
     */
    void handleSetLanes(uint8_t lanes);
    
    /**
     * @brief Handle setting the race mode
     * 
     * @param mode Race mode to set (0-3)
     */
    void handleSetRaceMode(uint8_t mode);
    
    /**
     * @brief Print current settings to Serial
     */
    void printSettingsToSerial() const;
    
    /**
     * @brief Get the number of laps
     * 
     * @return int Number of laps
     */
    int getNumLaps() const { return settings.numLaps; }
    
    /**
     * @brief Get the number of lanes
     * 
     * @return int Number of lanes
     */
    int getNumLanes() const { return settings.numLanes; }
    
    /**
     * @brief Get the race mode
     * 
     * @return RaceMode Current race mode
     */
    RaceMode getRaceMode() const { return static_cast<RaceMode>(settings.raceMode); }

private:
    ConfigSettings settings;
    bool _initialized = false;
    
    /**
     * @brief Save settings to persistent storage
     */
    void saveSettings();
    
    /**
     * @brief Load settings from persistent storage
     */
    void loadSettings();
    
    /**
     * @brief Display feedback message
     * 
     * @param msg Message to display
     * @param error Whether this is an error message
     */
    void feedback(const char* msg, bool error = false);
};

// Global instance declaration
extern ConfigModule configModule;
