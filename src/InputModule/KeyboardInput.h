#pragma once
#include "InputModule.h"
#include "common/TimeManager.h"
#include "common/Types.h"

/**
 * @brief Handles all keyboard commands for race control and configuration
 * 
 * Output Standardization:
 * - All outputs are standardized InputEvent structs using the shared InputCommand enum.
 * - Output format must match SensorInput and ButtonInput for corresponding commands (e.g., AddLap for lane 1 is identical regardless of source).
 * - KeyboardInput can send ALL supported commands (letters, numbers, multi-step commands).
 *
 * Prompt-driven multi-step commands:
 * - m: Change race mode (prompts for selection)
 * - n: Set number of laps (prompts for value)
 * - t: Set race time (prompts for mm:ss)
 *
 * For each, the system will prompt for additional input.
 *
 * Supported key mappings:
 * - s or S: Start Race
 * - p or P: Pause Race
 * - r or R: Resume Race
 * - t or T: Stop Race
 * - x or X: Reset Race
 * - l: Set number of lanes (prompts for value)
 * - 1-8: Simulate sensor trigger for lane 1-8
 * - d or -: Remove lap (manual correction)
 * - b: Toggle best lap tracking
 * - f: Toggle first trigger ignore / reaction time mode
 * - c: Enter configuration menu
 * - e: Enable/disable lane (prompts for lane number)
 * - a: Add racer
 * - z: Remove racer
 *
 * For multi-step commands (n, l, m, e), the system will prompt for additional input.
 */
// Keyboard input states
enum class KeyboardState {
    Idle,               // No input expected
    WaitModeNumber,     // Race mode selection (1-4)
    WaitLapNumber,     // Number of laps (1-999)
    WaitRaceTime,      // Race time in seconds or MM:SS
    WaitLaneNumber,    // Lane number (1-8)
    WaitLanesNumber,   // Number of lanes (1-8)
    WaitCountdownInterval // Countdown interval (1-999)
};

class KeyboardInput : public InputModule {
private:
    // Input buffer for multi-character input
    static constexpr uint8_t INPUT_BUFFER_SIZE = 8;
    char inputBuffer[INPUT_BUFFER_SIZE] = {0};
    uint8_t inputPos = 0;
    char digitBuffer[4] = {0};
    uint8_t digitCount = 0;
    KeyboardState kbdState = KeyboardState::Idle;
    char pendingCommand = 0; // 'e' for enable, 'd' for disable, etc.
    
    /**
     * @brief Reset the input state to idle
     */
    void resetInputState();
    
    /**
     * @brief Add a character to the input buffer
     * @param c Character to add
     * @return true if successful, false if buffer is full
     */
    bool addToInputBuffer(char c);
    
    /**
     * @brief Get the current input as an integer
     * @return int The parsed integer value
     */
    int getInputValue() const;
    
    /**
     * @brief Handle numeric input based on current state
     * @param c The numeric character received
     * @param event Reference to store the output event
     * @return true if an event was generated, false otherwise
     */
    bool handleNumericInput(char c, InputEvent& event);
    
    /**
     * @brief Handle the Enter key based on current state
     * @param event Reference to store the output event
     * @return true if an event was generated, false otherwise
     */
    bool handleEnterKey(InputEvent& event);
    
public:
    /**
     * @brief Initialize the keyboard input module
     * 
     * This should be called once during system startup.
     * 
     * @return bool true if initialization was successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Poll for keyboard input events
     * 
     * @param event Reference to store the received event
     * @return bool true if an event was received, false otherwise
     */
    bool poll(InputEvent& event) override;
    
private:
    bool _initialized = false;
};
