#include "DisplayManager.h"
#include "DisplayFactory.h"
#include "ESP32_8048S070_Lvgl_DisplayDriver.h"
#include <Ticker.h>

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        unsigned long now = millis(); \
        if (now - lastDebugPrint > DEBUG_THROTTLE_MS) { \
            Serial.printf("[DisplayManager] %s\n", __FUNCTION__); \
            lastDebugPrint = now; \
        } \
    } while(0)

// Static instance for singleton pattern
DisplayManager* DisplayManager::_instance = nullptr;

DisplayManager& DisplayManager::getInstance() {
    DEBUG_PRINT_METHOD();
    if (_instance == nullptr) {
        _instance = new DisplayManager();
    }
    return *_instance;
}

DisplayManager::DisplayManager()
    : _activeDisplayCount(0)
    , _currentScreen(ScreenType::Main)
    , _initialized(false)
    , _countdownDisplay("") {
    DEBUG_PRINT_METHOD();
    // Initialize array to nullptrs
    for (int i = 0; i < 3; i++) {
        _activeDisplays[i] = nullptr;
    }
}

bool DisplayManager::initialize(DisplayType* displayTypes, int count) {
    DEBUG_PRINT_METHOD();
    // Skip if already initialized
    if (_initialized) {
        Serial.println(F("DisplayManager: Already initialized"));
        return true;
    }
    
    Serial.println(F("DisplayManager: Initializing displays..."));
    
    // Initialize each display type
    bool success = true;
    for (int i = 0; i < count; i++) {
        Serial.printf("DisplayManager: Creating display type %d\n", (int)displayTypes[i]);
        IBaseDisplay* display = DisplayFactory::getInstance().getDisplay(displayTypes[i]);
        if (display == nullptr) {
            Serial.printf("DisplayManager: Failed to get display type %d\n", (int)displayTypes[i]);
            success = false;
            continue;
        }
        
        // Initialize the display
        Serial.printf("DisplayManager: Initializing display type %d\n", (int)displayTypes[i]);
        bool initResult = display->initialize();
        if (!initResult) {
            Serial.printf("DisplayManager: Failed to initialize display type %d\n", (int)displayTypes[i]);
            success = false;
            continue;
        }
        
        // Add to active displays
        _activeDisplays[_activeDisplayCount] = display;
        _activeDisplayTypes[_activeDisplayCount] = displayTypes[i];
        _activeDisplayCount++;
        Serial.printf("DisplayManager: Successfully initialized display type %d\n", (int)displayTypes[i]);
    }
    
    if (!success) {
        Serial.println(F("DisplayManager: Failed to initialize some displays"));
    } else {
        Serial.println(F("DisplayManager: All displays initialized successfully"));
    }
    
    _initialized = success;
    Serial.println(F("DisplayManager: Initialization complete"));
    return success;
}

void DisplayManager::update() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        debug("DisplayManager not initialized, skipping update");
        return;
    }
    
    debug(String("Updating displays (count: ") + String(_activeDisplayCount) + ")");
    
    // Update all active displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            // Get display type safely
            DisplayType type;
            {
                // Use a scope to ensure type is only accessed if the pointer is valid
                IBaseDisplay* display = _activeDisplays[i];
                if (display) {
                    type = display->getDisplayType();
                    debug(String("Updating display ") + String(i) + 
                          " (type: " + String(static_cast<int>(type)) + ")");
                    display->update();
                } else {
                    debug(String("Display at index ") + String(i) + " became null during update");
                }
            }
        } else {
            debug(String("Skipping null display at index ") + String(i));
        }
    }
}

