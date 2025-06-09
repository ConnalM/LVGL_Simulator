#include "SystemController.h"
#include "ConfigModule/ConfigModule.h"
#include "LightsModule/LightsModule.h"
#include "DisplayModule/DisplayManager.h"
#include <algorithm>
#include "../ModuleToggle.h"
#include <Arduino.h>

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        static bool firstCall = true; \
        unsigned long now = millis(); \
        if (firstCall || (now - lastDebugPrint > DEBUG_THROTTLE_MS)) { \
            DisplayManager::getInstance().debug(String("[SystemController] ") + __FUNCTION__, "SystemController"); \
            lastDebugPrint = now; \
            firstCall = false; \
        } \
    } while(0)

// External references
extern ConfigModule configModule;
extern LightsModule lightsModule;

// Helper function for InputCommand to string (static to prevent multiple definition errors)
static const char* inputCommandToString(InputCommand cmd) {
    switch (cmd) {
        case InputCommand::AddLap: return "AddLap";
        case InputCommand::RemoveLap: return "RemoveLap";
        case InputCommand::StartCountdown: return "StartCountdown";
        case InputCommand::PauseRace: return "PauseRace";
        case InputCommand::ResumeRace: return "ResumeRace";
        case InputCommand::StopRace: return "StopRace";
        case InputCommand::ResetRace: return "ResetRace";
        case InputCommand::SetNumLaps: return "SetNumLaps";
        case InputCommand::SetNumLanes: return "SetNumLanes";
        case InputCommand::ChangeMode: return "ChangeMode";
        case InputCommand::SetRaceTime: return "SetRaceTime";
        case InputCommand::ToggleBestLap: return "ToggleBestLap";
        case InputCommand::ToggleReactionTime: return "ToggleReactionTime";
        case InputCommand::EnterConfig: return "EnterConfig";
        case InputCommand::EnableLane: return "EnableLane";
        case InputCommand::DisableLane: return "DisableLane";
        case InputCommand::AddRacer: return "AddRacer";
        case InputCommand::RemoveRacer: return "RemoveRacer";
        case InputCommand::SetCountdownInterval: return "SetCountdownInterval";
        default: return "Unknown";
    }
}

// Static instance for singleton pattern
SystemController* SystemController::_instance = nullptr;

SystemController& SystemController::getInstance() {
    if (_instance == nullptr) {
        _instance = new SystemController();
    }
    return *_instance;
}

SystemController::SystemController()
    : _systemState(SystemState::Main)
    , inputManager(InputManager::getInstance())
    , raceModule(RaceModule::getInstance())
    , displayManager(DisplayManager::getInstance())
    , lightsModule(*(new LightsModule()))  // Temporary until we have a proper getInstance()
    , configModule(*(new ConfigModule()))  // Temporary until we have a proper getInstance()
    , _initialized(false) {
    DEBUG_PRINT_METHOD();
    // Constructor implementation
}

