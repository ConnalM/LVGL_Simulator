#include "KeyboardInput.h"
#include "DisplayModule/DisplayManager.h"
#include <Arduino.h> // For Serial
#include <cstring>  // For memset

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        static bool firstCall = true; \
        unsigned long now = millis(); \
        if (firstCall || (now - lastDebugPrint > DEBUG_THROTTLE_MS)) { \
            DisplayManager::getInstance().debug(String("[KeyboardInput] ") + __FUNCTION__, "KeyboardInput"); \
            lastDebugPrint = now; \
            firstCall = false; \
        } \
    } while(0)

// Implementation of helper functions
void KeyboardInput::resetInputState() {
    DEBUG_PRINT_METHOD();
    kbdState = KeyboardState::Idle;
    memset(inputBuffer, 0, INPUT_BUFFER_SIZE);
    inputPos = 0;
}

bool KeyboardInput::addToInputBuffer(char c) {
    DEBUG_PRINT_METHOD();
    if (inputPos < INPUT_BUFFER_SIZE - 1) {
        inputBuffer[inputPos++] = c;
        inputBuffer[inputPos] = '\0';
        return true;
    }
    return false;
}

int KeyboardInput::getInputValue() const {
    DEBUG_PRINT_METHOD();
    return atoi(inputBuffer);
}

bool KeyboardInput::handleNumericInput(char c, InputEvent& event) {
    DEBUG_PRINT_METHOD();
    if (!isdigit(c)) {
        return false;
    }

    // Handle based on current state
    switch (kbdState) {
        case KeyboardState::WaitLaneNumber:
        case KeyboardState::WaitLanesNumber:
        case KeyboardState::WaitLapNumber:
        case KeyboardState::WaitCountdownInterval:
        case KeyboardState::WaitRaceTime:
            if (addToInputBuffer(c)) {
                Serial.print(c); // Echo the digit
                return false;
            }
            break;
            
        default: {
            // Default behavior for numeric keys when not in a special state
            ScreenType currentScreen = DisplayManager::getInstance().getCurrentScreen();
            
            if (currentScreen == ScreenType::Main && c >= '1' && c <= '3') {
                // In Main Menu: keys 1-3 are for menu navigation
                int selection = c - '0';
                if (selection == 1) {
                    event.command = InputCommand::StartCountdown;  // Changed from StartRace to StartCountdown
                    event.sourceId = 1;
                } else if (selection == 2) {
                    event.command = InputCommand::EnterConfig;
                    event.sourceId = 2;
                } else if (selection == 3) {
                    event.command = InputCommand::EnterConfig; // For Stats
                    event.sourceId = 3;
                }
                event.target = getDefaultTargetForCommand(event.command);
                return true;
            } else if (c >= '1' && c <= '8') {
                // In any other screen, treat as AddLap for lanes 1-8
                event.command = InputCommand::AddLap;
                event.target = InputTarget::Race;
                event.sourceId = 20000;
                event.value = c - '0';
                return true;
            }
            break;
        }
    }
    return false;
}

bool KeyboardInput::handleEnterKey(InputEvent& event) {
    DEBUG_PRINT_METHOD();
    if (inputPos == 0) {
        return false; // No input to process
    }

    int value = getInputValue();
    bool success = false;

    switch (kbdState) {
        case KeyboardState::WaitLaneNumber: {
            if (value >= 1 && value <= 8) {
                event.command = InputCommand::EnableLane;
                event.target = getDefaultTargetForCommand(event.command);
                event.value = value;
                success = true;
            }
            break;
        }
        
        case KeyboardState::WaitLanesNumber: {
            if (value >= 1 && value <= 8) {
                event.command = InputCommand::SetNumLanes;
                event.target = getDefaultTargetForCommand(event.command);
                event.value = value;
                success = true;
            }
            break;
        }
        
        case KeyboardState::WaitLapNumber: {
            if (value > 0 && value <= 999) {
                event.command = InputCommand::SetNumLaps;
                event.target = getDefaultTargetForCommand(event.command);
                event.value = value;
                success = true;
            }
            break;
        }
        
        case KeyboardState::WaitRaceTime: {
            // Parse MM:SS format or seconds only
            char* colonPos = strchr(inputBuffer, ':');
            int seconds = 0;
            
            if (colonPos) {
                *colonPos = '\0';
                int minutes = atoi(inputBuffer);
                seconds = minutes * 60 + atoi(colonPos + 1);
            } else {
                seconds = value;
            }
            
            if (seconds > 0 && seconds <= 3600) { // Max 1 hour
                event.command = InputCommand::SetRaceTime;
                event.target = getDefaultTargetForCommand(event.command);
                event.value = seconds;
                success = true;
            }
            break;
        }
        
        case KeyboardState::WaitModeNumber: {
            if (value >= 1 && value <= 4) {
                event.command = InputCommand::ChangeMode;
                event.target = getDefaultTargetForCommand(event.command);
                event.value = value;
                success = true;
            }
            break;
        }
        
        case KeyboardState::WaitCountdownInterval: {
            if (value > 0 && value <= 999) {
                event.command = InputCommand::SetCountdownInterval;
                event.target = getDefaultTargetForCommand(event.command);
                event.value = value;
                success = true;
            }
            break;
        }
        
        default:
            break;
    }
    
    resetInputState();
    return success;
}