void DisplayManager::setScreen(ScreenType screen) {
    DEBUG_PRINT_METHOD();
    debug(String("DisplayManager::setScreen(") + String(static_cast<int>(screen)) + ")");
    
    // Skip if not initialized
    if (!_initialized) {
        debug("DisplayManager not initialized, cannot set screen");
        return;
    }
    
    // Only update if the screen is actually changing
    if (_currentScreen == screen) {
        debug(String("Screen already set to ") + String(static_cast<int>(screen)));
        return;
    }
    
    ScreenType oldScreen = _currentScreen;
    _currentScreen = screen; // Update current screen first to prevent recursion
    
    debug(String("Changing screen from ") + String(static_cast<int>(oldScreen)) + 
          " to " + String(static_cast<int>(screen)));
    
    // Force an immediate screen update to ensure any pending updates are processed
    debug("Forcing display update before screen change");
    update();
    
    // Update all displays to the new screen
    debug(String("Updating ") + String(_activeDisplayCount) + " displays");
    
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] == nullptr) {
            debug(String("Skipping null display at index ") + String(i));
            continue;
        }
        
        DisplayType displayType = _activeDisplays[i]->getDisplayType();
        debug(String("Updating display ") + String(i) + 
              " (type: " + String(static_cast<int>(displayType)) + ")");
        
        // Handle different display types
        switch (displayType) {
            case DisplayType::Serial: {
                // Handle Serial display
                switch (screen) {
                    case ScreenType::Main:
                        if (displayType == DisplayType::LCD) {
                            // For LCD displays, cast to IGraphicalDisplay and call drawMain()
                            IGraphicalDisplay* graphicalDisplay = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                            if (graphicalDisplay != nullptr) {
                                debug("Calling drawMain() on LCD display");
                                graphicalDisplay->drawMain();
                            } else {
                                error("Failed to cast to IGraphicalDisplay", "DisplayManager::setScreen");
                            }
                        } else {
                            // For non-LCD displays (like Serial), show a message
                            debug("Showing main menu on Serial display");
                            showMessage("=== MAIN MENU ===\n1. Race\n2. Config\n3. Stats");
                        }
                        break;
                    case ScreenType::Pause:
                        debug("Showing pause message on Serial display");
                        showMessage("Race Paused - Use RESUME or STOP buttons");
                        break;
                    case ScreenType::Stop:
                        debug("Showing stop message on Serial display");
                        showMessage("Race Stopped - Press NEW RACE to start again");
                        break;
                    case ScreenType::RaceReady:
                        debug("Showing race ready message on Serial display");
                        showMessage("Race Ready - Press START to begin");
                        break;
                    default:
                        debug(String("Unhandled screen type for Serial display: ") + 
                              String(static_cast<int>(screen)));
                        break;
                }
                break;
            }
                
            case DisplayType::LCD: {
                // Handle LCD display using IGraphicalDisplay interface
                IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                debug(String("Updating LCD display to screen type: ") + String(static_cast<int>(screen)));
                
                switch (screen) {
                    case ScreenType::Main:
                        debug("Calling drawMain() on LCD display");
                        lcd->drawMain();
                        break;
                    case ScreenType::RaceReady:
                        debug("Calling drawRaceReady() on LCD display");
                        lcd->drawRaceReady();
                        break;
                    case ScreenType::RaceActive:
                        debug("Calling drawRaceActive() on LCD display with default mode");
                        lcd->drawRaceActive(RaceMode::LAPS);
                        break;
                    case ScreenType::Pause:
                        debug("Calling drawPause() on LCD display");
                        lcd->drawPause();
                        break;
                    case ScreenType::Stop:
                        debug("Calling drawStop() on LCD display");
                        lcd->drawStop();
                        break;
                    default:
                        debug(String("Unhandled screen type for LCD display: ") + 
                              String(static_cast<int>(screen)));
                        break;
                }
                break;
            }
                
            default:
                debug(String("Unknown display type: ") + String(static_cast<int>(displayType)));
                break;
        }
        
        // Force an update after each display is updated
        debug("Forcing display update after screen change");
        _activeDisplays[i]->update();
    }
    
    // Final update to ensure everything is displayed
    debug("Final display update after screen change");
    update();
    
    debug("Screen change complete");
    
    // Force an immediate update of all displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            _activeDisplays[i]->update();
        }
    }
    
    // Final update to ensure everything is displayed
    update();
}

void DisplayManager::showMain() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        Serial.println("DisplayManager::showMain - Not initialized");
        return;
    }
    
    Serial.println("DisplayManager::showMain - Starting");
    
    // Display main menu on all active displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            Serial.print("Processing display type: ");
            Serial.println((int)_activeDisplays[i]->getDisplayType());
            
            // For LCD display, use the graphical interface
            if (_activeDisplays[i]->getDisplayType() == DisplayType::LCD) {
                Serial.println("Found LCD display, using existing display instance");
                
                // Cast the existing display to IGraphicalDisplay
                IGraphicalDisplay* graphicalDisplay = 
                    static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                
                if (graphicalDisplay != nullptr) {
                    Serial.println("Using graphical display, calling drawMain");
                    graphicalDisplay->drawMain();
                    Serial.println("drawMain completed");
                } else {
                    Serial.println("ERROR: Failed to cast display to IGraphicalDisplay");
                }
                continue;
            }
            
            Serial.print("Using text display for type: ");
            Serial.println((int)_activeDisplays[i]->getDisplayType());
            
            // For other display types (like Serial), use the generic approach
            _activeDisplays[i]->clear();
            _activeDisplays[i]->print(F("\n========== MAIN MENU =========="));
            _activeDisplays[i]->print(F("1) Race Menu"));
            _activeDisplays[i]->print(F("2) Config Menu"));
            _activeDisplays[i]->print(F("3) Stats Menu"));
            _activeDisplays[i]->print(F("Enter choice: "), false);
        }
    }
    
    Serial.println("DisplayManager::showMain - Completed");
}

