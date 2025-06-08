#pragma once

#include "DisplayModule.h"

/**
 * @brief Serial display implementation
 * 
 * This class implements the IBaseDisplay interface for Serial output.
 * It handles formatting and displaying text on the Serial console.
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
     * This is a no-op for Serial display.
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
    
private:
    bool _initialized;
    int _width;
    int _height;
};