bool SystemController::initialize() {
    DEBUG_PRINT_METHOD();
    // Skip if already initialized
    if (_initialized) {
        displayManager.info("Already initialized", "SystemController");
        return true;
    }
    
    displayManager.info("Initializing...", "SystemController");
    
    // Ensure TimeManager is initialized
    if (!TimeManager::GetInstance().Initialize()) {
        displayManager.error("Failed to initialize TimeManager", "SystemController");
        return false;
    }
    
    // Initialize all modules
    #ifdef ENABLE_RACEMODULE
        if (!raceModule.initialize()) {
            displayManager.error("Failed to initialize RaceModule", "SystemController");
            return false;
        }
    #else  // !ENABLE_RACEMODULE
        displayManager.debug("RaceModule disabled", "SystemController");
    #endif

    #ifdef ENABLE_LIGHTSMODULE
        if (!lightsModule.initialize()) {
            displayManager.error("Failed to initialize LightsModule", "SystemController");
            return false;
        }
    #else  // !ENABLE_LIGHTSMODULE
        displayManager.debug("LightsModule disabled", "SystemController");
    #endif

    #ifdef ENABLE_DISPLAYMODULE
        DisplayType displayTypes[] = {DisplayType::Serial};
        if (!displayManager.initialize(displayTypes, 1)) {
            displayManager.error("Failed to initialize DisplayManager", "SystemController");
            return false;
        }
    #else  // !ENABLE_DISPLAYMODULE
        displayManager.debug("DisplayModule disabled", "SystemController");
    #endif

    #ifdef ENABLE_INPUTMODULE
        if (!inputManager.initialize()) {
            displayManager.error("Failed to initialize InputManager", "SystemController");
            return false;
        }
    #else  // !ENABLE_INPUTMODULE
        displayManager.debug("InputModule disabled", "SystemController");
    #endif

    #ifdef ENABLE_CONFIGMODULE
        if (!configModule.initialize()) {
            displayManager.error("Failed to initialize ConfigModule", "SystemController");
            return false;
        }
    #else  // !ENABLE_CONFIGMODULE
        displayManager.debug("ConfigModule disabled", "SystemController");
    #endif
    
    // Set up event handlers for observer pattern
    raceModule.setOnRaceStateChangedCallback([this](RaceState state) {
        this->onRaceStateChanged(state);
    });
    
    raceModule.setOnSecondTickCallback([this](uint32_t raceTimeMs) {
        this->onSecondTick(raceTimeMs);
    });
    
    raceModule.setOnLapRegisteredCallback([this](int lane, uint32_t lapTimeMs) {
        this->onLapRegistered(lane, lapTimeMs);
    });
    
    // Register countdown step callback with LightsModule
    lightsModule.setOnCountdownStepCallback([this](int step) {
        // Update the countdown display on all displays
        displayManager.showCountdown(step);
    });
    
    // Register countdown completed callback with LightsModule
    lightsModule.setOnCountdownCompletedCallback([this]() {
        // Show the final GO! on all displays
        displayManager.showCountdown(0, true);
        
        // Start the race immediately
        raceModule.startRace();
        
        // Set a timer to switch to race active screen after 1 second
        static Ticker raceStartTimer;
        raceStartTimer.once_ms(1000, +[](SystemController* sc) {
            sc->displayManager.setScreen(ScreenType::RaceActive);
        }, this);
    });
    
    // Set initial state
    _systemState = SystemState::Main;
    
    _initialized = true;
    return _initialized;
}

void SystemController::update() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Update all modules
    raceModule.update();
    lightsModule.update();
    
    // Poll for input events
    InputEvent event;
    if (inputManager.poll(event)) {
        processInputEvent(event);
    }
    
    // Update system state based on current mode
    switch (_systemState) {
        case SystemState::Main:
            // Main menu is handled by showMain()
            break;
            
        case SystemState::RaceMode:
            // Race mode updates are handled by RaceModule
            break;
            
        case SystemState::ConfigMode:
            // Config mode updates are handled by ConfigModule
            break;
            
        case SystemState::StatsMode:
            // Stats mode updates are handled by StatsModule
            break;
    }
}

void SystemController::showMain() {
    DEBUG_PRINT_METHOD();
    // Display the main menu
    displayManager.setScreen(ScreenType::Main);
    displayManager.showMain();
    
    // Set system state
    _systemState = SystemState::Main;
}

void SystemController::processMainSelection(UserSelection selection) {
    DEBUG_PRINT_METHOD();
    switch (selection) {
        case UserSelection::Race:
            // Show race menu
            showRaceReady();
            break;
            
        case UserSelection::Config:
            // Show config menu
            _systemState = SystemState::ConfigMode;
            displayManager.setScreen(ScreenType::Config);
            displayManager.showConfig();
            break;
            
        case UserSelection::Stats:
            // Show stats menu - use RaceReady as placeholder since StatsMenu doesn't exist
            _systemState = SystemState::StatsMode;
            displayManager.setScreen(ScreenType::RaceReady);
            displayManager.showMessage("Stats not implemented yet");
            break;
            
        default:
            // Invalid selection, stay on main menu
            showMain();
            break;
    }
}