void DisplayManager::showRaceReady(RaceMode raceMode, int numLaps, int numLanes, uint32_t countdownInterval) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        debug("DisplayManager not initialized, cannot show race ready screen");
        return;
    }
    
    debug("Entering showRaceReady");
    
    // Update the current screen type
    _currentScreen = ScreenType::RaceReady;
    
    // Convert race mode to string
    const char* modeStr = "UNKNOWN";
    switch (raceMode) {
        case RaceMode::LAPS: modeStr = "LAPS"; break;
        case RaceMode::TIMER: modeStr = "TIMER"; break;
        case RaceMode::DRAG: modeStr = "DRAG"; break;
        case RaceMode::RALLY: modeStr = "RALLY"; break;
        case RaceMode::PRACTISE: modeStr = "PRACTISE"; break;
    }
    
    // Log the race configuration
    debug(String("Showing RaceReady screen - Mode: ") + modeStr + 
          ", Laps: " + String(numLaps) + 
          ", Lanes: " + String(numLanes) + 
          ", Countdown: " + String(countdownInterval) + "ms");
    
    // Display race ready screen on all active displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        IBaseDisplay* d = _activeDisplays[i];
        if (!d) continue;
        
        switch (d->getDisplayType()) {
            case DisplayType::Serial: {
                // Handle Serial display
                d->clear();
                d->print(F("\n========== RACE MENU =========="));
                d->print(F("Current Race Configuration:"));
                d->print(F("  Mode: %s\n"), modeStr);
                d->printf("  Laps: %d\n", numLaps);
                d->printf("  Lanes: %d\n", numLanes);
                d->printf("  Countdown Interval: %.1f seconds\n", countdownInterval / 1000.0f);
                d->print("");
                d->print(F("Race Status: NOT ACTIVE\n"));
                d->print(F("Race Commands:"));
                d->print(F("S) Start Race"));
                d->print(F("P) Pause Race"));
                d->print(F("R) Resume Race"));
                d->print(F("X) Stop Race"));
                d->print(F("1-8) Add Lap for Lane"));
                d->print(F("I) Set Countdown Interval"));
                d->print(F("E) Exit to Main Menu"));
                d->print(F("Enter command: "), false);
                break;
            }
            case DisplayType::LCD: {
                // Handle LCD display using IGraphicalDisplay interface
                IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(d);
                if (lcd) {
                    // Ensure the screen is created and shown
                    lcd->drawRaceReady();
                    // Force an immediate update
                    lcd->update();
                }
                break;
            }
            case DisplayType::Web:
                // Handle Web display if needed
                break;
        }
    }
}

