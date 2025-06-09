#include "LightsModule.h"
#include "DisplayModule/DisplayManager.h"

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        static bool firstCall = true; \
        unsigned long now = millis(); \
        if (firstCall || (now - lastDebugPrint > DEBUG_THROTTLE_MS)) { \
            DisplayManager::getInstance().debug(String("[LightsModule] ") + __FUNCTION__, "LightsModule"); \
            lastDebugPrint = now; \
            firstCall = false; \
        } \
    } while(0)

// Static pointer to the LightsModule instance for the static callback
static LightsModule* s_instance = nullptr;

bool LightsModule::initialize() {
    DEBUG_PRINT_METHOD();
    // Check if already initialized
    if (_initialized) {
        DisplayManager::getInstance().info("LightsModule: Already initialized", "LightsModule");
        return true;
    }
    
    DisplayManager::getInstance().info("LightsModule: Initializing...", "LightsModule");
    
    // Set the static instance pointer for the static callback
    s_instance = this;
    
    // Set initial state
    _currentStep = 0;
    _active = false;
    _currentLightState = LightState::Off;
    
    // Clean up any existing timer
    if (_countdownTimer) {
        delete _countdownTimer;
        _countdownTimer = nullptr;
    }
    
    // Mark as initialized
    _initialized = true;
    DisplayManager::getInstance().info("LightsModule: Initialized", "LightsModule");
    return true;
}

void LightsModule::startSequence(uint32_t intervalMs) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        DisplayManager::getInstance().info("LightsModule: Cannot start sequence - not initialized", "LightsModule");
        return;
    }
    
    // Force a shorter interval for testing (200ms)
    _intervalMs = 200; // Override the passed interval for faster testing
    DisplayManager::getInstance().debug("LightsModule: Starting sequence with interval: " + String(_intervalMs) + "ms", "LightsModule");
    
    _currentStep = _countdownStart;
    _lastStepTime = TimeManager::GetInstance().GetCurrentTimeMs();
    _active = true;
    
    // Set initial light state
    setLightState(LightState::Ready);
    
    // Display the first countdown step directly
    DisplayManager::getInstance().debug("Starting countdown sequence with step: " + String(_currentStep), "LightsModule");
    
    // Display the first countdown step
    displayCountdown(_currentStep);
    
    // Notify about countdown step
    if (_onCountdownStepCallback) {
        _onCountdownStepCallback(_currentStep);
    }
    
    // Display the initial countdown number
    displayCountdown(_currentStep);
}

void LightsModule::update() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized or not active
    if (!_initialized || !_active) {
        return;
    }
    
    uint32_t now = TimeManager::GetInstance().GetCurrentTimeMs();
    uint32_t elapsed = now - _lastStepTime;
    
    // Debug output every 500ms to show countdown progress
    static uint32_t lastDebugTime = 0;
    if (now - lastDebugTime >= 500) {
        DisplayManager::getInstance().debug("LightsModule::update - Current step: " + String(_currentStep) + ", Elapsed: " + String(elapsed) + "ms, Interval: " + String(_intervalMs) + "ms", "LightsModule");
        lastDebugTime = now;
    }
    
    if (elapsed >= _intervalMs) {
        DisplayManager::getInstance().debug("LightsModule::update - Decrementing step from " + String(_currentStep) + " to " + String(_currentStep-1), "LightsModule");
        _currentStep--;
        _lastStepTime = now;
        
        if (_currentStep > 0) {
            // Display accumulated countdown
            displayCountdown(_currentStep);
            
            // Notify about countdown step
            if (_onCountdownStepCallback) {
                _onCountdownStepCallback(_currentStep);
            }
            
            // Update light state based on countdown progress
            if (_currentStep == 1) {
                // Last step before start - red lights on
                setLightState(LightState::RedOn);
            }
        } else {
            // Red lights off - this is the actual race start trigger
            setLightState(LightState::RedOff);
            
            // Display the final GO!
            displayCountdown(0);
            
            // Notify that countdown is complete
            if (_onCountdownCompletedCallback) {
                _onCountdownCompletedCallback();
            }
            
            // Short delay before showing green lights
            // This is handled in triggerGo()
            triggerGo();
        }
    }
}

