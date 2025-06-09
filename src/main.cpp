#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif
#include <lvgl.h>
#include <stdio.h>
#include <stdbool.h>

// Conditional includes based on environment
#ifdef SIMULATOR
    // Simulator-specific includes
    #include <../.pio/libdeps/native/lvgl/src/misc/lv_timer.h>
    #include "DisplayModule/drivers/SimulatorDisplayDriver/SDLBackend.h"
    #include "DisplayModule/drivers/SimulatorDisplayDriver/SimulatorDisplayAdapter.h"
    #include "DisplayModule/DisplayManager.h"
    #include "DisplayModule/DisplayFactory.h"
    #include "InputModule/drivers/SimulatorInputDriver/SDLInputHandler.h"
#else
    // Production-specific includes
    #include <Arduino.h>
    // Add any production-specific headers here
    // #include "DisplayModule/drivers/ESP32DisplayDriver.h"
    // #include "InputModule/drivers/ESP32InputDriver.h"
#endif

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;

#ifdef SIMULATOR
    // Simulator display buffer sizes
    static lv_color_t buf1[DISP_HOR_RES * 10];
    static lv_color_t buf2[DISP_HOR_RES * 10];
#else
    // Production display buffer sizes - adjust as needed
    #define DISPLAY_WIDTH 320
    #define DISPLAY_HEIGHT 240
    static lv_color_t buf1[DISPLAY_WIDTH * 10];
    static lv_color_t buf2[DISPLAY_WIDTH * 10];
#endif

// Display flushing callback
static void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p) {
#ifdef SIMULATOR
    // Simulator-specific display flush
    if (SDLBackend::updateTexture(area, color_p)) {
        // Render the texture to the screen
        SDLBackend::render();
    }
#else
    // Production-specific display flush
    // Add production display flush code here
    // Example for ESP32 with ILI9341:
    // uint32_t w = (area->x2 - area->x1 + 1);
    // uint32_t h = (area->y2 - area->y1 + 1);
    // tft.setAddrWindow(area->x1, area->y1, w, h);
    // tft.pushColors((uint16_t*)color_p, w * h, true);
#endif
    
    // Inform LVGL that flushing is done
    lv_disp_flush_ready(disp);
}

#include <fstream>
#include <ctime>

// Forward declaration for log_message function
void log_message(const char* format, ...);

