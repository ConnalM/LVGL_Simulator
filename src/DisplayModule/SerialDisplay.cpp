#include "SerialDisplay.h"
#include <stdarg.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "DisplayManager.h"

// Throttle debug prints to once every 5 seconds
static unsigned long lastDebugPrint = 0;
const unsigned long DEBUG_THROTTLE_MS = 5000;

// Helper macro for throttled debug prints
#define DEBUG_PRINT_METHOD() \
    do { \
        static bool firstCall = true; \
        unsigned long now = millis(); \
        if (firstCall || (now - lastDebugPrint > DEBUG_THROTTLE_MS)) { \
            DisplayManager::getInstance().debug(String("[SerialDisplay] ") + __FUNCTION__, "SerialDisplay"); \
            lastDebugPrint = now; \
            firstCall = false; \
        } \
    } while(0)

// Set terminal to raw mode
static void setTerminalRaw() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Restore terminal to original settings
static void restoreTerminal() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

SerialDisplay::SerialDisplay()
    : _initialized(false)
    , _width(80)  // Default width for serial console
    , _height(24) // Default height for serial console
    , _running(false)
{
    DEBUG_PRINT_METHOD();
    // Constructor implementation
}

SerialDisplay::~SerialDisplay() {
    DEBUG_PRINT_METHOD();
    // Signal the input thread to stop
    _running = false;
    
    // If the input thread is joinable, wait for it to finish
    if (_inputThread.joinable()) {
        // Wake up the input thread if it's waiting
        _inputCond.notify_all();
        _lineCond.notify_all();
        
        // Wait for the thread to finish
        _inputThread.join();
    }
    
    // Restore terminal settings
    restoreTerminal();
}

bool SerialDisplay::initialize() {
    DEBUG_PRINT_METHOD();
    // Skip if already initialized
    if (_initialized) {
        DisplayManager::getInstance().debug(F("Already initialized"), "SerialDisplay");
        return true;
    }
    
    DisplayManager::getInstance().debug(F("Initializing..."), "SerialDisplay");
    
    try {
        // Set terminal to raw mode for non-blocking input
        setTerminalRaw();
        
        // Start the input thread
        _running = true;
        _inputThread = std::thread(&SerialDisplay::inputThreadFunc, this);
        
        _initialized = true;
        DisplayManager::getInstance().debug(F("Initialized successfully"), "SerialDisplay");
        return true;
    } catch (const std::exception& e) {
        DisplayManager::getInstance().error(String("Initialization failed: ") + e.what(), "SerialDisplay");
        _running = false;
        restoreTerminal();
        return false;
    }
}

void SerialDisplay::update() {
    DEBUG_PRINT_METHOD();
    // Process any pending input
    // The input is processed in the input thread, so this is a no-op
    // The update method is kept for interface compatibility
}

void SerialDisplay::clear() {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Clear serial by printing newlines
    for (int i = 0; i < 20; i++) {
        Serial.println();
    }
}

void SerialDisplay::print(const String& message, bool newLine) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Print to serial directly
    if (newLine) {
        Serial.println(message);
    } else {
        Serial.print(message);
    }
}

void SerialDisplay::printf(const char* format, ...) {
    DEBUG_PRINT_METHOD();
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Format the string
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Print to serial directly
    Serial.print(buffer);
}

int SerialDisplay::getWidth() const {
    return _width;
}

int SerialDisplay::getHeight() const {
    return _height;
}

DisplayType SerialDisplay::getDisplayType() const {
    return DisplayType::Serial;
}

bool SerialDisplay::available() const {
    std::lock_guard<std::mutex> lock(_inputMutex);
    return !_inputQueue.empty();
}

int SerialDisplay::read() {
    std::unique_lock<std::mutex> lock(_inputMutex);
    
    // Wait for input with a small timeout to prevent deadlocks
    if (_inputQueue.empty()) {
        return -1;
    }
    
    char c = _inputQueue.front();
    _inputQueue.pop();
    return static_cast<unsigned char>(c);
}

int SerialDisplay::readLine(char* buffer, size_t size, bool echo) {
    if (size == 0) return 0;
    
    std::unique_lock<std::mutex> lock(_lineMutex);
    _lineReady = false;
    
    // Wait for a line to be available
    _lineCond.wait(lock, [this] { return _lineReady || !_running; });
    
    if (!_running) {
        return -1;
    }
    
    // Copy the line to the buffer
    size_t len = _lineBuffer.length();
    if (len >= size) {
        len = size - 1; // Leave room for null terminator
    }
    
    std::copy_n(_lineBuffer.begin(), len, buffer);
    buffer[len] = '\0';
    
    if (echo) {
        // Echo the line with newline
        print(buffer, true);
    }
    
    return len;
}

void SerialDisplay::inputThreadFunc() {
    fd_set readfds;
    struct timeval tv;
    char c;
    
    while (_running) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        // Set timeout to 100ms
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        
        int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            // Read a single character
            if (read(STDIN_FILENO, &c, 1) > 0) {
                processInputChar(c);
            }
        } else if (ret < 0) {
            // Error occurred
            DisplayManager::getInstance().error("Error in input thread", "SerialDisplay");
            break;
        }
        
        // Small sleep to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SerialDisplay::processInputChar(char c) {
    // Handle special characters
    if (c == '\r' || c == '\n') {
        // End of line
        std::lock_guard<std::mutex> lock(_lineMutex);
        _lineReady = true;
        _lineCond.notify_one();
    } else if (c == 0x7F || c == 0x08) { // Backspace or delete
        std::lock_guard<std::mutex> lock(_lineMutex);
        if (!_lineBuffer.empty()) {
            _lineBuffer.pop_back();
            // Echo backspace: go back, print space, go back again
            std::cout << "\b \b" << std::flush;
        }
    } else if (c >= 32 && c <= 126) { // Printable ASCII
        std::lock_guard<std::mutex> lock(_lineMutex);
        _lineBuffer.push_back(c);
        // Echo the character
        std::cout << c << std::flush;
    }
    
    // Add to the input queue for single-character reads
    {
        std::lock_guard<std::mutex> lock(_inputMutex);
        _inputQueue.push(c);
        _inputCond.notify_one();
    }
}