void DisplayManager::showConfig() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        Serial.println(F("DisplayManager::showConfig - Not initialized"));
        return;
    }
    
    Serial.print(F("DisplayManager::showConfig - Active displays: "));
    Serial.println(_activeDisplayCount);
    
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] == nullptr) {
            Serial.println(F("DisplayManager::showConfig - Found null display, skipping"));
            continue;
        }
        
        DisplayType type = _activeDisplays[i]->getDisplayType();
        Serial.print(F("DisplayManager::showConfig - Processing display "));
        Serial.print(i);
        Serial.print(F(", type: "));
        Serial.println((int)type);
        
        switch (type) {
            case DisplayType::LCD: {
                Serial.println(F("DisplayManager::showConfig - Found LCD display"));
                IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                if (lcd != nullptr) {
                    Serial.println(F("DisplayManager::showConfig - Calling drawConfig"));
                    lcd->drawConfig();
                } else {
                    Serial.println(F("DisplayManager::showConfig - Failed to cast to IGraphicalDisplay"));
                }
                break;
            }
            case DisplayType::Serial: {
                Serial.println(F("DisplayManager::showConfig - Found Serial display"));
                _activeDisplays[i]->clear();
                _activeDisplays[i]->print(F("\n========== CONFIG MENU =========="));
                _activeDisplays[i]->print(F("Available Options:"));
                _activeDisplays[i]->print(F("n) SetNumLaps"));
                _activeDisplays[i]->print(F("l) SetNumLanes"));
                _activeDisplays[i]->print(F("m) ChangeMode"));
                _activeDisplays[i]->print(F("t) SetRaceTime"));
                _activeDisplays[i]->print(F("f) ToggleReactionTime"));
                _activeDisplays[i]->print(F("c) EnterConfig"));
                _activeDisplays[i]->print(F("e) EnableLane"));
                _activeDisplays[i]->print(F("d) DisableLane"));
                _activeDisplays[i]->print(F("a) AddRacer"));
                _activeDisplays[i]->print(F("z) RemoveRacer"));
                _activeDisplays[i]->print(F("q) Return to Main Menu"));
                _activeDisplays[i]->print(F("Enter command: "), false);
                Serial.println(F("DisplayManager::showConfig - Serial menu displayed"));
                break;
            }
            case DisplayType::Web:
                Serial.println(F("DisplayManager::showConfig - Found Web display (not implemented)"));
                // Handle Web display if needed
                break;
            default:
                Serial.print(F("DisplayManager::showConfig - Unknown display type: "));
                Serial.println((int)type);
                break;
        }
    }
}

void DisplayManager::showRaceActive(RaceMode raceMode) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    debug("Showing race active screen", "DisplayManager");
    
    // Update each display
    for (int i = 0; i < _activeDisplayCount; i++) {
        DisplayType type = _activeDisplayTypes[i];
        
        switch (type) {
            case DisplayType::LCD: {
                debug("Found LCD display, calling drawRaceActive", "DisplayManager");
                IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                if (lcd != nullptr) {
                    lcd->drawRaceActive(raceMode);
                } else {
                    error("Failed to cast to IGraphicalDisplay", "DisplayManager");
                }
                break;
            }
            case DisplayType::Serial: {
                debug("Found Serial display", "DisplayManager");
                _activeDisplays[i]->clear();
                _activeDisplays[i]->print(F("\n========== RACE ACTIVE ==========\n"));
                _activeDisplays[i]->print(F("Race is in progress.\n"));
                _activeDisplays[i]->print(F("Commands:\n"));
                _activeDisplays[i]->print(F("p) Pause Race\n"));
                _activeDisplays[i]->print(F("x) Stop Race\n"));
                break;
            }
            case DisplayType::Web:
                debug("Found Web display (not implemented)", "DisplayManager");
                // Handle Web display if needed
                break;
            default:
                warning("Unknown display type: " + String((int)type), "DisplayManager");
                break;
        }
    }
}

void DisplayManager::showStats() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    debug("Showing statistics screen", "DisplayManager");
    
    // Update each display
    for (int i = 0; i < _activeDisplayCount; i++) {
        DisplayType type = _activeDisplayTypes[i];
        
        switch (type) {
            case DisplayType::LCD: {
                debug("Found LCD display, calling drawStats", "DisplayManager");
                IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                if (lcd != nullptr) {
                    lcd->drawStats();
                } else {
                    error("Failed to cast to IGraphicalDisplay", "DisplayManager");
                }
                break;
            }
            case DisplayType::Serial: {
                debug("Found Serial display", "DisplayManager");
                _activeDisplays[i]->clear();
                _activeDisplays[i]->print(F("\n========== STATISTICS ==========\n"));
                _activeDisplays[i]->print(F("Dummy Stats Page\n"));
                _activeDisplays[i]->print(F("Coming Soon\n"));
                break;
            }
            case DisplayType::Web:
                debug("Found Web display (not implemented)", "DisplayManager");
                // Handle Web display if needed
                break;
            default:
                warning("Unknown display type: " + String((int)type), "DisplayManager");
                break;
        }
    }
}

