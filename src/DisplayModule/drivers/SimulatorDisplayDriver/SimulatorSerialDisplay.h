#pragma once

#include "DisplayModule/DisplayModule.h"
#include "SerialBridge.h"

/**
 * @brief Serial display implementation for the simulator
 * 
 * This class implements the IBaseDisplay interface for Serial output in the simulator.
 * It uses the SerialBridge to communicate with a real serial port.
 */
class SimulatorSerialDisplay : public IBaseDisplay {
public:
    /**
     * @brief Construct a new Simulator Serial Display object
     */
    SimulatorSerialDisplay();
    
    /**
     * @brief Destroy the Simulator Serial Display object
     */
    virtual ~SimulatorSerialDisplay();
    
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
     * Processes any incoming serial data.
     */
    void update() override;
    
    /**
     * @brief Clear the display
     * 
     * Sends a clear screen command to the serial port.
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
     * @brief Get the display type
     * 
     * @return DisplayType The display type (Serial)
     */
    DisplayType getDisplayType() const override;
    
    /**
     * @brief Configure the serial port
     * 
     * @param portName The name of the serial port (e.g., "COM3")
     * @param baudRate The baud rate (e.g., 115200)
     * @return true if configuration was successful
     * @return false if configuration failed
     */
    bool configurePort(const std::string& portName, int baudRate);
    
private:
    bool _initialized;
    std::string _portName;
    int _baudRate;
};
