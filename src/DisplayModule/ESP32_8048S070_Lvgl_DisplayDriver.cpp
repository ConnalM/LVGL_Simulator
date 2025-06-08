#include "ESP32_8048S070_Lvgl_DisplayDriver.h"
#include "../common/DebugUtils.h" // Correct relative path
#include "../DisplayModule/lvgl/screens/RaceScreen.h" // New race screen
#include "../DisplayModule/lvgl/screens/RaceReadyScreen.h" // Relative path
#include "../DisplayModule/lvgl/screens/PauseScreen.h" // Pause screen
#include "../DisplayModule/lvgl/screens/StopScreen.h" // Stop screen
#include "../common/Types.h"        // For RaceData, ErrorInfo etc.
#include "../DisplayModule/DisplayManager.h" // For DisplayManager
#include "../RaceDataStub.h" // For test mode

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        unsigned long now = millis(); \
        if (now - lastDebugPrint > DEBUG_THROTTLE_MS) { \
            Serial.printf("[ESP32_8048S070] %s\n", __FUNCTION__); \
            lastDebugPrint = now; \
        } \
    } while(0)

// Initialize static LVGL draw buffer descriptor (buffer itself is now dynamic)
static lv_disp_draw_buf_t _lvglDrawBuf;

ESP32_8048S070_Lvgl_DisplayDriver::ESP32_8048S070_Lvgl_DisplayDriver() : 
    _rgbPanel(nullptr), _gfx(nullptr), _psram_lvglBuffer1(nullptr), 
    ui_MainMenuScreen(nullptr), ui_RaceReadyScreen(nullptr),
    ui_ConfigScreen(nullptr), ui_RaceActiveScreen(nullptr),
    config_screen_(nullptr), ui_CountdownScreen(nullptr),
    race_ready_screen_(nullptr), _debugLabel(nullptr) {
    DEBUG_PRINT_METHOD();
}

ESP32_8048S070_Lvgl_DisplayDriver::~ESP32_8048S070_Lvgl_DisplayDriver() {
    DEBUG_PRINT_METHOD();
    // Clean up RaceReadyScreen
    if (race_ready_screen_) {
        delete race_ready_screen_;
        race_ready_screen_ = nullptr;
        DPRINTLN("Freed RaceReadyScreen instance.");
    }
    
    // Clean up ConfigScreen
    if (config_screen_) {
        delete config_screen_;
        config_screen_ = nullptr;
        DPRINTLN("Freed ConfigScreen instance.");
    }
    
    if (this->_psram_lvglBuffer1) {
        heap_caps_free(this->_psram_lvglBuffer1);
        this->_psram_lvglBuffer1 = nullptr;
        DPRINTLN("Freed LVGL PSRAM buffer.");
    }
    if (_gfx) {
        delete _gfx;
        _gfx = nullptr;
    }
    if (_rgbPanel) {
        delete _rgbPanel;
        _rgbPanel = nullptr;
    }
}

