#pragma once
#include <Arduino.h>
#include <functional>
#include "common/TimeManager.h"
#include <TickTwo.h>

/**
 * @brief Enumeration for light states during countdown
 */
enum class LightState {
    Off,        // All lights off
    Ready,      // Ready state (usually yellow)
    RedOn,      // Red lights on
    RedOff,     // Red lights off (race start trigger)
    GreenOn     // Green lights on (visual confirmation of start)
};

// Callback function types for observer pattern
using LightStateChangedCallback = std::function<void(LightState)>;
using CountdownStepCallback = std::function<void(int)>;
using CountdownCompletedCallback = std::function<void()>;

/**
 * @brief Module for handling race start light sequences
 * 
 * This module manages the countdown sequence for race starts,
 * including visual indicators and timing.
 */
class LightsModule {
public:
    /**
     * @brief Initialize the module
     * 
     * This should be called once during system startup.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Starts the countdown sequence
     * 
     * @param intervalMs Controls the delay between steps (default: 1000ms)
     */
    void startSequence(uint32_t intervalMs = 1000);

    /**
     * @brief Update the module state
     * 
     * Should be called regularly in the main loop to update countdown state.
     */
    void update();

    /**
     * @brief Handles how each countdown step is displayed or communicated
     * 
     * @param number The countdown number to display
     */
    void displayCountdown(int number);

    /**
     * @brief Check if the countdown is active
     * 
     * @return bool true if countdown is active, false otherwise
     */
    bool isActive() const;

    /**
     * @brief Set the countdown start value
     * 
     * For testing: manually set the countdown start value
     * 
     * @param startValue The value to start the countdown from
     */
    void setCountdownStart(int startValue);
    
    /**
     * @brief Set the countdown interval
     * 
     * For testing: set the interval between countdown steps
     * 
     * @param intervalMs The interval in milliseconds (default: 1000)
     */
    void setCountdownInterval(uint32_t intervalMs);
    
    /**
     * @brief Get the current countdown step
     * 
     * @return int The current countdown number
     */
    int getCurrentStep() const;
    
    /**
     * @brief Get the countdown interval
     * 
     * @return uint32_t The interval in milliseconds
     */
    uint32_t getCountdownInterval() const;

    /**
     * @brief Get the current light state
     * 
     * @return LightState The current light state
     */
    LightState getLightState() const;

    /**
     * @brief Set callback for light state changes
     * 
     * @param callback Function to call when light state changes
     */
    void setOnLightStateChangedCallback(LightStateChangedCallback callback);
    
    /**
     * @brief Set callback for countdown steps
     * 
     * @param callback Function to call for each countdown step
     */
    void setOnCountdownStepCallback(CountdownStepCallback callback);
    
    /**
     * @brief Set callback for countdown completion
     * 
     * @param callback Function to call when countdown completes
     */
    void setOnCountdownCompletedCallback(CountdownCompletedCallback callback);

private:
    uint32_t _intervalMs = 1000;
    int _currentStep = 0;
    int _countdownStart = 5;
    uint32_t _lastStepTime = 0;
    bool _active = false;
    bool _initialized = false;
    LightState _currentLightState = LightState::Off;
    
    // TickTwo timer for countdown sequence
    TickTwo* _countdownTimer = nullptr;
    
    // Callbacks
    LightStateChangedCallback _onLightStateChangedCallback = nullptr;
    CountdownStepCallback _onCountdownStepCallback = nullptr;
    CountdownCompletedCallback _onCountdownCompletedCallback = nullptr;
    
    /**
     * @brief Called at the end of the countdown
     * 
     * Triggers race start actions
     */
    void triggerGo();
    
    /**
     * @brief Countdown timer callback method
     * 
     * This is called by the TickTwo timer at each countdown step
     */
    void countdownStep();
    
    /**
     * @brief Static wrapper for the countdown timer callback
     * 
     * This is needed because TickTwo requires a static callback function
     */
    static void staticCountdownCallback();
    
    /**
     * @brief Set the light state and notify observers
     * 
     * @param state The new light state
     */
    void setLightState(LightState state);
};
