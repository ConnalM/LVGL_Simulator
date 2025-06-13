#ifndef LV_CONF_INCLUDE_SIMPLE
#define LV_CONF_INCLUDE_SIMPLE
#endif

#include <lvgl.h>
#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <string>
#include <chrono>
#include <iomanip>
#include <csignal>
#include <cstdlib>

// Conditional includes based on environment
#ifdef SIMULATOR
// Simulator-specific includes
// #include <../.pio/libdeps/native/lvgl/src/misc/lv_timer.h> // Likely not needed for headless
// #include "DisplayModule/drivers/SimulatorDisplayDriver/SDLBackend.h" // UI Removed
// #include "DisplayModule/drivers/SimulatorDisplayDriver/SimulatorDisplayAdapter.h" // UI Removed
// #include "DisplayModule/DisplayManager.h" // UI Removed
// #include "DisplayModule/DisplayFactory.h" // UI Removed
// #include "InputModule/drivers/SimulatorInputDriver/SDLInputHandler.h" // UI Removed
#include <thread> // For std::this_thread::sleep_for
#else
// Production-specific includes
#include <Arduino.h>
// Add any production-specific headers here
// #include "DisplayModule/drivers/ESP32DisplayDriver.h"
// #include "InputModule/drivers/ESP32InputDriver.h"
#endif

#include "common/ArduinoCompat.h"

// LVGL display buffer (No longer needed for headless simulator)
// static lv_disp_draw_buf_t draw_buf;

#ifdef SIMULATOR
// Simulator display buffer sizes (No longer needed for headless simulator)
// static lv_color_t buf1[DISP_HOR_RES * 10]; 
// static lv_color_t buf2[DISP_HOR_RES * 10];
// DISP_HOR_RES and DISP_VER_RES might not be defined if SDLBackend.h is removed.
// If they are needed for non-UI reasons, their definition source needs to be ensured.
// Declare global Serial for simulator
TerminalSerial Serial;
#else
// Production display buffer sizes - adjust as needed
#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240
static lv_color_t buf1[DISPLAY_WIDTH * 10];
static lv_color_t buf2[DISPLAY_WIDTH * 10];
// Declare global Serial for production
HardwareSerial Serial(0);
#endif

// Display flushing callback
static void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
#ifdef SIMULATOR
    // Simulator-specific display flush (UI Removed - this part should not be called in headless)
    // if (SDLBackend::updateTexture(area, color_p))
    // {
    //     // Render the texture to the screen
    //     SDLBackend::render();
    // }
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

// Global file stream for logging
std::ofstream logFile;

#ifdef SIMULATOR
// Mouse cursor read function for LVGL (No longer needed for headless simulator)
/*
static void mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    // Get mouse position
    // int x, y;
    // uint32_t buttons = SDL_GetMouseState(&x, &y); // SDL call removed

    // Set the coordinates
    // data->point.x = x;
    // data->point.y = y;

    // Set the state - pressed if left button is down
    // data->state = (buttons & SDL_BUTTON(1)) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED; // SDL call removed
}
*/
#endif

// Global file stream for logging
// std::ofstream logFile;

// Exit handler to log exit code and reason
static void exitHandler()
{
    // We can't reliably get the exit code here, so we'll just log that we're exiting
    if (logFile.is_open())
    {
        logFile << "Application exiting" << std::endl;
        logFile.close();
    }

    printf("Application exiting\n");
}

// Signal handler for abnormal terminations
static void signalHandler(int signal)
{
    const char *signalName = "Unknown";

    switch (signal)
    {
    case SIGABRT:
        signalName = "SIGABRT";
        break;
    case SIGFPE:
        signalName = "SIGFPE";
        break;
    case SIGILL:
        signalName = "SIGILL";
        break;
    case SIGINT:
        signalName = "SIGINT";
        break;
    case SIGSEGV:
        signalName = "SIGSEGV";
        break;
    case SIGTERM:
        signalName = "SIGTERM";
        break;
    }

    if (logFile.is_open())
    {
        logFile << "Signal received: " << signalName << " (" << signal << ")" << std::endl;
        logFile.close();
    }

    printf("Signal received: %s (%d)\n", signalName, signal);
    exit(signal);
}

