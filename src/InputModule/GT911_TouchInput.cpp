#include "GT911_TouchInput.h"
#include "../common/DebugUtils.h" // Use standard debug header

#ifdef SIMULATOR
#include "../common/ArduinoCompat.h" // Simulator compatibility
#else
#include <Arduino.h> // For map() and constrain() functions
#endif
#include "DisplayModule/DisplayManager.h"

// Initialize static members
std::queue<InputEvent> GT911_TouchInput::_inputEventQueue;
int16_t GT911_TouchInput::_lastTouchX = 0;
int16_t GT911_TouchInput::_lastTouchY = 0;
lv_indev_state_t GT911_TouchInput::_lastTouchState = LV_INDEV_STATE_RELEASED;
lv_indev_t* GT911_TouchInput::_lvglInputDevice = nullptr;

// Touch debouncing
static const uint32_t DEBOUNCE_DELAY_MS = 20; // 20ms debounce time
// static uint32_t _lastTouchTime = 0; // Commented out as currently unused

GT911_TouchInput::GT911_TouchInput() : _touchController(nullptr) {
}

GT911_TouchInput::~GT911_TouchInput() {
    if (_touchController) {
        delete _touchController;
        _touchController = nullptr;
    }
    // LVGL handles its own input device cleanup if any is needed beyond unregistering
}

bool GT911_TouchInput::initializeInput() {
    Serial.println("\n===== GT911 Touch Initialization =====");
    Serial.printf("Initializing GT911 Touch Controller...\n");
    Serial.printf("Pins - SDA: %d, SCL: %d, INT: %d, RST: %d\n", 
                 TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST);
    Serial.printf("Display dimensions: %dx%d\n", TOUCH_MAP_X1, TOUCH_MAP_Y1);

    #ifdef SIMULATOR
    // In simulator mode, we don't need a physical touch controller
    // We'll use SDL events for touch input instead
    Serial.println("Using simulator touch input");
    Serial.printf("Simulated touch panel resolution: %dx%d\n", TOUCH_PANEL_WIDTH, TOUCH_PANEL_HEIGHT);
#else
    // Create the touch controller instance with actual touch panel resolution
    _touchController = new TAMC_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, 
                                     TOUCH_GT911_INT, TOUCH_GT911_RST, 
                                     TOUCH_PANEL_WIDTH, TOUCH_PANEL_HEIGHT);
    
    if (!_touchController) {
        Serial.println("ERROR: Failed to create TAMC_GT911 instance!");
        return false;
    }
    Serial.println("Created TAMC_GT911 instance");
    Serial.printf("Touch panel resolution: %dx%d\n", TOUCH_PANEL_WIDTH, TOUCH_PANEL_HEIGHT);

    // Initialize the controller
    Serial.println("Initializing touch controller...");
    _touchController->begin();
    Serial.println("Touch controller initialized");
    
    // Set rotation
    _touchController->setRotation(ROTATION_NORMAL);
    Serial.println("Set touch rotation to NORMAL");
#endif

    // Initialize LVGL input device
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_read_cb;

    _lvglInputDevice = lv_indev_drv_register(&indev_drv);
    if (!_lvglInputDevice) {
        Serial.println("ERROR: Failed to register LVGL input device!");
        delete _touchController;
        _touchController = nullptr;
        return false;
    }
    
    Serial.println("LVGL input device registered successfully");
    Serial.println("GT911 Touch initialization complete!");
    Serial.println("==================================\n");
    
    return true;
}

// This method is called by the InputManager
bool GT911_TouchInput::poll(InputEvent& event) {
    static uint32_t lastPollTime = 0;
    static uint32_t pollCount = 0;
    uint32_t now = millis();
    
    // Process raw touch data to update _lastTouchX, _lastTouchY, _lastTouchState
    readRawTouch();
    
    // Update poll timing
    if (now - lastPollTime > 1000) {
        pollCount++;
        lastPollTime = now;
    }
    
    // Check for queued events
    if (!_inputEventQueue.empty()) {
        // Copy the event from the queue to the output parameter
        event = _inputEventQueue.front();
        _inputEventQueue.pop();
        
        // Debug output for touch events
        DisplayManager::getInstance().debug("Touch event processed: " + String(static_cast<int>(event.command)), "GT911_TouchInput");
                     
        return true;
    }
    
    return false;
}