bool ESP32_8048S070_Lvgl_DisplayDriver::initialize() {
    DEBUG_PRINT_METHOD();
    DPRINTLN("\n===== ESP32_8048S070_Lvgl_DisplayDriver: Initialization Start =====");
    DPRINTF("Display dimensions: %dx%d\n", LCD_WIDTH, LCD_HEIGHT);
    DPRINTF("PCLK speed: %d Hz\n", PREFER_SPEED_HZ);
    DPRINTF("HSYNC: front=%d, pulse=%d, back=%d\n", HSYNC_FRONT_PORCH, HSYNC_PULSE_WIDTH, HSYNC_BACK_PORCH);
    DPRINTF("VSYNC: front=%d, pulse=%d, back=%d\n", VSYNC_FRONT_PORCH, VSYNC_PULSE_WIDTH, VSYNC_BACK_PORCH);
    DPRINTF("Pin Definitions - DE:%d, VSYNC:%d, HSYNC:%d, PCLK:%d, BL:%d\n", 
           LCD_DE_PIN, LCD_VSYNC_PIN, LCD_HSYNC_PIN, LCD_PCLK_PIN, LCD_BACKLIGHT_PIN);

    // 1. Initialize GFX Driver (Arduino_GFX)
    DPRINTLN("\n[1/5] Initializing Arduino_ESP32RGBPanel...");
    _rgbPanel = new Arduino_ESP32RGBPanel(
        LCD_DE_PIN, LCD_VSYNC_PIN, LCD_HSYNC_PIN, LCD_PCLK_PIN,
        LCD_R0_PIN, LCD_R1_PIN, LCD_R2_PIN, LCD_R3_PIN, LCD_R4_PIN,
        LCD_G0_PIN, LCD_G1_PIN, LCD_G2_PIN, LCD_G3_PIN, LCD_G4_PIN, LCD_G5_PIN,
        LCD_B0_PIN, LCD_B1_PIN, LCD_B2_PIN, LCD_B3_PIN, LCD_B4_PIN,
        HSYNC_POLARITY, HSYNC_FRONT_PORCH, HSYNC_PULSE_WIDTH, HSYNC_BACK_PORCH,
        VSYNC_POLARITY, VSYNC_FRONT_PORCH, VSYNC_PULSE_WIDTH, VSYNC_BACK_PORCH,
        PCLK_ACTIVE_NEG, PREFER_SPEED_HZ);

    if (!_rgbPanel) {
        DEBUG_ERROR("FAILED: Could not create Arduino_ESP32RGBPanel instance");
        return false;
    }
    DPRINTLN("  - Arduino_ESP32RGBPanel instance created");

    // Create Arduino_RGB_Display with rotation 0 and auto_flush=true
    DPRINTLN("\n[2/5] Creating Arduino_RGB_Display...");
    _gfx = new Arduino_RGB_Display(LCD_WIDTH, LCD_HEIGHT, _rgbPanel, 0 /*rotation*/, true /*auto_flush*/);
    if (!_gfx) {
        DEBUG_ERROR("FAILED: Could not create Arduino_RGB_Display instance");
        delete _rgbPanel; _rgbPanel = nullptr;
        return false;
    }
    DPRINTLN("  - Arduino_RGB_Display instance created");

    DPRINTF("  - Calling _gfx->begin(%d)...\n", PREFER_SPEED_HZ);
    if (!_gfx->begin(PREFER_SPEED_HZ)) {
        DEBUG_ERROR("FAILED: _gfx->begin() returned false");
        delete _gfx; _gfx = nullptr;
        delete _rgbPanel; _rgbPanel = nullptr;
        return false;
    }
    DPRINTLN("  - _gfx->begin() successful");
    // _gfx->fillScreen(BLACK); // Optional: Clear screen initially with GFX

    // Manual backlight control
#ifdef LCD_BACKLIGHT_PIN
    DPRINTLN("\n[3/5] Initializing backlight...");
    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
    digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
    DPRINTF("  - Backlight turned ON (pin %d)\n", LCD_BACKLIGHT_PIN);
#else
    DPRINTLN("\n[3/5] WARNING: No backlight pin defined (LCD_BACKLIGHT_PIN)");
#endif

    // 2. Initialize LVGL
    DPRINTLN("\n[4/5] Initializing LVGL...");
    lv_init();
    DPRINTF("  - LVGL initialized (v%d.%d.%d)\n", lv_version_major(), lv_version_minor(), lv_version_patch());

    // Calculate and log memory requirements
    DPRINTLN("\n[5/5] Setting up display buffers...");
    size_t total_pixels = LCD_WIDTH * LCD_HEIGHT;
    size_t buffer_pixel_count = total_pixels / 4; // Number of pixels
    size_t buffer_size_bytes = buffer_pixel_count * sizeof(lv_color_t);
    
    DPRINTF("  - Display resolution: %ux%u = %u pixels\n", LCD_WIDTH, LCD_HEIGHT, total_pixels);
    DPRINTF("  - LVGL buffer size: %u pixels (%.1f%% of total)\n", buffer_pixel_count, (float)buffer_pixel_count/total_pixels*100);
    DPRINTF("  - Buffer memory required: %u bytes\n", buffer_size_bytes);
    DPRINTF("  - LV_COLOR_DEPTH: %d bits\n", LV_COLOR_DEPTH);
    DPRINTF("  - sizeof(lv_color_t): %u bytes\n", sizeof(lv_color_t));
    DPRINTF("  - Attempting to allocate PSRAM buffer...");
    
    // Check available PSRAM before allocation
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    DPRINTF("Available PSRAM: %u bytes\n", free_psram);
    
    // Allocate primary buffer in PSRAM
    this->_psram_lvglBuffer1 = (lv_color_t*)heap_caps_malloc(buffer_size_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    bool using_psram = (this->_psram_lvglBuffer1 != nullptr);

    // Fallback to internal RAM if PSRAM allocation fails
    if (!using_psram) {
        DPRINTLN("  - PSRAM allocation failed!");
        DPRINTLN("  - Attempting to allocate smaller buffer in internal RAM...");
        
        // Try with a smaller buffer (1/10th of screen)
        buffer_pixel_count = (LCD_WIDTH * LCD_HEIGHT) / 10;
        buffer_size_bytes = buffer_pixel_count * sizeof(lv_color_t);
        this->_psram_lvglBuffer1 = (lv_color_t*)heap_caps_malloc(buffer_size_bytes, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        
        if (!this->_psram_lvglBuffer1) {
            DEBUG_ERROR("FATAL: Failed to allocate any display buffers!");
            if (_gfx) { delete _gfx; _gfx = nullptr; }
            if (_rgbPanel) { delete _rgbPanel; _rgbPanel = nullptr; }
            return false;
        }
        DPRINTF("  - WARNING: Using smaller buffer in internal RAM: %u pixels (%u bytes)\n", 
               buffer_pixel_count, buffer_size_bytes);
    } else {
        DPRINTF("  - Successfully allocated buffer in %s: %u pixels (%u bytes)\n",
               using_psram ? "PSRAM" : "internal RAM", 
               buffer_pixel_count, buffer_size_bytes);
    }

    // Initialize LVGL draw buffer
    DPRINTLN("\nInitializing LVGL draw buffer and display driver...");
    lv_disp_draw_buf_init(&_lvglDrawBuf, this->_psram_lvglBuffer1, nullptr, buffer_pixel_count);
    DPRINTF("  - LVGL draw buffer initialized with %u pixels\n", buffer_pixel_count);

    // Initialize LVGL display driver
    lv_disp_drv_init(&this->_lvglDisplayDriver);
    this->_lvglDisplayDriver.hor_res = LCD_WIDTH;
    this->_lvglDisplayDriver.ver_res = LCD_HEIGHT;
    this->_lvglDisplayDriver.flush_cb = ESP32_8048S070_Lvgl_DisplayDriver::lvgl_display_flush_cb;
    this->_lvglDisplayDriver.user_data = this;
    this->_lvglDisplayDriver.draw_buf = &_lvglDrawBuf;
    
    DPRINTLN("  - Registering LVGL display driver...");
    lv_disp_t* disp = lv_disp_drv_register(&this->_lvglDisplayDriver);
    if (!disp) {
        DEBUG_ERROR("FAILED: lv_disp_drv_register() returned NULL");
        return false;
    }
    DPRINTF("  - LVGL display driver registered successfully (handle: %p)\n", disp);

    // Note: Touch input driver (GT911_TouchInput) should be initialized separately
    // and will register its own LVGL input device.
    DPRINTLN("\nCreating LVGL screens...");
    
    // 3. Create LVGL Screens
    createMainMenuScreen();
    DPRINTLN("  - Created Main Menu screen");
    
    createRaceReadyScreen();
    DPRINTLN("  - Created Race Ready screen");
    
    createRaceActiveScreen();
    DPRINTLN("  - Created Race Active screen");
    
    createConfigScreen();
    DPRINTLN("  - Created Config Menu screen");
    
    createCountdownScreen();
    DPRINTLN("  - Created Countdown screen");

    // Don't load any screen by default - this should be done by DisplayManager or SystemController
    // This respects proper separation of concerns and module boundaries
    DPRINTLN("\nInitialization complete - screens created but none loaded by default");
    DPRINTLN("The caller (DisplayManager/SystemController) will decide which screen to show");
    
    // TODO: In normal operation (non-test mode), main.cpp should explicitly call displayManager.showMain()
    // after initialization to show the main menu as the default screen. This ensures proper architectural
    // boundaries are maintained while still providing the expected default behavior.

    DPRINTLN("\n===== ESP32_8048S070_Lvgl_DisplayDriver: Initialization SUCCESSFUL =====");
    return true;
}

void ESP32_8048S070_Lvgl_DisplayDriver::update() {
    DEBUG_PRINT_METHOD();
    static uint32_t lastUpdate = 0;
    const uint32_t updateInterval = 16;  // ~60fps (16.67ms)
    
    uint32_t now = millis();
    if (now - lastUpdate >= updateInterval) {
        lv_timer_handler();
        lastUpdate = now;
    }
    
    // Small delay to prevent 100% CPU usage
    vTaskDelay(1);
    
    // If we're using LV_TICK_CUSTOM=1, ensure TimeManager is updating LVGL ticks
    // This is handled by SystemController calling TimeManager::update()
}

void ESP32_8048S070_Lvgl_DisplayDriver::clear() {
    DEBUG_PRINT_METHOD();
    // Temporarily making this a no-op to prevent crashes
    // We'll implement proper screen clearing after stabilizing the display transitions
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::clear() called - no-op for now");
    
    // Note: We're not calling any LVGL functions here to prevent crashes
    // The display will be refreshed when the new screen is loaded
}

// Basic print/printf - might draw to a dedicated debug label on screen
void ESP32_8048S070_Lvgl_DisplayDriver::print(const String& message, bool newLine) {
    DEBUG_PRINT_METHOD();
    if (!_debugLabel) {
        // Create a debug label on the current screen if it doesn't exist
        // This is a basic implementation. A better one might use a scrolling log area.
        lv_obj_t* act_scr = lv_disp_get_scr_act(NULL);
        if (act_scr) {
            _debugLabel = lv_label_create(act_scr);
            lv_obj_align(_debugLabel, LV_ALIGN_BOTTOM_LEFT, 5, -5);
            lv_label_set_long_mode(_debugLabel, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(_debugLabel, LCD_WIDTH - 10);
        }
    }
    if (_debugLabel) {
        String currentText = lv_label_get_text(_debugLabel);
        if (newLine) currentText += "\n";
        currentText += message;
        // Limit text length to avoid excessive memory use by label
        if (currentText.length() > 512) { 
            currentText = currentText.substring(currentText.length() - 512);
        }
        lv_label_set_text(_debugLabel, currentText.c_str());
    }
}

void ESP32_8048S070_Lvgl_DisplayDriver::printf(const char* format, ...) {
    DEBUG_PRINT_METHOD();
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    print(String(buffer), false); // Assume printf doesn't auto-newline unless specified in format
}

// --- IGraphicalDisplay Implementations ---
void ESP32_8048S070_Lvgl_DisplayDriver::setCursor(int x, int y) {
    DEBUG_PRINT_METHOD(); 
    // Store cursor position for future text operations if needed
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::setCursor called");
    // No direct cursor in LVGL, but we can store position for future use
}

void ESP32_8048S070_Lvgl_DisplayDriver::setTextColor(uint32_t color) {
    DEBUG_PRINT_METHOD(); 
    // Store text color for future text operations
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::setTextColor called");
    // Could be used to set default text color for future text objects
}

void ESP32_8048S070_Lvgl_DisplayDriver::setTextSize(uint8_t size) {
    DEBUG_PRINT_METHOD(); 
    // Store text size for future text operations
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::setTextSize called");
    // Could be used to set default text size for future text objects
}

void ESP32_8048S070_Lvgl_DisplayDriver::drawRect(int x, int y, int w, int h, uint32_t color) {
    DEBUG_PRINT_METHOD();
    // Implementation for LVGL
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::drawRect called");
    
    // Convert 32-bit color to LVGL color format
    lv_color_t lvgl_color = lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    
    // Use a static object for drawing rectangles to avoid memory leaks
    // This approach reuses the same object for all rectangles
    static lv_obj_t* rect = nullptr;
    
    // Create the rectangle object if it doesn't exist yet
    if (rect == nullptr) {
        rect = lv_obj_create(lv_scr_act());
    }
    
    // Update the rectangle properties
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    
    // Set style for outlined rectangle
    lv_obj_set_style_bg_opa(rect, LV_OPA_TRANSP, 0); // Transparent background
    lv_obj_set_style_border_color(rect, lvgl_color, 0);
    lv_obj_set_style_border_width(rect, 1, 0);
    
    // Make sure the rectangle is visible
    lv_obj_clear_flag(rect, LV_OBJ_FLAG_HIDDEN);
    
    // Note: This approach has limitations as it only shows one rectangle at a time
    // For multiple persistent rectangles, a more sophisticated approach would be needed
}

void ESP32_8048S070_Lvgl_DisplayDriver::fillRect(int x, int y, int w, int h, uint32_t color) {
    DEBUG_PRINT_METHOD();
    // Implementation for LVGL
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::fillRect called");
    
    // Convert 32-bit color to LVGL color format
    lv_color_t lvgl_color = lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    
    // Use a static object for drawing filled rectangles to avoid memory leaks
    static lv_obj_t* rect = nullptr;
    
    // Create the rectangle object if it doesn't exist yet
    if (rect == nullptr) {
        rect = lv_obj_create(lv_scr_act());
    }
    
    // Update the rectangle properties
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    
    // Set style for filled rectangle
    lv_obj_set_style_bg_color(rect, lvgl_color, 0);
    lv_obj_set_style_border_width(rect, 0, 0); // No border
    
    // Make sure the rectangle is visible
    lv_obj_clear_flag(rect, LV_OBJ_FLAG_HIDDEN);
    
    // Note: This approach has limitations as it only shows one filled rectangle at a time
}

void ESP32_8048S070_Lvgl_DisplayDriver::drawCircle(int x, int y, int r, uint32_t color) {
    DEBUG_PRINT_METHOD();
    // Implementation for LVGL
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::drawCircle called");
    
    // Convert 32-bit color to LVGL color format
    lv_color_t lvgl_color = lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    
    // Use a static object for drawing circles to avoid memory leaks
    static lv_obj_t* circle = nullptr;
    
    // Create the circle object if it doesn't exist yet
    if (circle == nullptr) {
        circle = lv_obj_create(lv_scr_act());
    }
    
    // Update the circle properties
    lv_obj_set_pos(circle, x - r, y - r); // Position adjusted to center
    lv_obj_set_size(circle, 2 * r, 2 * r);
    
    // Set style for outlined circle
    lv_obj_set_style_radius(circle, r, 0); // Make it circular
    lv_obj_set_style_bg_opa(circle, LV_OPA_TRANSP, 0); // Transparent background
    lv_obj_set_style_border_color(circle, lvgl_color, 0);
    lv_obj_set_style_border_width(circle, 1, 0);
    
    // Make sure the circle is visible
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_HIDDEN);
    
    // Note: This approach has limitations as it only shows one circle at a time
}

void ESP32_8048S070_Lvgl_DisplayDriver::fillCircle(int x, int y, int r, uint32_t color) {
    DEBUG_PRINT_METHOD();
    // Implementation for LVGL
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::fillCircle called");
    
    // Convert 32-bit color to LVGL color format
    lv_color_t lvgl_color = lv_color_make((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    
    // Use a static object for drawing filled circles to avoid memory leaks
    static lv_obj_t* circle = nullptr;
    
    // Create the circle object if it doesn't exist yet
    if (circle == nullptr) {
        circle = lv_obj_create(lv_scr_act());
    }
    
    // Update the circle properties
    lv_obj_set_pos(circle, x - r, y - r); // Position adjusted to center
    lv_obj_set_size(circle, 2 * r, 2 * r);
    
    // Set style for filled circle
    lv_obj_set_style_radius(circle, r, 0); // Make it circular
    lv_obj_set_style_bg_color(circle, lvgl_color, 0);
    lv_obj_set_style_border_width(circle, 0, 0); // No border
    
    // Make sure the circle is visible
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_HIDDEN);
    
    // Note: This approach has limitations as it only shows one filled circle at a time
}

// --- Screen Drawing Methods --- 
void ESP32_8048S070_Lvgl_DisplayDriver::drawMain() {
    DEBUG_PRINT_METHOD();
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::drawMain");
    
    if (ui_MainMenuScreen) {
        DPRINTLN("Loading Main Menu screen...");
        lv_scr_load(ui_MainMenuScreen);
        
        // Force a full refresh of the display
        lv_refr_now(NULL);
        
        // Process any pending LVGL tasks
        lv_task_handler();
        
        DPRINTLN("Main Menu screen loaded and refreshed");
    } else {
        DPRINTLN("ERROR: Main Menu screen is null!");
    }
}

void ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData(const std::vector<RaceLaneData>& laneData) {
    DEBUG_PRINT_METHOD();
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - Updating race data display");
    
    // Debug: Print the lane data we received
    DPRINTF("Received %d lanes of race data:\n", laneData.size());
    for (const auto& lane : laneData) {
        if (lane.enabled) {
            DPRINTF("  Lane %d: Lap %d/%d, Last: %dms, Best: %dms, Total: %dms\n",
                  lane.laneId, lane.currentLap, lane.totalLaps,
                  lane.lastLapTime, lane.bestLapTime, lane.totalTime);
        }
    }
    
    // Check if we have a valid race screen reference
    if (_activeRaceScreen == nullptr) {
        DPRINTLN("_activeRaceScreen is null, trying to get it from UI...");
        
        // If _activeRaceScreen is not set, try to get it from the screen's user data
        if (ui_RaceActiveScreen != nullptr) {
            lv_obj_t* currentScreen = lv_scr_act();
            DPRINTF("Current screen: %p, RaceActiveScreen: %p\n", currentScreen, ui_RaceActiveScreen);
            
            if (currentScreen == ui_RaceActiveScreen) {
                _activeRaceScreen = static_cast<RaceScreen*>(lv_obj_get_user_data(ui_RaceActiveScreen));
                DPRINTF("Got _activeRaceScreen from UI: %p\n", _activeRaceScreen);
            } else {
                DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - Not on race active screen");
                return;
            }
        } else {
            DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - Race active screen not created");
            return;
        }
        
        // If we still don't have a valid reference, return
        if (_activeRaceScreen == nullptr) {
            DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - No RaceScreen instance found");
            return;
        }
    } else {
        DPRINTF("Using existing _activeRaceScreen: %p\n", _activeRaceScreen);
    }
    
    // Get the active race mode UI from the race screen
    RaceModeUI* raceModeUI = _activeRaceScreen->GetActiveRaceModeUI();
    if (raceModeUI == nullptr) {
        DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - No active race mode UI");
        return;
    }
    
    // Check the race mode and update the appropriate UI
    if (raceModeUI->GetMode() == RaceMode::LAPS || raceModeUI->GetMode() == RaceMode::TIMER) {
        // For both LAPS and TIMER modes, we can update the race data
        // as they both use the same update mechanism
        raceModeUI->UpdateRaceData(laneData);
        DPRINTF("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - Updated %s UI\n", 
               raceModeUI->GetMode() == RaceMode::LAPS ? "LapsRaceUI" : "TimerRaceUI");
        
        // Debug output to verify data is being received
        for (size_t i = 0; i < laneData.size(); i++) {
            if (laneData[i].enabled) {
                DPRINTF("Lane %d: Pos %d, Lap %d/%d, Last: %d ms, Best: %d ms\n", 
                       laneData[i].laneId, 
                       laneData[i].position, 
                       laneData[i].currentLap, 
                       laneData[i].totalLaps, 
                       laneData[i].lastLapTime, 
                       laneData[i].bestLapTime);
            }
        }
    } else {
        DPRINTF("ESP32_8048S070_Lvgl_DisplayDriver::updateRaceData - Unsupported race mode: %d\n", 
               static_cast<int>(raceModeUI->GetMode()));
    }
}

void ESP32_8048S070_Lvgl_DisplayDriver::drawConfig() {
    DEBUG_PRINT_METHOD();
    static bool first_run = true;
    
    // Check if LVGL is initialized
    if (!lv_is_initialized()) {
        DPRINTLN("ERROR: LVGL not initialized, cannot show config screen");
        return;
    }
    
    DPRINTF("drawConfig - First run: %s\n", first_run ? "true" : "false");
    
    // Create the config screen if it doesn't exist
    if (!config_screen_) {
        DPRINTLN("Creating new ConfigScreen instance");
        config_screen_ = new (std::nothrow) ConfigScreen();
        if (!config_screen_) {
            DPRINTLN("ERROR: Failed to create ConfigScreen (memory allocation failed)");
            return;
        }
        DPRINTLN("ConfigScreen instance created");
    }
    
    // Store a reference to the config screen for future use
    ui_ConfigScreen = config_screen_->getScreen();
    if (!ui_ConfigScreen) {
        DPRINTLN("ERROR: Failed to get screen object from ConfigScreen");
        return;
    }
    
    DPRINTLN("Calling ConfigScreen::Show()");
    config_screen_->Show();
    
    // Force a full screen refresh on first run
    if (first_run) {
        DPRINTLN("First run - forcing screen refresh");
        lv_refr_now(nullptr);
        first_run = false;
    }
    
    DPRINTLN("drawConfig completed successfully");
}

// This method is now part of drawRaceReady and should not be used directly
// It's kept for backward compatibility but will be removed in future versions
void ESP32_8048S070_Lvgl_DisplayDriver::drawRaceScreenMenu() {
    DEBUG_PRINT_METHOD();
    // Deprecated: This method is kept for backward compatibility
    // Use drawRaceReady() instead
    DPRINTLN("WARNING: drawRaceScreenMenu is deprecated, use drawRaceReady instead");
    drawRaceReady();
}

void ESP32_8048S070_Lvgl_DisplayDriver::drawRaceReady() {
    DEBUG_PRINT_METHOD();
    DPRINTLN("Entering drawRaceReady()");
    
    // Ensure the RaceReadyScreen instance exists and is properly initialized
    if (!race_ready_screen_) {
        DPRINTLN("Creating new RaceReadyScreen instance");
        race_ready_screen_ = new RaceReadyScreen();
        if (race_ready_screen_) {
            DPRINTLN("RaceReadyScreen instance created successfully");
        } else {
            DPRINTLN("ERROR: Failed to create RaceReadyScreen instance");
            return;
        }
    } else {
        DPRINTLN("Using existing RaceReadyScreen instance");
    }
    
    // Show the RaceReady screen
    if (race_ready_screen_) {
        DPRINTLN("Showing RaceReady screen");
        
        // Call Show() which will handle the screen loading
        race_ready_screen_->Show();
        
        // Force an immediate update
        DPRINTLN("Forcing screen refresh");
        lv_refr_now(nullptr);
        
        DPRINTLN("Race ready screen shown and updated successfully");
    } else {
        DPRINTLN("ERROR: RaceReadyScreen instance is null");
    }
}

// Update the countdown display with the specified step
void ESP32_8048S070_Lvgl_DisplayDriver::updateCountdownDisplay(int currentStep) {
    DEBUG_PRINT_METHOD();
    if (!ui_CountdownScreen) {
        DPRINTLN("Countdown screen not created yet, creating now");
        createCountdownScreen();
    }
    
    // Update the countdown text
    lv_obj_t * countdown_label = lv_obj_get_child(ui_CountdownScreen, 0);
    if (countdown_label) {
        if (currentStep > 0) {
            lv_label_set_text_fmt(countdown_label, "%d", currentStep);
            lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFF0000), 0);
        } else {
            lv_label_set_text(countdown_label, "GO!");
            lv_obj_set_style_text_color(countdown_label, lv_color_hex(0x00FF00), 0);
        }
        lv_obj_center(countdown_label);
    }
    
    // Update the countdown display without changing screens
    // We don't load a new screen here as countdown is part of RaceReady
    
    DPRINTF("Countdown display updated with step: %d\n", currentStep);
}

// Draw the race active screen with the specified race mode
void ESP32_8048S070_Lvgl_DisplayDriver::drawRaceActive(RaceMode raceMode) {
    DEBUG_PRINT_METHOD();
    DPRINTF("ESP32_8048S070_Lvgl_DisplayDriver::drawRaceActive(raceMode: %d)\n", static_cast<int>(raceMode));
    
    // Create the race active screen if it doesn't exist
    if (!ui_RaceActiveScreen) {
        createRaceActiveScreen();
    }
    
    lv_scr_load(ui_RaceActiveScreen);
    
    static RaceScreen raceScreen(8); // Always create with 8 lanes initially
    static bool raceScreenInitialized = false;
    
    if (!raceScreenInitialized) {
        raceScreenInitialized = true;
        lv_obj_set_user_data(ui_RaceActiveScreen, &raceScreen);
    } else {
        // Update the number of lanes if needed
        raceScreen.SetNumLanes(8);
    }
    
    // Store a reference to the race screen for use in updateRaceData
    _activeRaceScreen = &raceScreen;
    
    // Store the previous race mode to detect changes
    // Currently unused - will be used for mode change detection in the future
    // static RaceMode previousRaceMode = RaceMode::LAPS; // Default to LAPS
    // previousRaceMode = raceMode;
    
    raceScreen.SetRaceMode(raceMode);
    
    raceScreen.Show();
    
    // In a proper architecture, we shouldn't be directly accessing RaceModule or RaceDataStub here
    // The DisplayManager should be passing race data to us through updateRaceData()
    // For testing purposes, we'll just set up the screen without any data
    // The main.cpp will call DisplayManager.updateRaceData() with the stub data
    
    DPRINTLN("Setting up RaceActive screen - waiting for data through updateRaceData()");
    
    // For testing purposes, we'll always assume we're in the Active state
    RaceState currentState = RaceState::Active;
    DPRINTF("Current race state: %d\n", static_cast<int>(currentState));
    
  
    // Initialize with empty race data - proper data will come through updateRaceData()
    std::vector<RaceLaneData> currentRaceData;
    DPRINTF("Initial race data size: %d\n", currentRaceData.size());
    
    // Log detailed lane data if available
    if (!currentRaceData.empty()) {
        DPRINTLN("Lane Data:");
        for (const auto& lane : currentRaceData) {
            DPRINTF("  Lane %d: %s, Laps: %d/%d, Enabled: %s, Finished: %s\n",
                  lane.laneId,
                  lane.racerName.c_str(),
                  lane.currentLap,
                  lane.totalLaps,
                  lane.enabled ? "true" : "false",
                  lane.finished ? "true" : "false");
        }
        
        // Always update UI with current data
        DPRINTF("Updating UI with %d lanes of race data\n", currentRaceData.size());
        raceScreen.GetActiveRaceModeUI()->UpdateRaceData(currentRaceData);
    } else {
        DPRINTLN("No race data available - initializing with empty data");
        // Initialize with empty data to show the UI structure
        std::vector<RaceLaneData> emptyRaceData;
        raceScreen.GetActiveRaceModeUI()->UpdateRaceData(emptyRaceData);
        
        // Commented out along with the race preparation code
        /*
        // If we just tried to prepare a race but still have no data, log an error
        if (needsRacePrep) {
            DPRINTLN("ERROR: Failed to prepare default race!");
        }
        */
    }
    
    raceScreen.Update();
}

// updateRaceData is already implemented elsewhere in the file

void ESP32_8048S070_Lvgl_DisplayDriver::startLightSequence() {
    DEBUG_PRINT_METHOD();
    DPRINTLN("ESP32_8048S070_Lvgl_DisplayDriver::startLightSequence()");
    
    // Make sure we're on the RaceReadyScreen
    if (lv_scr_act() != ui_RaceReadyScreen) {
        DPRINTLN("Warning: Not on RaceReadyScreen, switching to it first");
        drawRaceReady();
    }
    
    // Access the RaceReadyScreen instance and start the light sequence
    if (race_ready_screen_) {
        // Set up the callback to update the DisplayManager with countdown steps
        race_ready_screen_->SetCountdownStepCallback([](int step) {
            // Update the DisplayManager with the current countdown step
            DisplayManager::getInstance().showCountdown(step, step == 0);
            DPRINTLN("Countdown step: " + String(step));
        });
        
        race_ready_screen_->StartRedSequence();
        DPRINTLN("Light sequence started using stored instance");
    } else {
        DPRINTLN("ERROR: RaceReadyScreen instance is null");
        // Create it if it doesn't exist
        race_ready_screen_ = new RaceReadyScreen();
        
        // Set up the callback to update the DisplayManager with countdown steps
        race_ready_screen_->SetCountdownStepCallback([](int step) {
            // Update the DisplayManager with the current countdown step
            DisplayManager::getInstance().showCountdown(step, step == 0);
            DPRINTLN("Countdown step: " + String(step));
        });
        
        race_ready_screen_->StartRedSequence();
        DPRINTLN("Light sequence started with new instance");
    }
    
    DPRINTLN("Light sequence started");
}

void ESP32_8048S070_Lvgl_DisplayDriver::createConfigScreen() {
    DEBUG_PRINT_METHOD();
    DPRINTLN("Creating Config Screen...");
    
    // Create the ConfigScreen instance if it doesn't exist
    if (!config_screen_) {
        config_screen_ = new ConfigScreen();
        if (!config_screen_) {
            DEBUG_ERROR("FAILED: Could not create ConfigScreen instance");
            return;
        }
        DPRINTLN("  - ConfigScreen instance created successfully");
    } else {
        DPRINTLN("  - ConfigScreen instance already exists");
    }
    
    // Store the screen in the class member for later use
    ui_ConfigScreen = config_screen_->getScreen();
    if (!ui_ConfigScreen) {
        DEBUG_ERROR("FAILED: Could not get ConfigScreen LVGL screen");
        return;
    }
    
    DPRINTLN("  - Config Screen created successfully");
}

void ESP32_8048S070_Lvgl_DisplayDriver::event_cb_main_menu_config_button(lv_event_t * e) {
    DEBUG_PRINT_METHOD();
    DPRINTLN("Config Menu button clicked");
    InputEvent event;
    event.command = InputCommand::EnterConfig; // Existing command
    event.value = 0;
    event.sourceId = (int)InputSourceId::TOUCH;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = getDefaultTargetForCommand(event.command);
    GT911_TouchInput::queueSystemInputEvent(event);
}

void ESP32_8048S070_Lvgl_DisplayDriver::event_cb_race_menu_start_button(lv_event_t* e) {
    DEBUG_PRINT_METHOD();
    DPRINTLN("Start Race button clicked from Race Menu");
    InputEvent event;
    event.command = InputCommand::StartCountdown;  // Changed from StartRace to StartCountdown
    event.value = 0;
    event.sourceId = (int)InputSourceId::TOUCH;
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = getDefaultTargetForCommand(event.command);
    GT911_TouchInput::queueSystemInputEvent(event);
}

void ESP32_8048S070_Lvgl_DisplayDriver::event_cb_race_menu_return_button(lv_event_t * e) {
    DEBUG_PRINT_METHOD();
    if (e->code != LV_EVENT_CLICKED) return;
    
    // Get the display driver instance
    ESP32_8048S070_Lvgl_DisplayDriver* driver = static_cast<ESP32_8048S070_Lvgl_DisplayDriver*>(lv_event_get_user_data(e));
    if (!driver) return;
    
    // Create InputEvent for returning to main menu
    InputEvent event;
    event.command = InputCommand::ChangeMode;
    event.value = 0; // Value for main menu
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = InputTarget::Race;
    
    // Queue the input event through the input manager
    GT911_TouchInput::queueSystemInputEvent(event);
}

void ESP32_8048S070_Lvgl_DisplayDriver::event_cb_main_menu_race_button(lv_event_t * e) {
    DEBUG_PRINT_METHOD();
    if (e->code != LV_EVENT_CLICKED) return;
    
    // Get the display driver instance
    ESP32_8048S070_Lvgl_DisplayDriver* driver = static_cast<ESP32_8048S070_Lvgl_DisplayDriver*>(lv_event_get_user_data(e));
    if (!driver) return;
    
    // Create InputEvent for race menu
    InputEvent event;
    event.command = InputCommand::EnterRaceReady;
    event.value = 0; // No specific value needed
    event.sourceId = static_cast<int>(InputSourceId::TOUCH);
    event.timestamp = TimeManager::GetInstance().GetCurrentTimeMs();
    event.target = InputTarget::Race;
    
    // Queue the input event through the input manager
    GT911_TouchInput::queueSystemInputEvent(event);
}

void ESP32_8048S070_Lvgl_DisplayDriver::lvgl_display_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    DEBUG_PRINT_METHOD();
    ESP32_8048S070_Lvgl_DisplayDriver* driver = (ESP32_8048S070_Lvgl_DisplayDriver*)disp_drv->user_data;
    
    if (!driver || !driver->_gfx) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    // Start a new pixel data transfer
    driver->_gfx->startWrite();
    
    // Draw the pixels directly to the display using draw16bitBeRGBBitmap
    // This is the correct method for 16-bit color depth displays
    driver->_gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t*)color_p, w, h);
    
    // End the transfer
    driver->_gfx->endWrite();
    
    // Inform LVGL that flushing is done
    lv_disp_flush_ready(disp_drv);
}

