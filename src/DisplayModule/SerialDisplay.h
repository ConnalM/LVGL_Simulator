#pragma once

#include "DisplayModule.h"
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>

/**
 * @brief Serial display implementation
 * 
 * This class implements the IBaseDisplay interface for Serial output.
 * It handles formatting and displaying text on the Serial console.
 * It also provides non-blocking input handling from the terminal.
 */
class SerialDisplay : public IBaseDisplay {
public:
    /**
     * @brief Construct a new Serial Display object
     */
    SerialDisplay();
    
    /**
     * @brief Destroy the Serial Display object
     */
    virtual ~SerialDisplay();
    
    /**
     * @brief Initialize the display
     * 
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    bool initialize() override;
    
    /**
     * @brief Update the display
     * 
     * Processes any pending input events.
     */
    void update() override;
    
    /**
     * @brief Clear the display
     * 
     * Clears the Serial console by printing multiple newlines.
     */
    void clear() override;
    
    /**
     * @brief Print a message to the display
     * 
     * @param message The message to print
     * @param newLine Whether to add a newline after the message (default: true)
     */
    void print(const String& message, bool newLine = true) override;
    
    /**
     * @brief Print a formatted message to the display
     * 
     * @param format The format string
     * @param ... Variable arguments for the format string
     */
    void printf(const char* format, ...) override;
    
    /**
     * @brief Get the width of the display
     * 
     * @return int Display width in columns (default: 80)
     */
    int getWidth() const;
    
    /**
     * @brief Get the height of the display
     * 
     * @return int Display height in rows (default: 24)
     */
    int getHeight() const;
    
    /**
     * @brief Get the display type
     * 
     * @return DisplayType The display type (Serial)
     */
    DisplayType getDisplayType() const override;

    /**
     * @brief Check if input is available
     * 
     * @return true if there is input waiting to be read
     * @return false if no input is available
     */
    bool available() const;

    /**
     * @brief Read a single character from input
     * 
     * @return int The character read, or -1 if no input is available
     */
    int read();

    /**
     * @brief Read a line of input
     * 
     * @param buffer The buffer to store the input
     * @param size The size of the buffer
     * @param echo Whether to echo input to the display (default: true)
     * @return int The number of characters read, or -1 if no input is available
     */
    int readLine(char* buffer, size_t size, bool echo = true);
    
private:
    // Input thread function
    void inputThreadFunc();
    
    // Process a single character of input
    void processInputChar(char c);
    
    bool _initialized;
    int _width;
    int _height;
    
    // Input handling
    std::thread _inputThread;
    std::atomic<bool> _running{false};
    std::queue<char> _inputQueue;
    mutable std::mutex _inputMutex;
    std::condition_variable _inputCond;
    
    // Line buffer for readLine
    std::string _lineBuffer;
    std::mutex _lineMutex;
    std::condition_variable _lineCond;
    bool _lineReady{false};
};