// LVGL calls this function periodically to get the current touch state
void GT911_TouchInput::lvgl_touch_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    static lv_indev_state_t lastState = LV_INDEV_STATE_RELEASED;
    
    // Get the latest touch data
    data->point.x = _lastTouchX;
    data->point.y = _lastTouchY;
    data->state = _lastTouchState;
    
    // Update last state for change detection
    if (_lastTouchState != lastState) {
        lastState = _lastTouchState;
    }
    
    // Note: We don't reset _lastTouchState here as LVGL needs to see both PRESSED and RELEASED states
    // Reset is handled in readRawTouch() when new data is available
}

// Called by LVGL widget event handlers (in DisplayDriver typically) to queue a system event
void GT911_TouchInput::queueSystemInputEvent(const InputEvent& sysEvent) {
    _inputEventQueue.push(sysEvent);
    // DEBUG_DEBUG("GT911_TouchInput: Queued event - Command: %d, SourceID: %d, Value: %d", 
    //          static_cast<int>(sysEvent.command), sysEvent.sourceId, sysEvent.value);
}

bool GT911_TouchInput::validateTouchCoordinates(int16_t& x, int16_t& y) const {
    // Store original values for debugging
    int16_t originalX = x;
    int16_t originalY = y;
    
    // Clamp values to valid range
    x = clamp(x, MIN_TOUCH_X, MAX_TOUCH_X);
    y = clamp(y, MIN_TOUCH_Y, MAX_TOUCH_Y);
    
    // Check if values were out of bounds
    bool wasValid = (x == originalX && y == originalY);
    
    // Log warning if coordinates were adjusted
    if (!wasValid) {
        String warning = "Touch coordinates adjusted from (";
        warning += originalX;
        warning += ", ";
        warning += originalY;
        warning += ") to (";
        warning += x;
        warning += ", ";
        warning += y;
        warning += ")";
        DisplayManager::getInstance().warning(warning, "GT911_TouchInput");
    }
    
    return wasValid;
}

// Reads from the touch controller and updates the static members for LVGL
void GT911_TouchInput::readRawTouch() {
    static uint32_t lastDebugOutput = 0;
    static uint32_t touchCount = 0;
    static uint32_t lastTouchTime = 0;
    uint32_t now = millis();
    
#ifdef SIMULATOR
    // In simulator mode, we use SDL events for touch input
    // This is handled by the SDLInputHandler class, which will update
    // _lastTouchX, _lastTouchY, and _lastTouchState directly
    
    // For testing, we can simulate touch events here
    // This is just a placeholder - in a real implementation,
    // the SDL event system would update these values
    
    // For now, we'll just leave the touch state as it is
    // The actual touch events will be handled by SDL
    
#else
    if (!_touchController) {
        return;
    }

    // Read data from GT911
    _touchController->read();
    
    if (_touchController->isTouched) {
        // Get raw touch coordinates
        int16_t rawX = _touchController->points[0].x;
        int16_t rawY = _touchController->points[0].y;
        
        // Map touch coordinates to display coordinates
        // Invert both X and Y axes to match display orientation
        _lastTouchX = map(rawX, 0, TOUCH_PANEL_WIDTH, TOUCH_MAP_X2, TOUCH_MAP_X1);
        _lastTouchY = map(rawY, 0, TOUCH_PANEL_HEIGHT, TOUCH_MAP_Y2, TOUCH_MAP_Y1); // Inverted Y mapping
        
        // Ensure coordinates are within display bounds
        _lastTouchX = constrain(_lastTouchX, 0, TOUCH_MAP_X2);
        _lastTouchY = constrain(_lastTouchY, 0, TOUCH_MAP_Y2);
        
        // Validate and adjust touch coordinates
        validateTouchCoordinates(_lastTouchX, _lastTouchY);
        
        _lastTouchState = LV_INDEV_STATE_PRESSED;
        touchCount++;
        lastTouchTime = now;
        
        // Update touch timing
        lastDebugOutput = now;
    } else {
        _lastTouchState = LV_INDEV_STATE_RELEASED;
        
        // Reset touch count if we had a previous touch
        if (touchCount > 0 && now - lastTouchTime < 1000) {
            touchCount = 0;
        } else if (now - lastDebugOutput > 1000) {
            lastDebugOutput = now;
        }
    }
#endif
}
