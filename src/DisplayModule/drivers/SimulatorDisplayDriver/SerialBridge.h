#pragma once

#include <string>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

/**
 * @brief Serial communication bridge for the simulator
 * 
 * This class provides a way to send and receive serial data in the simulator
 * environment, bridging between the simulator and a real serial port.
 */
class SerialBridge {
public:
    /**
     * @brief Get the singleton instance
     * 
     * @return SerialBridge& The singleton instance
     */
    static SerialBridge& getInstance();
    
    /**
     * @brief Initialize the serial bridge
     * 
     * @param portName The name of the serial port (e.g., "COM3")
     * @param baudRate The baud rate (e.g., 115200)
     * @return true if initialization was successful
     * @return false if initialization failed
     */
    bool initialize(const std::string& portName, int baudRate);
    
    /**
     * @brief Close the serial connection
     */
    void close();
    
    /**
     * @brief Send data to the serial port
     * 
     * @param data The data to send
     * @return true if the data was sent successfully
     * @return false if the data could not be sent
     */
    bool send(const std::string& data);
    
    /**
     * @brief Check if data is available to read
     * 
     * @return true if data is available
     * @return false if no data is available
     */
    bool dataAvailable();
    
    /**
     * @brief Read data from the serial port
     * 
     * @return std::string The data read from the serial port
     */
    std::string read();
    
    /**
     * @brief Update the serial bridge
     * 
     * This should be called regularly in the main loop.
     */
    void update();

private:
    SerialBridge();
    ~SerialBridge();
    
    // Prevent copying
    SerialBridge(const SerialBridge&) = delete;
    SerialBridge& operator=(const SerialBridge&) = delete;
    
    // Serial port handle
    void* serialPort_;
    
    // Thread for reading from the serial port
    std::thread readThread_;
    std::atomic<bool> running_;
    
    // Queue for incoming data
    std::queue<std::string> incomingData_;
    std::mutex incomingMutex_;
    
    // Read thread function
    void readThreadFunc();
};