void SystemController::showRaceReady() {
    DEBUG_PRINT_METHOD();
    displayManager.debug("Showing race ready screen", "SystemController");
    
    // Set system state
    _systemState = SystemState::RaceMode;
    
    // Get race configuration
    RaceMode raceMode = configModule.getRaceMode();
    uint8_t numLaps = configModule.getNumLaps();
    uint8_t numLanes = configModule.getNumLanes();
    uint32_t countdownInterval = lightsModule.getCountdownInterval();
    
    // Display the race ready screen with configuration
    displayManager.setScreen(ScreenType::RaceReady);
    displayManager.showRaceReady(raceMode, numLaps, numLanes, countdownInterval);
    
    displayManager.debug("showRaceReady() completed", "SystemController");
}

void SystemController::showRaceActive() {
    DEBUG_PRINT_METHOD();
    // Get the current race mode
    RaceMode currentRaceMode = raceModule.getRaceMode();
    
    // Display the race active screen with the current race mode
    Serial.println("[SystemController] Showing RaceActive screen");
    displayManager.setScreen(ScreenType::RaceActive);
    displayManager.showRaceActive(currentRaceMode);
    
    // System state should already be RaceMode, but set it just to be safe
    _systemState = SystemState::RaceMode;
}

void SystemController::startRaceWithCountdown() {
    DEBUG_PRINT_METHOD();
    displayManager.debug("Starting race with countdown", "SystemController");
    
    // Prepare the race with current configuration
    raceModule.prepareRace(
        configModule.getRaceMode(),
        configModule.getNumLanes(),
        configModule.getNumLaps(),
        0  // Use 0 as default since getRaceTimeSeconds is missing
    );
    
    // Start the countdown
    raceModule.startCountdown();
    
    // Make sure we're on the RaceReady screen
    displayManager.setScreen(ScreenType::RaceReady);
    
    // Start the light sequence on the display
    // This will set up the callback to update the DisplayManager with countdown steps
    displayManager.startLightSequence();
}

