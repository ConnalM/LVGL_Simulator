#pragma once
#include <cstdint>

// Target module for an input event
enum class InputTarget { Race, Config };

/**
 * InputEvent.timestamp: Synchronized hardware timer value in milliseconds.
 * Uses TimeManager::GetInstance().GetCurrentTimeMs() for consistent timing across the system.
 * All InputEvent timestamps must use the same timebase for consistency across modules and devices.
 * InputEvent.target: Specifies which module (Race or Config) should process this event.
 */
enum class InputCommand {
    // ===== SCREEN NAVIGATION COMMANDS =====
    // Commands that change which screen is displayed
    EnterMain,          // Navigate to main menu screen
    EnterRaceReady,     // Navigate to race ready screen (race setup)
    StartRace,          // Start the race (after countdown)
    EnterConfig,        // Navigate to configuration screen
    EnterStats,         // Navigate to statistics screen
    ReturnToPrevious,   // Return to previous screen from any screen
    
    // ===== ACTION COMMANDS =====
    // Commands that change program state or trigger actions
    AddLap,             // Register a lap for a lane
    RemoveLap,          // Remove the last lap for a lane
    StartCountdown,     // Start the countdown sequence
    PauseRace,          // Pause an active race
    ResumeRace,         // Resume a paused race
    StopRace,           // Stop the current race
    ResetRace,          // Reset the race data
    SetNumLaps,         // Set the number of laps for the race
    SetNumLanes,        // Set the number of lanes for the race
    ChangeMode,         // Change the race mode
    SetRaceTime,        // Set race time in seconds (value = seconds)
    ToggleBestLap,      // Toggle display of best lap times
    ToggleReactionTime, // Toggle display of reaction times
    EnableLane,         // Enable a specific lane
    DisableLane,        // Disable a specific lane
    AddRacer,           // Add a racer to the system
    RemoveRacer,        // Remove a racer from the system
    SetCountdownInterval // Set countdown interval in seconds
    // Extend as needed
};

inline InputTarget getDefaultTargetForCommand(InputCommand command) {
    switch (command) {
        // Screen navigation commands - all routed to SystemController via Race target
        case InputCommand::EnterMain:
        case InputCommand::EnterRaceReady:
        case InputCommand::StartRace:  // Start the race (after countdown)
        case InputCommand::EnterStats:
        case InputCommand::ReturnToPrevious:
        // Race action commands
        case InputCommand::AddLap:
        case InputCommand::RemoveLap:
        case InputCommand::StartCountdown:  // Changed from StartRace
        case InputCommand::PauseRace:
        case InputCommand::ResumeRace:
        case InputCommand::StopRace:
        case InputCommand::ResetRace:
        case InputCommand::ToggleBestLap:
        case InputCommand::SetCountdownInterval:
            return InputTarget::Race;
        case InputCommand::SetNumLaps:
        case InputCommand::SetNumLanes:
        case InputCommand::ChangeMode:
        case InputCommand::ToggleReactionTime:
        case InputCommand::EnterConfig:
        case InputCommand::EnableLane:
        case InputCommand::DisableLane:
        case InputCommand::AddRacer:
        case InputCommand::RemoveRacer:
            return InputTarget::Config;
        default:
            return InputTarget::Race; // Fallback
    }
}

struct InputEvent {
    InputCommand command;     // The action (AddLap, StartRace, etc.)
    int sourceId;             // Lane number, button ID, or 0 if not applicable
    int value;                // Additional value if needed (e.g., number of laps, mode, etc.)
    uint32_t timestamp;       // Synchronized timer value in ms (see above)
    InputTarget target;       // NEW: destination module for this event (Race or Config)
};
