#pragma once

#include "DisplayModule/DisplayModule.h"
#include "SDLBackend.h"
#include <string>

/**
 * @brief Graphical display implementation for the simulator
 * 
 * This class implements the IGraphicalDisplay interface for the simulator.
 * It uses the SDL backend to render the display.
 */
class SimulatorDisplay : public IGraphicalDisplay {
public:
    /**
     * @brief Construct a new Simulator Display object
     */
    SimulatorDisplay();
    
    /**
     * @brief Destroy the Simulator Display object
     */
    virtual ~SimulatorDisplay();
    
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
     * This should be called regularly in the main loop.
     */
    void update() override;
    
    /**
     * @brief Clear the display
     */
    void clear() override;
    
    /**
     * @brief Print a message to the display
     * 
     * @param message The message to print
     * @param newLine Whether to add a newline
     */
    void print(const String& message, bool newLine = true) override;
    
    /**
     * @brief Print a formatted message to the display
     * 
     * @param format The format string
     * @param ... The format arguments
     */
    void printf(const char* format, ...) override;
    
    /**
     * @brief Set cursor position
     * 
     * @param x X coordinate
     * @param y Y coordinate
     */
    void setCursor(int x, int y) override;
    
    /**
     * @brief Set text color
     * 
     * @param color Color value
     */
    void setTextColor(uint32_t color) override;
    
    /**
     * @brief Set text size
     * 
     * @param size Text size multiplier
     */
    void setTextSize(uint8_t size) override;
    
    /**
     * @brief Draw a pixel
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Color value
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    /**
     * @brief Draw a line
     * 
     * @param x0 Start X coordinate
     * @param y0 Start Y coordinate
     * @param x1 End X coordinate
     * @param y1 End Y coordinate
     * @param color Color value
     */
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
    
    /**
     * @brief Draw a rectangle
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Color value
     */
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    
    /**
     * @brief Fill a rectangle
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param w Width
     * @param h Height
     * @param color Color value
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
    
    /**
     * @brief Draw a circle
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param r Radius
     * @param color Color value
     */
    void drawCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;
    
    /**
     * @brief Fill a circle
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param r Radius
     * @param color Color value
     */
    void fillCircle(int16_t x, int16_t y, int16_t r, uint16_t color) override;
    
    /**
     * @brief Draw a triangle
     * 
     * @param x0 First point X coordinate
     * @param y0 First point Y coordinate
     * @param x1 Second point X coordinate
     * @param y1 Second point Y coordinate
     * @param x2 Third point X coordinate
     * @param y2 Third point Y coordinate
     * @param color Color value
     */
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
    
    /**
     * @brief Fill a triangle
     * 
     * @param x0 First point X coordinate
     * @param y0 First point Y coordinate
     * @param x1 Second point X coordinate
     * @param y1 Second point Y coordinate
     * @param x2 Third point X coordinate
     * @param y2 Third point Y coordinate
     * @param color Color value
     */
    void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) override;
    
    /**
     * @brief Draw text
     * 
     * @param x X coordinate
     * @param y Y coordinate
     * @param text Text to draw
     * @param color Color value
     * @param size Text size
     */
    void drawText(int16_t x, int16_t y, const String& text, uint16_t color, uint8_t size = 1) override;
    
    /**
     * @brief Get the display type
     * 
     * @return DisplayType The display type
     */
    DisplayType getDisplayType() const override;
    
    /**
     * @brief Get the display width
     * 
     * @return int Display width in pixels
     */
    int getWidth() const override;
    
    /**
     * @brief Get the display height
     * 
     * @return int Display height in pixels
     */
    int getHeight() const override;
    
private:
    bool _initialized;
    int _cursorX;
    int _cursorY;
    uint32_t _textColor;
    uint8_t _textSize;
    
    // Log a message to the console
    void log(const std::string& message);
};