void SystemController::processInputEvent(const InputEvent& event) {
    DEBUG_PRINT_METHOD();
    // Skip debug print for AddLap events to avoid flooding the log
    if (event.command == InputCommand::AddLap) {
        return;
    }
    // Log the input event with more detailed information
    String eventInfo = "Input event: " + String(inputCommandToString(event.command));
    eventInfo += ", Target: " + String((int)event.target);
    eventInfo += ", Value: " + String(event.value);
    eventInfo += ", Current state: " + String((int)_systemState);
    displayManager.debug(eventInfo, "SystemController");
    
    // Add direct serial output for easier debugging
    Serial.print("[SystemController] Received event - Command: ");
    Serial.print(inputCommandToString(event.command));
    Serial.print(", Target: ");
    Serial.print((int)event.target);
    Serial.print(", Value: ");
    Serial.println(event.value);
    
    // Debug current screen type
    ScreenType currentScreen = displayManager.getCurrentScreen();
    displayManager.debug("Current screen: " + String((int)currentScreen), "SystemController");
    
    // Route entirely by target and command
    if (event.target == InputTarget::Race) {
        // Handle screen navigation commands first
        if (event.command == InputCommand::EnterMain) {
            displayManager.debug("Navigating to Main Menu", "SystemController");
            showMain();
            return;
        } 
        else if (event.command == InputCommand::EnterRaceReady) {
            displayManager.debug("Navigating to Race Ready Screen", "SystemController");
            showRaceReady();
            return;
        }
        else if (event.command == InputCommand::StartRace) {
            displayManager.debug("Race starting - starting race timer", "SystemController");
            _systemState = SystemState::RaceMode;
            
            // Log current race state to help diagnose issues
            RaceState currentState = raceModule.getRaceState();
            Serial.print("[SystemController] Current race state before starting: ");
            Serial.println(static_cast<int>(currentState));
            
            // If race is not in countdown or starting state, set it to starting state
            if (currentState != RaceState::Countdown && currentState != RaceState::Starting) {
                Serial.println("[SystemController] Setting race state to Starting before starting race");
                raceModule.startCountdown(); // This sets the state to Countdown
            }
            
            // Start the race timer immediately when green lights are shown
            Serial.println("[SystemController] Starting race timer");
            ErrorInfo result = raceModule.startRace();
            if (result.isSuccess()) {
                Serial.println("[SystemController] Race timer started successfully");
                
                // The RaceReadyScreen will hide itself after showing green lights
                // We'll transition to RaceActiveScreen immediately after race starts
                Serial.println("[SystemController] Race started successfully - will show RaceActive screen");
                
                // Note: We're not using a timer here to avoid potential memory issues
                // The RaceReadyScreen will still show green lights for 1 second before hiding
                // We'll transition to RaceActiveScreen right away so it's ready when RaceReadyScreen hides
                showRaceActive();
            } else {
                Serial.print("[SystemController] Failed to start race: ");
                Serial.println(result.message);
            }
            return;
        }
        else if (event.command == InputCommand::EnterStats) {
            displayManager.debug("Navigating to Stats Screen", "SystemController");
            _systemState = SystemState::StatsMode;
            displayManager.setScreen(ScreenType::RaceReady); // Placeholder until stats screen is implemented
            displayManager.showMessage("Stats not implemented yet");
            return;
        }
        else if (event.command == InputCommand::ReturnToPrevious) {
            // Handle return to previous screen based on current state
            displayManager.debug("Returning to previous screen", "SystemController");
            
            // Default to returning to main menu
            showMain();
            return;
        }
        
        // Handle action commands
        if (event.command == InputCommand::StartCountdown) {
            // Start the countdown sequence
            ScreenType currentScreen = displayManager.getCurrentScreen();
            
            if (currentScreen == ScreenType::RaceReady) {
                // If we're already on the race ready screen, start the countdown sequence
                displayManager.debug("Starting countdown sequence from RaceReady", "SystemController");
                
                // Call the proper method to start the countdown sequence
                startRaceWithCountdown();
            }
        } 
        else if (event.command == InputCommand::AddLap) {
            raceModule.registerLap(event.value);
        }
        else if (event.command == InputCommand::PauseRace) {
            Serial.println("[SystemController] Handling PauseRace command");
            displayManager.debug("Pausing race - switching to PauseScreen", "SystemController");
            ErrorInfo result = raceModule.pauseRace();
            if (result.isSuccess()) {
                Serial.println("[SystemController] Race paused successfully");
                displayManager.setScreen(ScreenType::Pause);
                Serial.println("[SystemController] Switched to PauseScreen");
            } else {
                Serial.print("[SystemController] Failed to pause race: ");
                Serial.println(result.message); // Using the message member directly
            }
            return;
        }
        else if (event.command == InputCommand::StopRace) {
            Serial.println("[SystemController] Handling StopRace command");
            displayManager.debug("Stopping race - switching to StopScreen", "SystemController");
            ErrorInfo result = raceModule.stopRace();
            if (result.isSuccess()) {
                Serial.println("[SystemController] Race stopped successfully");
                displayManager.setScreen(ScreenType::Stop);
                Serial.println("[SystemController] Switched to StopScreen");
            } else {
                Serial.print("[SystemController] Failed to stop race: ");
                Serial.println(result.message); // Using the message member directly
            }
            return;
        }
        else if (event.command == InputCommand::ResumeRace) {
            displayManager.debug("Resuming race - switching to RaceReady", "SystemController");
            
            // First stop the current race to reset race state to Idle
            // This ensures proper state transitions when starting a new countdown
            Serial.println("[SystemController] Stopping current race to reset state before resuming");
            raceModule.stopRace();
            
            // Then navigate to RaceReady screen
            showRaceReady();
            
            // The race will be resumed when the user starts the countdown again
            return;
        }
        // Handle other Race commands as needed (Reset, etc.)
        return;
    }
    if (event.target == InputTarget::Config) {
        // Debug the config command being processed
        displayManager.debug("Processing config command: " + String(inputCommandToString(event.command)), "SystemController");
        
        // EnterConfig should be handled above in the Race target section
        // This is kept for backward compatibility
        if (event.command == InputCommand::EnterConfig) {
            displayManager.debug("Entering config menu", "SystemController");
            _systemState = SystemState::ConfigMode;
            displayManager.setScreen(ScreenType::Config);
            displayManager.showConfig();
            return;
        } else if (event.command == InputCommand::ReturnToPrevious) {
            displayManager.debug("Returning to main menu from config", "SystemController");
            _systemState = SystemState::Main;
            displayManager.setScreen(ScreenType::Main);
            return;
        }
        
        // Process other configuration commands
        bool configChanged = false;
        
        switch (event.command) {
            case InputCommand::SetNumLaps:
                displayManager.debug("Setting number of laps to: " + String(event.value), "SystemController");
                configModule.handleSetLaps(event.value);
                configChanged = true;
                break;
                
            case InputCommand::SetNumLanes:
                displayManager.debug("Setting number of lanes to: " + String(event.value), "SystemController");
                configModule.handleSetLanes(event.value);
                configChanged = true;
                break;
                
            case InputCommand::ChangeMode:
                displayManager.debug("Changing race mode to: " + String(event.value), "SystemController");
                configModule.handleSetRaceMode(event.value - 1); // Convert 1-4 to 0-3 enum values
                configChanged = true;
                break;
                
            case InputCommand::SetRaceTime:
                displayManager.debug("Setting race time to: " + String(event.value) + " seconds", "SystemController");
                // Implementation for setting race time would go here
                displayManager.debug("Race time setting not implemented yet", "SystemController");
                configChanged = true;
                break;
                
            case InputCommand::ToggleReactionTime:
                displayManager.debug("Toggling reaction time", "SystemController");
                // Implementation for toggling reaction time would go here
                displayManager.debug("Reaction time toggle not implemented yet", "SystemController");
                configChanged = true;
                break;
                
            case InputCommand::EnableLane:
                displayManager.debug("Enabling lane: " + String(event.value), "SystemController");
                raceModule.enableLane(event.value);
                configChanged = true;
                break;
                
            case InputCommand::DisableLane:
                displayManager.debug("Disabling lane: " + String(event.value), "SystemController");
                raceModule.disableLane(event.value);
                configChanged = true;
                break;
                
            case InputCommand::AddRacer:
                displayManager.debug("Adding racer", "SystemController");
                // Implementation for adding racer would go here
                configChanged = true;
                break;
                
            case InputCommand::RemoveRacer:
                displayManager.debug("Removing racer", "SystemController");
                // Implementation for removing racer would go here
                configChanged = true;
                break;
                
            default:
                // Unknown command, ignore
                break;
        }
        
        // Update the display if configuration changed
        if (configChanged) {
            displayManager.showConfig();
        }
        return;
    }
}