void DisplayManager::updateRaceData(const std::vector<RaceLaneData>& laneData) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        Serial.println("DisplayManager::updateRaceData - Not initialized, skipping update");
        return;
    }
    
    debug("Updating race data display with " + String(laneData.size()) + " lanes", "DisplayManager");
    
    // Debug: Print lane data being received
    for (const auto& lane : laneData) {
        if (lane.enabled) {
            String laneInfo = "Lane " + String(lane.laneId) + 
                           " Lap: " + String(lane.currentLap) + 
                           "/" + String(lane.totalLaps) +
                           " Last: " + String(lane.lastLapTime) + "ms" +
                           " Best: " + String(lane.bestLapTime) + "ms" +
                           " Total: " + String(lane.totalTime) + "ms";
            debug(laneInfo, "DisplayManager");
        }
    }
    
    // Update each display
    for (int i = 0; i < _activeDisplayCount; i++) {
        DisplayType type = _activeDisplayTypes[i];
        
        switch (type) {
            case DisplayType::LCD: {
                debug("Found LCD display, updating race data", "DisplayManager");
                IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
                if (lcd != nullptr) {
                    // Call the updateRaceData method on the LVGL display driver
                    // This will be implemented in ESP32_8048S070_Lvgl_DisplayDriver
                    lcd->updateRaceData(laneData);
                } else {
                    error("Failed to cast to IGraphicalDisplay", "DisplayManager");
                }
                break;
            }
            case DisplayType::Serial: {
                debug("Found Serial display, showing race data summary", "DisplayManager");
                // For Serial display, just show a summary of the race data
                _activeDisplays[i]->print(F("\n--- Race Data Update ---\n"));
                
                for (const auto& lane : laneData) {
                    if (lane.enabled) {
                        String laneInfo = "Lane " + String(lane.laneId) + 
                                         ", Pos: " + String(lane.position) + 
                                         ", Last: " + formatTimeMMSSmmm(lane.lastLapTime) + 
                                         ", Total: " + formatTimeMMSSmmm(lane.totalTime);
                        _activeDisplays[i]->print(laneInfo);
                    }
                }
                break;
            }
            case DisplayType::Web:
                debug("Found Web display (not implemented)", "DisplayManager");
                // Handle Web display if needed
                break;
            default:
                warning("Unknown display type: " + String((int)type), "DisplayManager");
                break;
        }
    }
}

void DisplayManager::showCountdown(int currentStep, bool isComplete) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Format the countdown display
    String countdownText = formatCountdown(currentStep, isComplete);
    
    // Debug the countdown step
    debug("DisplayManager::showCountdown - Step: " + String(currentStep) + ", Text: " + countdownText, "DisplayManager");
    
    // Display countdown on all active displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            // Clear the display first
            _activeDisplays[i]->clear();
            
            // Print the countdown text with emphasis
            _activeDisplays[i]->print("\n==== COUNTDOWN ====");
            _activeDisplays[i]->print(countdownText);
            _activeDisplays[i]->print("==================\n");
        }
    }
}

void DisplayManager::startLightSequence() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    debug("Starting light sequence on RaceReadyScreen", "DisplayManager");
    
    // For each active display, start the light sequence if it's a graphical display
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr && _activeDisplays[i]->getDisplayType() == DisplayType::LCD) {
            // Cast to IGraphicalDisplay to access LVGL-specific methods
            IGraphicalDisplay* lcd = static_cast<IGraphicalDisplay*>(_activeDisplays[i]);
            if (lcd) {
                // Start the light sequence on the RaceReadyScreen
                lcd->startLightSequence();
            }
        }
    }
}

String DisplayManager::formatTimeMMSSmmm(uint32_t timeMs) {
    DEBUG_PRINT_METHOD();
    uint32_t totalSeconds = timeMs / 1000;
    uint32_t minutes = totalSeconds / 60;
    uint32_t seconds = totalSeconds % 60;
    uint32_t millis = timeMs % 1000;
    
    char buffer[16]; // MM:SS:mmm\0
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%03lu", (unsigned long)minutes, (unsigned long)seconds, (unsigned long)millis);
    return String(buffer);
}

String DisplayManager::formatCountdown(int currentStep, bool isComplete) {
    DEBUG_PRINT_METHOD();
    // Format the countdown display
    // Reset the countdown display if this is the first number (5)
    static const int countdownStart = 5; // Match the value in LightsModule
    if (currentStep == countdownStart) {
        _countdownDisplay = "";
    }
    
    if (currentStep > 0) {
        // First number or adding to sequence
        if (_countdownDisplay.isEmpty()) {
            _countdownDisplay = String(currentStep);
        } else {
            _countdownDisplay += "..." + String(currentStep);
        }
    } else if (isComplete) {
        // Show final GO!
        _countdownDisplay += "...GO!";
        
        // Reset for next countdown
        String result = _countdownDisplay;
        _countdownDisplay = "";
        return result;
    }
    
    return _countdownDisplay;
}