// Create the main menu screen
void ESP32_8048S070_Lvgl_DisplayDriver::createMainMenuScreen() {
    DEBUG_PRINT_METHOD();
    if (ui_MainMenuScreen) {
        lv_obj_del(ui_MainMenuScreen);
        ui_MainMenuScreen = nullptr;
    }
    
    // Create the main menu screen with black background
    ui_MainMenuScreen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_MainMenuScreen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_MainMenuScreen, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_MainMenuScreen, 0, LV_PART_MAIN);
    
    // Add title
    lv_obj_t * title_label = lv_label_create(ui_MainMenuScreen);
    lv_label_set_text(title_label, "MAIN MENU");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_24, 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 20);
    
    // Style for buttons
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_hex(0x333333));
    lv_style_set_radius(&btn_style, 10);
    lv_style_set_border_width(&btn_style, 0);
    
    // Add race button
    lv_obj_t * btn_race = lv_btn_create(ui_MainMenuScreen);
    lv_obj_add_style(btn_race, &btn_style, 0);
    lv_obj_set_size(btn_race, 200, 50);
    lv_obj_align(btn_race, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_event_cb(btn_race, event_cb_main_menu_race_button, LV_EVENT_CLICKED, this);
    
    lv_obj_t * btn_race_label = lv_label_create(btn_race);
    lv_label_set_text(btn_race_label, "Race");
    lv_obj_set_style_text_color(btn_race_label, lv_color_white(), 0);
    lv_obj_center(btn_race_label);
    
    // Add config button
    lv_obj_t * btn_config = lv_btn_create(ui_MainMenuScreen);
    lv_obj_add_style(btn_config, &btn_style, 0);
    lv_obj_set_size(btn_config, 200, 50);
    lv_obj_align(btn_config, LV_ALIGN_CENTER, 0, 30);
    lv_obj_add_event_cb(btn_config, event_cb_main_menu_config_button, LV_EVENT_CLICKED, this);
    
    lv_obj_t * btn_config_label = lv_label_create(btn_config);
    lv_label_set_text(btn_config_label, "Configuration");
    lv_obj_set_style_text_color(btn_config_label, lv_color_white(), 0);
    lv_obj_center(btn_config_label);
}