// Event handlers for observer pattern

void SystemController::onRaceStateChanged(RaceState state) {
    DEBUG_PRINT_METHOD();
    // Update display based on race state changes
    switch (state) {
        case RaceState::Idle:
            // Race is idle, show race menu
            displayManager.setScreen(ScreenType::RaceReady);
            displayManager.showRaceReady(
                configModule.getRaceMode(),
                configModule.getNumLaps(),
                configModule.getNumLanes(),
                lightsModule.getCountdownInterval()
            );
            break;
            
        case RaceState::Countdown:
            // Race is in countdown, handled by LightsModule
            break;
            
        case RaceState::Starting:
            // Race is about to start
            displayManager.setScreen(ScreenType::RaceActive);
            displayManager.showMessage("Race starting...");
            break;
            
        case RaceState::Active:
            // Race is active
            displayManager.setScreen(ScreenType::RaceActive);
            {
                // Get the current race mode from the race module
                RaceMode currentRaceMode = raceModule.getRaceMode();
                displayManager.showRaceActive(currentRaceMode);
                displayManager.raceLog(displayManager.formatRaceStatus(raceModule, false));
                
                // Update race data display with the latest race lane data
                displayManager.updateRaceData(createRaceDataSnapshot());
            }
            break;
            
        case RaceState::Paused:
            // Race is paused
            displayManager.raceLog(displayManager.formatRaceStatus(raceModule, true));
            break;
            
        case RaceState::Finished:
            // Race is finished - use RaceReady screen since RaceResults doesn't exist
            displayManager.setScreen(ScreenType::RaceReady);
            displayManager.showMessage("Race finished!");
            break;
    }
}