void DisplayManager::showRaceStatus(const RaceModule& raceModule, bool isPaused) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Format the race status
    String raceStatus = formatRaceStatus(raceModule, isPaused);
    
    // Display race status on all active displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            _activeDisplays[i]->clear();
            _activeDisplays[i]->print(raceStatus);
        }
    }
}

String DisplayManager::formatRaceStatus(const RaceModule& raceModule, bool isPaused) {
    DEBUG_PRINT_METHOD();
    String status = "";
    
    // Add race mode
    RaceMode mode = raceModule.getRaceMode();
    status += "Mode: ";
    switch (mode) {
        case RaceMode::LAPS: status += "LAPS"; break;
        case RaceMode::TIMER: status += "TIMER"; break;
        case RaceMode::DRAG: status += "DRAG"; break;
        case RaceMode::RALLY: status += "RALLY"; break;
        default: status += "UNKNOWN"; break;
    }
    
    // Add race state
    status += " | State: ";
    if (isPaused) {
        status += "PAUSED";
    } else {
        status += "ACTIVE";
    }
    
    // Add lap counts for each lane
    status += "\n";
    for (int i = 1; i <= raceModule.getNumLanes(); i++) {
        const RaceLaneData& lane = raceModule.getLaneData(i);
        if (lane.enabled) {
            status += "Lane " + String(i) + ": ";
            status += String(lane.currentLap) + "/" + String(lane.totalLaps) + " laps";
            if (lane.finished) {
                status += " (FINISHED)";
            }
            status += "\n";
        }
    }
    
    return status;
}

void DisplayManager::showMessage(const String& message) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Display message on all active displays
    for (int i = 0; i < _activeDisplayCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            _activeDisplays[i]->print(message);
        }
    }
}

ScreenType DisplayManager::getCurrentScreen() const {
    DEBUG_PRINT_METHOD();
    return _currentScreen;
}

int DisplayManager::getActiveDisplayTypes(DisplayType* displayTypes, int maxCount) const {
    DEBUG_PRINT_METHOD();
    int count = 0;
    
    // Get active display types
    for (int i = 0; i < _activeDisplayCount && count < maxCount; i++) {
        if (_activeDisplays[i] != nullptr) {
            displayTypes[count++] = _activeDisplays[i]->getDisplayType();
        }
    }
    
    return count;
}

void DisplayManager::log(LogLevel level, const String& message, const String& moduleName) {
    // Always log debug messages, even if not initialized
    bool isDebug = (level == LogLevel::Debug);
    
    // Format log message with module name
    String out = "";
    if (moduleName.length() > 0) out += moduleName + ": ";
    out += message;
    
    // Add prefix based on log level
    switch (level) {
        case LogLevel::Debug:    out = "[DEBUG] " + out; break;
        case LogLevel::Info:     out = "[INFO] " + out; break;
        case LogLevel::Warning:  out = "[WARN] " + out; break;
        case LogLevel::Error:    out = "[ERROR] " + out; break;
    }
    
    // Always output to Serial for debug messages, regardless of initialization state
    // For other log levels, only output if initialized
    if (isDebug || _initialized) {
#ifdef ENABLE_OUTPUT_SERIAL
        Serial.println(out);
#endif
    }
}

void DisplayManager::raceLog(const String& message) {
    DEBUG_PRINT_METHOD();
#ifdef ENABLE_OUTPUT_SERIAL
    Serial.println(String("LC: ") + message);
#endif
}

void DisplayManager::debug(const String& message, const String& moduleName) {
    DEBUG_PRINT_METHOD();
    log(LogLevel::Debug, message, moduleName);
}

void DisplayManager::info(const String& message, const String& moduleName) {
    DEBUG_PRINT_METHOD();
    log(LogLevel::Info, message, moduleName);
}

void DisplayManager::warning(const String& message, const String& moduleName) {
    DEBUG_PRINT_METHOD();
    log(LogLevel::Warning, message, moduleName);
}

void DisplayManager::error(const String& message, const String& moduleName) {
    DEBUG_PRINT_METHOD();
    log(LogLevel::Error, message, moduleName);
}