// Helper function to print keyboard help message
static void printHelpMessage() {
    // No debug print here to avoid recursion
    DisplayManager& display = DisplayManager::getInstance();
    
    display.info("", "KeyboardInput");
    display.info("=== ESP32 Lap Counter HELP ===", "KeyboardInput");
    display.info("CONFIG MODE COMMANDS:", "KeyboardInput");
    display.info("  n       Set laps (prompt)", "KeyboardInput");
    display.info("  l       Set lanes (prompt)", "KeyboardInput");
    display.info("  m       Set mode (prompt)", "KeyboardInput");
    display.info("  t       Set race time (prompt, mm:ss)", "KeyboardInput");
    display.info("  f       Toggle reaction time", "KeyboardInput");
    display.info("  e       Enable lane (prompt for lane number)", "KeyboardInput");
    display.info("  d       Disable lane (prompt for lane number)", "KeyboardInput");
    display.info("  a       Add racer", "KeyboardInput");
    display.info("  z       Remove racer", "KeyboardInput");
    display.info("  h/?     Show this help", "KeyboardInput");
    display.info("  q/x     Return to previous menu", "KeyboardInput");
    display.info("RACING MODE COMMANDS:", "KeyboardInput");
    display.info("  s       Start race", "KeyboardInput");
    display.info("  p       Pause race", "KeyboardInput");
    display.info("  x       Stop race", "KeyboardInput");
    display.info("  r       Reset race", "KeyboardInput");
    display.info("  1-8     Add lap for lane 1-8", "KeyboardInput");
    display.info("  b       Toggle best lap display", "KeyboardInput");
    display.info("", "KeyboardInput");
}

// State is now part of the KeyboardInput class

bool KeyboardInput::initialize() {
    DEBUG_PRINT_METHOD();
    // Check if already initialized
    if (_initialized) {
        DisplayManager::getInstance().info("Already initialized", "KeyboardInput");
        return true;
    }
    
    DisplayManager::getInstance().info("Initializing...", "KeyboardInput");
    
    // Reset state
    kbdState = KeyboardState::Idle;
    pendingCommand = 0;
    
    // No special initialization required for this module
    // but we follow the standard pattern for consistency
    
    _initialized = true;
    DisplayManager::getInstance().info("Initialized successfully", "KeyboardInput");
    return true;
}