void LightsModule::displayCountdown(int number) {
    DEBUG_PRINT_METHOD();
    static String countdownDisplay = "";
    
    // Reset the countdown display if this is the first number (5)
    if (number == _countdownStart) {
        countdownDisplay = "";
        DisplayManager::getInstance().debug("Resetting countdown display", "LightsModule");
    }
    
    if (number > 0) {
        // First number or adding to sequence
        if (countdownDisplay.isEmpty()) {
            countdownDisplay = String(number);
            DisplayManager::getInstance().debug("First countdown step: " + countdownDisplay, "LightsModule");
        } else {
            countdownDisplay += "..." + String(number);
            DisplayManager::getInstance().debug("Updated countdown: " + countdownDisplay, "LightsModule");
        }
        
        // Display the accumulated countdown
        DisplayManager::getInstance().info("COUNTDOWN: " + countdownDisplay, "LightsModule");
        
        // Also print directly to Serial to ensure it's visible
        Serial.println("\nCOUNTDOWN: " + countdownDisplay);
    } else {
        // Show final GO!
        countdownDisplay += "...GO!";
        DisplayManager::getInstance().debug("Final countdown: " + countdownDisplay, "LightsModule");
        
        // Display the complete countdown
        DisplayManager::getInstance().info("COUNTDOWN: " + countdownDisplay, "LightsModule");
        
        // Also print directly to Serial to ensure it's visible
        Serial.println("\nCOUNTDOWN: " + countdownDisplay);
        
        // Reset for next countdown
        countdownDisplay = "";
    }
}

void LightsModule::triggerGo() {
    DEBUG_PRINT_METHOD();
    // This method is called when the countdown reaches zero
    
    // Set green lights on after a short delay
    // In a real implementation, this would be handled with a timer
    // For now, we'll just set it immediately
    setLightState(LightState::GreenOn);
    
    // Mark countdown as complete
    _active = false;
}

bool LightsModule::isActive() const {
    DEBUG_PRINT_METHOD();
    return _active;
}

void LightsModule::setCountdownStart(int startValue) {
    DEBUG_PRINT_METHOD();
    _countdownStart = startValue;
}

void LightsModule::setCountdownInterval(uint32_t intervalMs) {
    DEBUG_PRINT_METHOD();
    _intervalMs = intervalMs;
}

int LightsModule::getCurrentStep() const {
    DEBUG_PRINT_METHOD();
    return _currentStep;
}

uint32_t LightsModule::getCountdownInterval() const {
    DEBUG_PRINT_METHOD();
    return _intervalMs;
}

LightState LightsModule::getLightState() const {
    DEBUG_PRINT_METHOD();
    return _currentLightState;
}

void LightsModule::setOnLightStateChangedCallback(LightStateChangedCallback callback) {
    DEBUG_PRINT_METHOD();
    _onLightStateChangedCallback = callback;
}

void LightsModule::setOnCountdownStepCallback(CountdownStepCallback callback) {
    DEBUG_PRINT_METHOD();
    _onCountdownStepCallback = callback;
}

void LightsModule::setOnCountdownCompletedCallback(CountdownCompletedCallback callback) {
    DEBUG_PRINT_METHOD();
    _onCountdownCompletedCallback = callback;
}

void LightsModule::setLightState(LightState state) {
    DEBUG_PRINT_METHOD();
    if (_currentLightState != state) {
        _currentLightState = state;
        
        // Update display based on the new light state
        switch (state) {
            case LightState::Off:
                DisplayManager::getInstance().info("Lights: OFF", "LightsModule");
                break;
                
            case LightState::Ready:
                DisplayManager::getInstance().info("Lights: READY", "LightsModule");
                break;
                
            case LightState::RedOn:
                DisplayManager::getInstance().info("Lights: RED", "LightsModule");
                break;
                
            case LightState::RedOff:
                DisplayManager::getInstance().info("Lights: START", "LightsModule");
                break;
                
            case LightState::GreenOn:
                DisplayManager::getInstance().info("Lights: GREEN", "LightsModule");
                break;
        }
        
        // Notify observers of the state change
        if (_onLightStateChangedCallback) {
            _onLightStateChangedCallback(state);
        }
    }
}