void SystemController::onSecondTick(uint32_t raceTimeMs) {
    // No debug print to avoid flooding the log with tick updates
    // Update race clock display every second
    String formattedTime = formatTimeMMSS(raceTimeMs);
    displayManager.showMessage("Race Time: " + formattedTime);
    
    // Only update race data display if race is active or paused
    if (raceModule.getRaceState() == RaceState::Active || 
        raceModule.getRaceState() == RaceState::Paused) {
        // Update race data display with the latest race lane data
        displayManager.updateRaceData(createRaceDataSnapshot());
    }
}

void SystemController::onLapRegistered(int lane, uint32_t lapTimeMs) {
    DEBUG_PRINT_METHOD();
    // Update lap display when a lap is registered
    String formattedTime = formatTimeMMSSmmm(lapTimeMs);
    displayManager.showMessage("Lane " + String(lane) + " Lap: " + formattedTime);
    
    // Update race status display
    displayManager.raceLog(displayManager.formatRaceStatus(raceModule, raceModule.isRacePaused()));
    
    // Update race data display with the latest race lane data
    displayManager.updateRaceData(createRaceDataSnapshot());
}

String SystemController::formatTimeMMSS(uint32_t timeMs) {
    // No debug print for utility functions to avoid performance impact
    uint32_t totalSeconds = timeMs / 1000;
    uint32_t minutes = totalSeconds / 60;
    uint32_t seconds = totalSeconds % 60;
    
    char buffer[10]; // Increased buffer size for safety: MM:SS\0
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu", (unsigned long)minutes, (unsigned long)seconds);
    return String(buffer);
}

String SystemController::formatTimeMMSSmmm(uint32_t timeMs) {
    // No debug print for utility functions to avoid performance impact
    uint32_t totalSeconds = timeMs / 1000;
    uint32_t minutes = totalSeconds / 60;
    uint32_t seconds = totalSeconds % 60;
    uint32_t millis = timeMs % 1000;
    
    char buffer[16]; // Increased buffer size for safety: MM:SS:mmm\0
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%03lu", (unsigned long)minutes, (unsigned long)seconds, (unsigned long)millis);
    return String(buffer);
}

std::vector<RaceLaneData> SystemController::createRaceDataSnapshot() {
    DEBUG_PRINT_METHOD();
    // Get the number of lanes from the race module
    int numLanes = raceModule.getNumLanes();
    std::vector<RaceLaneData> laneDataSnapshot;
    
    // Collect data for each lane
    for (int i = 1; i <= numLanes; i++) {
        // Get the lane data from the race module
        RaceLaneData laneData = raceModule.getLaneData(i);
        
        // Only include enabled lanes
        if (laneData.enabled) {
            // Add the lane data to the snapshot
            laneDataSnapshot.push_back(laneData);
        }
    }
    
    // Log that we created a race data snapshot
    displayManager.debug("Created race data snapshot with " + String(laneDataSnapshot.size()) + " lanes", "SystemController");
    
    return laneDataSnapshot;
}