#ifdef SIMULATOR
// Mouse cursor read function for LVGL
static void mouse_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data) {
    // Get mouse position
    int x, y;
    uint32_t buttons = SDL_GetMouseState(&x, &y);
    
    // Set the coordinates
    data->point.x = x;
    data->point.y = y;
    
    // Set the state - pressed if left button is down
    data->state = (buttons & SDL_BUTTON(1)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    
    // Log mouse state for debugging
    static bool last_state = false;
    bool current_state = data->state == LV_INDEV_STATE_PRESSED;
    if (current_state != last_state) {
        log_message("Mouse state changed: %s at position (%d,%d)", 
                  current_state ? "PRESSED" : "RELEASED", x, y);
        last_state = current_state;
    }
}
#endif

// Global file stream for logging
std::ofstream logFile;

// Custom log function that writes to both console and file
void log_message(const char* format, ...) {
    // Get current time
    time_t now = time(0);
    char timeBuffer[80];
    struct tm timeInfo;
    localtime_s(&timeInfo, &now);
    strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
    
    // Format the message
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Write to console
    printf("%s: %s\n", timeBuffer, buffer);
    
    // Write to file if open
    if (logFile.is_open()) {
        logFile << timeBuffer << ": " << buffer << std::endl;
        logFile.flush(); // Ensure it's written immediately
    }
}

#ifdef SIMULATOR
// Main function for simulator
int main(int argc, char** argv) {
    // Open log file
    logFile.open("simulator_log.txt");
    if (!logFile.is_open()) {
        printf("Failed to open log file\n");
    }
    
    log_message("Simulator starting...");
    
    // Initialize SDL backend
    if (!SDLBackend::initialize()) {
        log_message("Failed to initialize SDL backend");
        return 1;
    }
    log_message("SDL backend initialized successfully");
    
    // Initialize LVGL
    lv_init();
    log_message("LVGL initialized");

    // Initialize the display buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISP_HOR_RES * 10);
    log_message("Display buffer initialized");

    // Initialize the display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.full_refresh = 1;  // Enable full screen refresh
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    log_message("Display driver registered");
    
    // Set the display to the default screen
    lv_disp_set_default(disp);
    
    // Initialize the mouse input device
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = mouse_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv);
    log_message("Mouse input device registered");

    // Initialize DisplayManager and DisplayFactory
    DisplayManager& displayManager = DisplayManager::getInstance();
    DisplayFactory& displayFactory = DisplayFactory::getInstance();
    log_message("DisplayManager and DisplayFactory initialized");
    
    // Create and initialize the LCD display (which will be our SimulatorDisplayAdapter)
    IGraphicalDisplay* display = displayFactory.createGraphicalDisplay(DisplayType::LCD);
    if (display) {
        display->initialize();
        displayManager.registerDisplay(display);
        log_message("SimulatorDisplayAdapter registered with DisplayManager");
        
        // Show the race screen through DisplayManager
        displayManager.showRaceActive(RaceMode::TIME_TRIAL);
        log_message("Race screen shown through DisplayManager");
    } else {
        log_message("ERROR: Failed to create SimulatorDisplayAdapter");
    }
    
    // Create a timer to update the display every 100ms
    static lv_timer_t* displayUpdateTimer = nullptr;
    displayUpdateTimer = lv_timer_create([](lv_timer_t* timer) {
        DisplayManager* manager = static_cast<DisplayManager*>(timer->user_data);
        if (manager) {
            manager->update();
        }
    }, 100, &displayManager);
    log_message("Race screen update timer created");
    
    // Create a simple keep-alive timer that refreshes the screen periodically
    // This prevents the simulator from exiting prematurely
    lv_timer_t* keepAliveTimer = lv_timer_create([](lv_timer_t* timer) {
        static int counter = 0;
        counter++;
        // Just trigger a screen refresh to keep the application alive
        lv_obj_invalidate(lv_scr_act());
    }, 5000, NULL);  // Run every 5 seconds
    
    // Main loop
    bool quit = false;
    int loopCount = 0;
    uint32_t startTime = SDL_GetTicks();
    log_message("Entering main loop at time: %u ms", startTime);
    
    // Add a watchdog timer to detect if we're about to exit
    lv_timer_t* watchdogTimer = lv_timer_create([](lv_timer_t* timer) {
        uint32_t* startTimePtr = (uint32_t*)timer->user_data;
        uint32_t elapsedTime = SDL_GetTicks() - *startTimePtr;
        log_message("Watchdog timer fired at %u ms (elapsed: %u ms)", SDL_GetTicks(), elapsedTime);
    }, 1000, &startTime);  // Check every second
    
    while (!quit) {
        // Process SDL events
        quit = SDLInputHandler::processEvents();
        if (quit) {
            log_message("Quitting due to SDL event at time: %u ms (elapsed: %u ms)", 
                   SDL_GetTicks(), SDL_GetTicks() - startTime);
        }
        
        // Call LVGL task handler
        lv_timer_handler();
        
        // Small delay to prevent high CPU usage
        SDL_Delay(5);
        
        // Print a message every 1000 loops (about 5 seconds)
        if (++loopCount % 1000 == 0) {
            uint32_t currentTime = SDL_GetTicks();
            log_message("Still running... loop count: %d, time: %u ms (elapsed: %u ms)", 
                   loopCount, currentTime, currentTime - startTime);
            
            // Check if we've been running for more than 20 seconds
            if (currentTime - startTime > 20000) {
                // Check LVGL timer status
                log_message("LVGL timer status after %u ms:", currentTime - startTime);
                log_message("  Active timers: %d", lv_timer_get_idle());
            }
        }
    }
    
    log_message("Main loop exited with quit = %d", quit);
    
    // Cleanup SDL backend
    SDLBackend::cleanup();
    
    log_message("Simulator shutting down");
    
    // Close the log file
    if (logFile.is_open()) {
        logFile.close();
    }
    return 0;
}
#else
// Arduino setup function for production
void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    Serial.println("ESP32 Display Module starting...");
    
    // Initialize production hardware
    // Example: tft.begin();
    
    // Initialize LVGL
    lv_init();
    
    // Initialize the display buffer
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DISPLAY_WIDTH * 10);

    // Initialize the display
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);
    
    // Set the display to the default screen
    lv_disp_set_default(disp);

    // Initialize UI
    // Example: showMainScreen();
    
    Serial.println("Setup complete");
}

// Arduino loop function for production
void loop() {
    // Call LVGL task handler
    lv_timer_handler();
    
    // Production-specific loop code
    // Example: processInputs();
    
    // Small delay to prevent high CPU usage
    delay(5);
}
#endif
