#include "SerialBridge.h"
#include <Windows.h>
#include <iostream>
#include <chrono>
#include "common/log_message.h"


// Singleton instance
SerialBridge& SerialBridge::getInstance() {
    static SerialBridge instance;
    return instance;
}

SerialBridge::SerialBridge() : serialPort_(INVALID_HANDLE_VALUE), running_(false) {
}

SerialBridge::~SerialBridge() {
    close();
}

bool SerialBridge::initialize(const std::string& portName, int baudRate) {
    // Close any existing connection
    close();
    
    log_message("Initializing serial bridge on port %s at %d baud", portName.c_str(), baudRate);
    
    // Open the serial port
    HANDLE hSerial = CreateFileA(
        portName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,                          // No sharing
        NULL,                       // No security
        OPEN_EXISTING,              // Open existing port only
        0,                          // Non-overlapped I/O
        NULL                        // Null for comm devices
    );
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        log_message("Error opening serial port: %lu", error);
        return false;
    }
    
    // Configure the serial port
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        log_message("Error getting serial port state");
        CloseHandle(hSerial);
        return false;
    }
    
    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        log_message("Error setting serial port state");
        CloseHandle(hSerial);
        return false;
    }
    
    // Set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        log_message("Error setting serial port timeouts");
        CloseHandle(hSerial);
        return false;
    }
    
    // Store the handle
    serialPort_ = hSerial;
    
    // Start the read thread
    running_ = true;
    readThread_ = std::thread(&SerialBridge::readThreadFunc, this);
    
    log_message("Serial bridge initialized successfully");
    return true;
}

void SerialBridge::close() {
    // Stop the read thread
    if (running_) {
        running_ = false;
        if (readThread_.joinable()) {
            readThread_.join();
        }
    }
    
    // Close the serial port
    if (serialPort_ != INVALID_HANDLE_VALUE) {
        CloseHandle(static_cast<HANDLE>(serialPort_));
        serialPort_ = INVALID_HANDLE_VALUE;
        log_message("Serial bridge closed");
    }
}

bool SerialBridge::send(const std::string& data) {
    if (serialPort_ == INVALID_HANDLE_VALUE) {
        return false;
    }
    
    DWORD bytesWritten;
    bool success = WriteFile(
        static_cast<HANDLE>(serialPort_),
        data.c_str(),
        static_cast<DWORD>(data.size()),
        &bytesWritten,
        NULL
    );
    
    if (!success || bytesWritten != data.size()) {
        log_message("Error writing to serial port");
        return false;
    }
    
    return true;
}

bool SerialBridge::dataAvailable() {
    std::lock_guard<std::mutex> lock(incomingMutex_);
    return !incomingData_.empty();
}

std::string SerialBridge::read() {
    std::lock_guard<std::mutex> lock(incomingMutex_);
    if (incomingData_.empty()) {
        return "";
    }
    
    std::string data = incomingData_.front();
    incomingData_.pop();
    return data;
}

void SerialBridge::update() {
    // Nothing to do here, the read thread handles reading
}

void SerialBridge::readThreadFunc() {
    const int bufferSize = 256;
    char buffer[bufferSize];
    
    while (running_) {
        if (serialPort_ == INVALID_HANDLE_VALUE) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        DWORD bytesRead;
        bool success = ReadFile(
            static_cast<HANDLE>(serialPort_),
            buffer,
            bufferSize - 1,
            &bytesRead,
            NULL
        );
        
        if (success && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            std::string data(buffer, bytesRead);
            
            // Add to the queue
            {
                std::lock_guard<std::mutex> lock(incomingMutex_);
                incomingData_.push(data);
            }
            
            // Log the received data
            log_message("Serial received: %s", data.c_str());
        }
        
        // Sleep a bit to avoid hogging the CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
