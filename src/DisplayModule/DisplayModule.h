#pragma once

#ifndef SIMULATOR
#include <Arduino.h>
#else
// Include Arduino compatibility layer for simulator
#include "../common/ArduinoCompat.h"
#endif
#include <vector>
#include "../common/Types.h"

// Forward declaration for RaceLaneData
struct RaceLaneData;

/**
 * @brief Display types supported by the system
 */
enum class DisplayType {
    Serial,     // Serial monitor output
    LCD,        // Physical ESP32-8048S070C display
    Web         // Web page display
};

/**
 * @brief Base interface for display implementations
 * 
 * This abstract base class defines the core interface that all
 * display implementations must support, focusing on basic text output.
 */
class IBaseDisplay {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~IBaseDisplay() {}
    
    /**
     * @brief Initialize the display
     * 
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    virtual bool initialize() = 0;
    
    /**
     * @brief Update the display
     * 
     * This should be called regularly in the main loop.
     */
    virtual void update() = 0;
    
    /**
     * @brief Clear the display
     */
    virtual void clear() = 0;
    
    /**
     * @brief Print a message to the display
     * 
     * @param message The message to print
     * @param newLine Whether to add a newline
     */
    virtual void print(const String& message, bool newLine = true) = 0;
    
    /**
     * @brief Print a formatted message to the display
     * 
     * @param format The format string
     * @param ... The format arguments
     */
    virtual void printf(const char* format, ...) = 0;
    
    /**
     * @brief Get the display type
     * 
     * @return DisplayType The display type
     */
    virtual DisplayType getDisplayType() const = 0;
};

/**
 * @brief Extended interface for graphical display implementations
 * 
 * This interface extends IBaseDisplay with methods for graphical operations
 * like drawing shapes and setting text properties.
 */
class IGraphicalDisplay : public IBaseDisplay {
public:
    /**
     * @brief Set cursor position
     * 
     * @param x X coordinate
     * @param y Y coordinate
     */
    virtual void setCursor(int x, int y) = 0;
    
    /**
     * @brief Set text color
     * 
     * @param color Color value
     */
    virtual void setTextColor(uint32_t color) = 0;
    
    /**
     * @brief Set text size
     * 
     * @param size Text size multiplier
     */
    virtual void setTextSize(uint8_t size) = 0;
    
    /**
     * @brief Draw a rectangle
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Color value
     */
    virtual void drawRect(int x, int y, int w, int h, uint32_t color) = 0;
    
    /**
     * @brief Fill a rectangle
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Color value
     */
    virtual void fillRect(int x, int y, int w, int h, uint32_t color) = 0;
    
    /**
     * @brief Draw a circle
     * 
     * @param x X coordinate of center
     * @param y Y coordinate of center
     * @param r Radius
     * @param color Color value
     */
    virtual void drawCircle(int x, int y, int r, uint32_t color) = 0;
    
    /**
     * @brief Fill a circle
     * 
     * @param x X coordinate of center
     * @param y Y coordinate of center
     * @param r Radius
     * @param color Color value
     */
    virtual void fillCircle(int x, int y, int r, uint32_t color) = 0;
    
    /**
     * @brief Get the width of the display
     * 
     * @return int Display width in pixels
     */
    virtual int getWidth() const = 0;
    
    /**
     * @brief Get the height of the display
     * 
     * @return int Display height in pixels
     */
    virtual int getHeight() const = 0;
    
    /**
     * @brief Draw main screen with buttons for Race, Config, and Stats
     */
    virtual void drawMain() = 0;
    
    /**
     * @brief Draw the race ready screen
     * 
     * This method is responsible for displaying the race ready screen
     * which allows the user to configure race settings and start the race.
     */
    virtual void drawRaceReady() = 0;
    
    /**
     * @brief Draw the configuration screen
     */
    virtual void drawConfig() = 0;
    
    /**
     * @brief Draw the active race screen
     * 
     * @param raceMode The current race mode
     */
    virtual void drawRaceActive(RaceMode raceMode) = 0;
    
    /**
     * @brief Start the light sequence on the RaceReadyScreen
     * 
     * This method is responsible for starting the countdown light sequence
     * on the RaceReadyScreen. It should be called by the SystemController
     * through the DisplayManager when a race start is requested.
     */
    virtual void startLightSequence() = 0;
    
    /**
     * @brief Update the race data display with current lane data
     * 
     * This method is responsible for updating the race data display on the
     * RaceActive screen with the current lane data from the RaceModule.
     * 
     * @param laneData Vector of lane data to display
     */
    virtual void updateRaceData(const std::vector<RaceLaneData>& laneData) = 0;
    
    /**
     * @brief Draw the statistics screen
     * 
     * This method is responsible for displaying the statistics screen
     * which shows race statistics and historical data.
     */
    virtual void drawStats() = 0;

    /**
     * @brief Draw the pause screen
     * 
     * This method displays the pause screen with alternating yellow lights
     * and resume/stop buttons.
     */
    virtual void drawPause() = 0;

    /**
     * @brief Draw the stop screen
     * 
     * This method displays the stop screen with a stop light and stop button.
     */
    virtual void drawStop() = 0;
};

/**
 * @brief Typedef for backward compatibility
 * 
 * This allows existing code to continue using IDisplay
 * while we transition to the new interface hierarchy.
 */
typedef IBaseDisplay IDisplay;