bool KeyboardInput::poll(InputEvent& event) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return false;
    }
    
    if (!Serial.available()) return false;
    char c = Serial.read();
    if (c >= 32 && c <= 126) { // printable ASCII
        Serial.print(c);
        // Add debug information for input received
        DisplayManager::getInstance().debug(String("Serial input received: ") + String(c), "KeyboardInput");
    }
    c = tolower(c);
    
    // Get current screen for command handling
    ScreenType currentScreen = DisplayManager::getInstance().getCurrentScreen();
    
    // Log the current screen and input character for debugging
    DisplayManager::getInstance().debug("Current screen: " + String((int)currentScreen) + ", char received: '" + String(c) + "'", "KeyboardInput");

    switch (kbdState) {
        case KeyboardState::Idle:
            switch (c) {
                case 's': 
                    event.command = InputCommand::StartCountdown;  // Changed from StartRace to StartCountdown
                    event.target = getDefaultTargetForCommand(event.command);
                    return true;
                case 'p': 
                    event.command = InputCommand::PauseRace; 
                    event.target = getDefaultTargetForCommand(event.command);
                    return true;
                case 'x': 
                    event.command = InputCommand::StopRace; 
                    event.target = getDefaultTargetForCommand(event.command);
                    return true;
                case 'r': 
                    event.command = InputCommand::ResetRace; 
                    event.target = getDefaultTargetForCommand(event.command);
                    return true;
                case 'm':
                    // Check if we're in the CONFIG MENU before processing this command
                    if (currentScreen == ScreenType::Config) {
                        // ChangeMode command
                        event.command = InputCommand::ChangeMode;
                        event.target = getDefaultTargetForCommand(event.command);
                        DisplayManager::getInstance().debug("ChangeMode command received", "KeyboardInput");
                        DisplayManager::getInstance().info("", "KeyboardInput");
                        DisplayManager::getInstance().info("Select race mode:", "KeyboardInput");
                        DisplayManager::getInstance().info("  1 - LAPS: Multiple laps around a track", "KeyboardInput");
                        DisplayManager::getInstance().info("  2 - TIMER: Simple timing mode", "KeyboardInput");
                        DisplayManager::getInstance().info("  3 - DRAG: Drag race mode", "KeyboardInput");
                        DisplayManager::getInstance().info("  4 - RALLY: Rally with checkpoints", "KeyboardInput");
                        DisplayManager::getInstance().info("Enter mode number (1-4):", "KeyboardInput");
                        kbdState = KeyboardState::WaitModeNumber;
                        return false;
                    } else {
                        // Not in CONFIG MENU, handle as normal 'm' command
                        // For other screens, we'll just pass it through
                        DisplayManager::getInstance().debug("'m' pressed but not in CONFIG MENU", "KeyboardInput");
                        return false;
                    }
                case 'n':
                    // Check if we're in the CONFIG MENU before processing this command
                    if (currentScreen == ScreenType::Config) {
                        // SetNumLaps command
                        event.command = InputCommand::SetNumLaps;
                        event.target = getDefaultTargetForCommand(event.command);
                        DisplayManager::getInstance().debug("SetNumLaps command received in CONFIG MENU", "KeyboardInput");
                        DisplayManager::getInstance().info("", "KeyboardInput");
                        DisplayManager::getInstance().info("How many laps? (Enter a number up to 999):", "KeyboardInput");
                        kbdState = KeyboardState::WaitLapNumber;
                        return false;
                    } else {
                        // Not in CONFIG MENU, ignore the command
                        DisplayManager::getInstance().debug("'n' pressed but not in CONFIG MENU, current screen: " + String((int)currentScreen), "KeyboardInput");
                        return false;
                    }
                case 't':
                    // SetRaceTime command
                    event.command = InputCommand::SetRaceTime;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("SetRaceTime command received", "KeyboardInput");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Enter race time (mm:ss):", "KeyboardInput");
                    kbdState = KeyboardState::WaitRaceTime;
                    return false;
                case 'l':
                    // SetNumLanes command
                    event.command = InputCommand::SetNumLanes;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("SetNumLanes command received", "KeyboardInput");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("How many lanes? (Enter a number 1-8):", "KeyboardInput");
                    kbdState = KeyboardState::WaitLanesNumber;
                    return false;
                case 'e':
                    // EnableLane command
                    event.command = InputCommand::EnableLane;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("EnableLane command received", "KeyboardInput");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Enable which lane? (Enter lane number 1-8):", "KeyboardInput");
                    kbdState = KeyboardState::WaitLaneNumber;
                    return false;
                case 'd':
                    // DisableLane command
                    event.command = InputCommand::DisableLane;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("DisableLane command received", "KeyboardInput");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Disable which lane? (Enter lane number 1-8):", "KeyboardInput");
                    kbdState = KeyboardState::WaitLaneNumber;
                    return false;
                case 'a':
                    // AddRacer command
                    event.command = InputCommand::AddRacer;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("AddRacer command received", "KeyboardInput");
                    return true;
                case 'z':
                    // RemoveRacer command
                    event.command = InputCommand::RemoveRacer;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("RemoveRacer command received", "KeyboardInput");
                    return true;
                case 'f':
                    // ToggleReactionTime command
                    event.command = InputCommand::ToggleReactionTime;
                    event.target = getDefaultTargetForCommand(event.command);
                    DisplayManager::getInstance().debug("ToggleReactionTime command received", "KeyboardInput");
                    return true;
                // 'q' is handled in a separate case below to avoid duplicate case labels
                case 'b':
                    event.command = InputCommand::ToggleBestLap;
                    event.target = getDefaultTargetForCommand(event.command);
                    return true;
                // 'c' command removed as per user request
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                {
                    // Get current screen to determine behavior
                    ScreenType currentScreen = DisplayManager::getInstance().getCurrentScreen();
                    
                    // Debug the current screen and state
                    DisplayManager::getInstance().debug("Current screen: " + String((int)currentScreen) + ", state: " + String((int)kbdState), "KeyboardInput");
                    
                    // Check if we're in a state that expects a lane number
                    if (kbdState == KeyboardState::WaitLaneNumber && pendingCommand == 'd') {
                        
                        int lane = c - '0';
                        if (lane >= 1 && lane <= 8) {
                            // Set the appropriate command based on state
                            event.command = (kbdState == KeyboardState::WaitLaneNumber) ? 
                                InputCommand::EnableLane : InputCommand::DisableLane;
                                
                            event.target = getDefaultTargetForCommand(event.command);
                            event.value = lane;
                            event.sourceId = 20000; // Keyboard source ID
                            
                            // Reset state
                            kbdState = KeyboardState::Idle;
                            
                            // Log the action
                            String action = (kbdState == KeyboardState::WaitLaneNumber) ? "Enabling" : "Disabling";
                            DisplayManager::getInstance().debug(action + " lane: " + String(lane), "KeyboardInput");
                            
                            return true;
                        } else {
                            DisplayManager::getInstance().info("Error: Invalid lane number! Enter 1-8.", "KeyboardInput");
                            return false;
                        }
                    }
                    // Handle numeric keys differently based on the current screen
                    else if (currentScreen == ScreenType::Main && c >= '1' && c <= '3') {
                        // In Main Menu: keys 1-3 are for menu navigation
                        int selection = c - '0';
                        DisplayManager::getInstance().debug("Main menu selection: " + String(selection), "KeyboardInput");
                        
                        if (selection == 1) {
                            // Use EnterRaceReady command to navigate to Race Menu
                            event.command = InputCommand::EnterRaceReady;
                            event.value = 0; // No specific value needed
                            event.sourceId = 1; // Identify as coming from key '1'
                        } else if (selection == 2) {
                            event.command = InputCommand::EnterConfig;
                            event.sourceId = 2; // Identify as coming from key '2'
                        } else if (selection == 3) {
                            event.command = InputCommand::EnterStats; // Now using proper EnterStats command
                            event.sourceId = 3; // Identify as coming from key '3' (for Stats)
                        }
                        event.target = getDefaultTargetForCommand(event.command);
                        return true;
                    } 
                    else if (currentScreen == ScreenType::Config) {
                        // In Config Menu: all numeric keys are for AddLap by default
                        // unless we're in a specific state (handled above)
                        DisplayManager::getInstance().debug("Config menu - numeric key: " + String(c), "KeyboardInput");
                        event.command = InputCommand::AddLap;
                        event.target = getDefaultTargetForCommand(event.command);
                        event.sourceId = 20000; // Keyboard source ID
                        event.value = c - '0';  // Convert ASCII to number
                        Serial.print(" Lane ");
                        Serial.println(event.value);
                        return true;
                    }
                    else if (currentScreen == ScreenType::RaceActive) {
                        // In Race Menus and Screens: all numeric keys are for AddLap
                        DisplayManager::getInstance().debug("Race screen - numeric key: " + String(c), "KeyboardInput");
                        event.command = InputCommand::AddLap;
                        event.target = getDefaultTargetForCommand(event.command);
                        event.sourceId = 20000; // Keyboard source ID
                        event.value = c - '0';  // Convert ASCII to number
                        Serial.print(" Lane ");
                        Serial.println(event.value);
                        return true;
                    }
                    else {
                        // For any other screen, default to AddLap for all numeric keys
                        DisplayManager::getInstance().debug("Other screen - numeric key: " + String(c), "KeyboardInput");
                        event.command = InputCommand::AddLap;
                        event.target = getDefaultTargetForCommand(event.command);
                        event.sourceId = 20000; // Keyboard source ID
                        event.value = c - '0';  // Convert ASCII to number
                        Serial.print(" Lane ");
                        Serial.println(event.value);
                        return true;
                    }
                }
                case 'h':
                case '?':
                    printHelpMessage();
                    return false;
                case 'q':
                    // Return to previous menu/screen
                    DisplayManager::getInstance().debug("Return to previous menu command received, current screen: " + String((int)currentScreen), "KeyboardInput");
                    event.command = InputCommand::ReturnToPrevious;
                    
                    // Set the appropriate target based on the current screen
                    if (currentScreen == ScreenType::Config) {
                        DisplayManager::getInstance().debug("Returning from CONFIG MENU to Main Menu", "KeyboardInput");
                        event.target = InputTarget::Config;
                    } else {
                        // Default target for other screens
                        event.target = getDefaultTargetForCommand(event.command);
                    }
                    return true;
                // case 's': - Removed duplicate case (already defined at line 459)
                    // This was a duplicate case for StartRace command
                    
                case 'i': 
                    // Start countdown interval setting
                    DisplayManager::getInstance().info("\nEnter countdown interval in seconds (1-999): ", "KeyboardInput");
                    kbdState = KeyboardState::WaitCountdownInterval;
                    return false;
                default:
                    return false;
            }
        case KeyboardState::WaitModeNumber: {
            if (isdigit(c)) {
                int mode = c - '0';
                if (mode >= 1 && mode <= 4) {
                    event.command = InputCommand::ChangeMode;
                    event.target = getDefaultTargetForCommand(event.command);
                    event.value = mode;
                    kbdState = KeyboardState::Idle;
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Mode set to: ", "KeyboardInput");
                    switch (mode) {
                        case 1: 
                            DisplayManager::getInstance().info("LAPS", "KeyboardInput"); 
                            break;
                        case 2: 
                            DisplayManager::getInstance().info("TIMER", "KeyboardInput"); 
                            break;
                        case 3: 
                            DisplayManager::getInstance().info("DRAG", "KeyboardInput"); 
                            break;
                        case 4: 
                            DisplayManager::getInstance().info("RALLY", "KeyboardInput"); 
                            break;
                    }
                    return true;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid mode! Enter a number between 1 and 4.", "KeyboardInput");
                    kbdState = KeyboardState::Idle;
                    return false;
                }
            }
            kbdState = KeyboardState::Idle;
            return false;
        }
        case KeyboardState::WaitLapNumber: {
            if (isdigit(c)) {
                Serial.print("[DEBUG] KeyboardInput - Digit received in ORIGINAL handler: ");
                Serial.println(c);
                
                if (digitCount < 3) {
                    digitBuffer[digitCount++] = c;
                    Serial.print("[DEBUG] KeyboardInput - Buffer now: ");
                    Serial.println(digitBuffer);
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Too many digits! Enter a number up to 999.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                }
                return false;
            } else if (c == '\r' || c == '\n') {
                Serial.println("[DEBUG] KeyboardInput - Enter key pressed in ORIGINAL handler");
                
                if (digitCount == 0) {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: No number entered!", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    return false;
                }
                
                digitBuffer[digitCount] = 0;
                Serial.print("[DEBUG] KeyboardInput - Processing laps number in ORIGINAL handler: ");
                Serial.println(digitBuffer);
                
                int value = atoi(digitBuffer);
                if (value > 0 && value <= 999) {
                    Serial.println("[DEBUG] KeyboardInput - Valid laps number in ORIGINAL handler, creating SetNumLaps event");
                    
                    event.command = InputCommand::SetNumLaps;
                    event.target = getDefaultTargetForCommand(event.command);
                    event.value = value;
                    
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    kbdState = KeyboardState::Idle;
                    
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Laps set to: ", "KeyboardInput");
                    DisplayManager::getInstance().info(String(value), "KeyboardInput");
                    
                    Serial.println("[DEBUG] KeyboardInput - Returning true with SetNumLaps event from ORIGINAL handler");
                    return true;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid number of laps! Enter a number between 1 and 999.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    return false;
                }
            } else {
                // Ignore invalid chars
                return false;
            }
        }
        case KeyboardState::WaitRaceTime: {
            // CRITICAL DEBUG - Direct console output for WaitRaceTime handler
            Serial.println("[DEBUG] KeyboardInput - In WaitRaceTime handler");
            Serial.print("[DEBUG] KeyboardInput - Current char: '");
            Serial.print(c);
            Serial.println("'");
            
            static char timeBuffer[6] = {0};
            static uint8_t timeDigitCount = 0;
            
            if (isdigit(c) || c == ':') {
                Serial.print("[DEBUG] KeyboardInput - Valid character received: ");
                Serial.println(c);
                
                if (timeDigitCount < 5) {
                    timeBuffer[timeDigitCount++] = c;
                    Serial.print("[DEBUG] KeyboardInput - Time buffer now: ");
                    Serial.println(timeBuffer);
                } else {
                    Serial.println("[DEBUG] KeyboardInput - Too many characters entered");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Too many characters! Format: mm:ss", "KeyboardInput");
                    timeDigitCount = 0;
                    timeBuffer[0] = 0;
                }
                return false;
            } else if (c == '\r' || c == '\n') {
                Serial.println("[DEBUG] KeyboardInput - Enter key pressed in WaitRaceTime handler");
                
                if (timeDigitCount == 0) {
                    Serial.println("[DEBUG] KeyboardInput - No time entered");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: No time entered!", "KeyboardInput");
                    timeDigitCount = 0;
                    timeBuffer[0] = 0;
                    return false;
                }
                
                timeBuffer[timeDigitCount] = 0;
                Serial.print("[DEBUG] KeyboardInput - Processing time input: ");
                Serial.println(timeBuffer);
                
                // Parse mm:ss format
                int minutes = 0;
                int seconds = 0;
                char* colonPos = strchr(timeBuffer, ':');
                
                if (colonPos) {
                    *colonPos = 0; // Split the string
                    minutes = atoi(timeBuffer);
                    seconds = atoi(colonPos + 1);
                    Serial.printf("[DEBUG] KeyboardInput - Parsed as MM:SS format: %d minutes, %d seconds\n", minutes, seconds);
                } else {
                    // No colon, assume all seconds
                    seconds = atoi(timeBuffer);
                    minutes = seconds / 60;
                    seconds = seconds % 60;
                    Serial.printf("[DEBUG] KeyboardInput - Parsed as seconds only: %d total seconds = %d minutes, %d seconds\n", atoi(timeBuffer), minutes, seconds);
                }
                
                int totalSeconds = minutes * 60 + seconds;
                Serial.printf("[DEBUG] KeyboardInput - Total seconds calculated: %d\n", totalSeconds);
                
                if (totalSeconds > 0 && totalSeconds <= 3600) { // Max 1 hour
                    Serial.println("[DEBUG] KeyboardInput - Valid race time, creating SetRaceTime event");
                    
                    event.command = InputCommand::SetRaceTime;
                    event.target = getDefaultTargetForCommand(event.command);
                    event.value = totalSeconds;
                    
                    timeDigitCount = 0;
                    timeBuffer[0] = 0;
                    kbdState = KeyboardState::Idle;
                    
                    // Format seconds with leading zero if needed
                    String secondsStr = seconds < 10 ? "0" + String(seconds) : String(seconds);
                    
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Race time set to: ", "KeyboardInput");
                    DisplayManager::getInstance().info(String(minutes) + ":" + secondsStr, "KeyboardInput");
                    
                    Serial.printf("[DEBUG] KeyboardInput - Race time set to: %d:%s\n", minutes, secondsStr.c_str());
                    Serial.println("[DEBUG] KeyboardInput - Returning true with SetRaceTime event");
                    return true;
                } else {
                    Serial.println("[DEBUG] KeyboardInput - Invalid race time");
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid race time! Enter a time between 1 second and 1 hour.", "KeyboardInput");
                    timeDigitCount = 0;
                    timeBuffer[0] = 0;
                    return false;
                }
            } else {
                // Ignore invalid chars
                return false;
            }
        }
        case KeyboardState::WaitLanesNumber: {
            if (isdigit(c)) {
                int lanes = c - '0';
                if (lanes >= 1 && lanes <= 8) {
                    event.command = InputCommand::SetNumLanes;
                    event.target = getDefaultTargetForCommand(event.command);
                    event.value = lanes;
                    kbdState = KeyboardState::Idle;
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Lanes set to: ", "KeyboardInput");
                    DisplayManager::getInstance().info(String(lanes), "KeyboardInput");
                    return true;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid number of lanes! Enter a number between 1 and 8.", "KeyboardInput");
                }
            }
            return false;
        }
        case KeyboardState::WaitLaneNumber: {
            static char digitBuffer[2] = {0};
            static uint8_t digitCount = 0;
            
            // Handle digit input
            if (isdigit(c)) {
                if (digitCount < 1) {
                    digitBuffer[digitCount++] = c;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid lane number! Enter a number between 1 and 8.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                }
                return false;
            }
            
            // Handle Enter/Return key
            if (c == '\r' || c == '\n') {
                if (digitCount == 0 || digitCount > 1) {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid lane number! Enter a number between 1 and 8.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    return false;
                }
                
                digitBuffer[digitCount] = 0;
                int value = atoi(digitBuffer);
                
                if (value >= 1 && value <= 8) {
                    // Determine the command based on pendingCommand
                    if (pendingCommand == 'e') {
                        event.command = InputCommand::EnableLane;
                    } else if (pendingCommand == 'd') {
                        event.command = InputCommand::DisableLane;
                    } else {
                        event.command = InputCommand::SetNumLanes;
                    }
                    
                    event.target = getDefaultTargetForCommand(event.command);
                    event.value = value;
                    
                    // Log the action
                    const char* action = "";
                    if (pendingCommand == 'e') {
                        action = "Enabled lane: ";
                    } else if (pendingCommand == 'd') {
                        action = "Disabled lane: ";
                    } else {
                        action = "Set number of lanes to: ";
                    }
                    
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info(action, "KeyboardInput");
                    DisplayManager::getInstance().info(String(value), "KeyboardInput");
                    
                    // Reset state
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    kbdState = KeyboardState::Idle;
                    pendingCommand = 0;
                    
                    return true;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid lane number! Enter a number between 1 and 8.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    pendingCommand = 0;
                    return false;
                }
            }
            
            // Ignore any other characters
            return false;
        }
        case KeyboardState::WaitCountdownInterval: {
            static char digitBuffer[4] = {0};
            static uint8_t digitCount = 0;
            if (isdigit(c)) {
                if (digitCount < 3) {
                    digitBuffer[digitCount++] = c;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Too many digits! Enter a number up to 999.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                }
                return false;
            } else if (c == '\r' || c == '\n') {
                if (digitCount == 0) {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: No number entered!", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    return false;
                }
                digitBuffer[digitCount] = 0;
                Serial.println(digitBuffer); // Echo user input on its own line
                int value = atoi(digitBuffer);
                if (value > 0 && value <= 999) {
                    event.command = InputCommand::SetCountdownInterval;
                    event.target = getDefaultTargetForCommand(event.command);
                    event.value = value;
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    kbdState = KeyboardState::Idle;
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Countdown interval set to: ", "KeyboardInput");
                    DisplayManager::getInstance().info(String(value), "KeyboardInput");
                    return true;
                } else {
                    DisplayManager::getInstance().info("", "KeyboardInput");
                    DisplayManager::getInstance().info("Error: Invalid countdown interval! Enter a number between 1 and 999.", "KeyboardInput");
                    digitCount = 0;
                    digitBuffer[0] = 0;
                    return false;
                }
            } else {
                // Ignore invalid chars
                return false;
            }
        }
    }
    // Default fallback if no case matches
    kbdState = KeyboardState::Idle;
    return false;
} // End of poll function