// Function to log messages to both console and file
// This function now also uses Serial.printf for simulator to ensure output via TerminalSerial
static void log_message(const char *format, ...)
{
    auto now = std::chrono::system_clock::now();
    auto time_val = std::chrono::system_clock::to_time_t(now);
    std::tm tm_val = *std::localtime(&time_val);
    std::stringstream timestamp;
    timestamp << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

#ifdef SIMULATOR
    Serial.printf("%s: %s\n", timestamp.str().c_str(), buffer);
#else
    // Print to console for production or if Serial is not yet ready in early simulator stages
    printf("%s: %s\n", timestamp.str().c_str(), buffer);
#endif

    if (logFile.is_open())
    {
        logFile << timestamp.str() << ": " << buffer << std::endl;
        logFile.flush();
    }
}

#ifdef SIMULATOR
// Main function for simulator
int main(int argc, char *argv[])
{
    std::atexit(exitHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE, signalHandler);
    std::signal(SIGILL, signalHandler);
    std::signal(SIGINT, signalHandler);
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGTERM, signalHandler);

    logFile.open("simulator_log.txt");
    if (!logFile.is_open())
    {
        // Use Serial here as log_message itself might try to use logFile
        Serial.println("ERROR: Failed to open log file"); 
    }

    log_message("Headless Simulator starting...");
    Serial.println("Hello from Virtual Serial! This is the Headless Simulator.");
    Serial.println("Type 'quit' to exit.");

    bool quit_flag = false;
    unsigned long last_report_time = millis(); // Using ArduinoCompat millis()
    const unsigned long report_interval = 15000; // Report every 15 seconds
    unsigned long loop_counter = 0;

    log_message("Entering main loop...");

    try {
        while (!quit_flag)
        {
            if (Serial.available() > 0)
            {
                String incomingMessage = Serial.readLine();
                // Trim leading/trailing whitespace from incomingMessage (std::string)
                const std::string WHITESPACE_CHARS = " \n\r\t\f\v";
                size_t start_pos = incomingMessage.find_first_not_of(WHITESPACE_CHARS);
                if (std::string::npos == start_pos) { // String is all whitespace or empty
                    incomingMessage.clear();
                } else {
                    size_t end_pos = incomingMessage.find_last_not_of(WHITESPACE_CHARS);
                    incomingMessage = incomingMessage.substr(start_pos, end_pos - start_pos + 1);
                }
                if (incomingMessage.length() > 0) {
                    log_message("Received command: '%s'", incomingMessage.c_str());
                    if (incomingMessage == "quit")
                    {
                        Serial.println("Quit command received. Exiting simulator...");
                        quit_flag = true;
                    }
                    else
                    {
                        Serial.print("Echo: ");
                        Serial.println(incomingMessage);
                    }
                }
            }

            // Periodically report that the simulator is still running
            unsigned long current_m = millis();
            if (current_m - last_report_time >= report_interval)
            {
                Serial.printf("Still running... Loop count: %lu, Uptime: %lu ms\n", loop_counter, current_m);
                last_report_time = current_m;
            }
            
            // lv_timer_handler(); // Not needed for headless if no LVGL timers are active
            // If ArduinoCompat or other systems create LVGL timers, this might be needed.
            // For a truly headless system with no LVGL UI or timers, this can be omitted.

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Prevent high CPU usage
            loop_counter++;
        }
    }
    catch (const std::exception& e) {
        log_message("ERROR: Exception in main loop: %s", e.what());
        Serial.printf("ERROR: Exception in main loop: %s\n", e.what());
    }
    catch (...) {
        log_message("ERROR: Unknown exception in main loop.");
        Serial.println("ERROR: Unknown exception in main loop.");
    }

    log_message("Exiting headless simulator main function.");
    // logFile is closed by exitHandler registered with atexit
    // SDLBackend::cleanup(); // UI Removed
    return 0;
}
#else
// Arduino setup function for production
void setup()
{
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
void loop()
{
    // Call LVGL task handler
    lv_timer_handler();

    // Production-specific loop code
    // Example: processInputs();

    // Small delay to prevent high CPU usage
    delay(5);
}
#endif