// Create the race ready screen
void ESP32_8048S070_Lvgl_DisplayDriver::createRaceReadyScreen() {
    DEBUG_PRINT_METHOD();
    
    // This method is now just a wrapper for drawRaceReady to maintain backward compatibility
    DPRINTLN("createRaceReadyScreen() called - delegating to drawRaceReady()");
    drawRaceReady();
}

// Create the race active screen
void ESP32_8048S070_Lvgl_DisplayDriver::createRaceActiveScreen() {
    DEBUG_PRINT_METHOD();
    if (ui_RaceActiveScreen) {
        lv_obj_del(ui_RaceActiveScreen);
        ui_RaceActiveScreen = nullptr;
    }
    
    // Create the race active screen as a simple black container
    // The RaceScreen class will add all necessary UI elements
    ui_RaceActiveScreen = lv_obj_create(NULL);
    lv_obj_set_size(ui_RaceActiveScreen, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(ui_RaceActiveScreen, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_pad_all(ui_RaceActiveScreen, 0, 0);
}

// Create the countdown screen
void ESP32_8048S070_Lvgl_DisplayDriver::createCountdownScreen() {
    DEBUG_PRINT_METHOD();
    if (ui_CountdownScreen) {
        lv_obj_del(ui_CountdownScreen);
        ui_CountdownScreen = nullptr;
    }
    
    // Create the countdown screen
    ui_CountdownScreen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(ui_CountdownScreen, lv_color_hex(0x000000), LV_PART_MAIN);
    
    // Add countdown label
    lv_obj_t * countdown_label = lv_label_create(ui_CountdownScreen);
    lv_label_set_text(countdown_label, "3");
    lv_obj_set_style_text_color(countdown_label, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(countdown_label, &lv_font_montserrat_48, 0);
    lv_obj_center(countdown_label);
}

// Note: The createRaceActiveScreen method is already defined above

// Note: createRaceScreenMenu_common method has been removed as it's no longer needed in the new design

void ESP32_8048S070_Lvgl_DisplayDriver::drawStats() {
    DEBUG_PRINT_METHOD();
    
    // Create the stats screen if it doesn't exist
    if (!_statsScreen) {
        _statsScreen = new StatsScreen();
        if (!_statsScreen) {
            DPRINTLN("ERROR: Failed to create StatsScreen instance");
            return;
        }
        DPRINTLN("Created StatsScreen instance");
    }
    
    // Clear the display
    clear();
    
    // Show the stats screen
    _statsScreen->Show();
    
    // Force an immediate update
    lv_timer_handler();
    update();
    
    DPRINTLN("Stats screen shown");
}

void ESP32_8048S070_Lvgl_DisplayDriver::drawPause() {
    DEBUG_PRINT_METHOD();
    
    // Create the pause screen if it doesn't exist
    static PauseScreen* pauseScreen = nullptr;
    if (!pauseScreen) {
        pauseScreen = new PauseScreen();
        if (!pauseScreen) {
            DPRINTLN("ERROR: Failed to create PauseScreen instance");
            return;
        }
        DPRINTLN("Created PauseScreen instance");
    }
    
    // Clear the display
    clear();
    
    // Show the pause screen
    pauseScreen->Show();
    
    // Force an immediate update
    lv_timer_handler();
    update();
    
    DPRINTLN("Pause screen shown");
}

void ESP32_8048S070_Lvgl_DisplayDriver::drawStop() {
    DEBUG_PRINT_METHOD();
    
    // Create the stop screen if it doesn't exist
    static StopScreen* stopScreen = nullptr;
    if (!stopScreen) {
        stopScreen = new StopScreen();
        if (!stopScreen) {
            DPRINTLN("ERROR: Failed to create StopScreen instance");
            return;
        }
        DPRINTLN("Created StopScreen instance");
    }
    
    // Clear the display
    clear();
    
    // Show the stop screen
    stopScreen->Show();
    
    // Force an immediate update
    lv_timer_handler();
    update();
    
    DPRINTLN("Stop screen shown");
}
