#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>
#include <chrono>
#include <windows.h>
#include <conio.h>

class SerialDisplay {
private:
    std::thread _inputThread;
    std::atomic<bool> _running{false};
    std::queue<char> _inputQueue;
    std::mutex _inputMutex;
    bool _echo;

    void inputThreadFunc() {
        // Save the original console mode
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode = 0;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

        while (_running) {
            if (_kbhit()) {
                char c = _getch();
                std::lock_guard<std::mutex> lock(_inputMutex);
                _inputQueue.push(c);
                std::cout << "[DEBUG] Got char: " << (int)c << std::endl << std::flush;
                if (_echo && c != '\n' && c != '\r') {
                    std::cout << c << std::flush;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Restore the original console mode
        SetConsoleMode(hStdin, mode);
    }

public:
    SerialDisplay(bool echo = true) : _echo(echo) {}
    
    bool initialize() {
        _running = true;
        _inputThread = std::thread(&SerialDisplay::inputThreadFunc, this);
        return true;
    }

    ~SerialDisplay() {
        _running = false;
        if (_inputThread.joinable()) {
            _inputThread.join();
        }
    }

    void print(const std::string& message, bool newLine = false) {
        std::cout << message;
        if (newLine) std::cout << std::endl;
        std::cout << std::flush;
    }

    bool available() {
        std::lock_guard<std::mutex> lock(_inputMutex);
        return !_inputQueue.empty();
    }

    char read() {
        std::lock_guard<std::mutex> lock(_inputMutex);
        if (_inputQueue.empty()) return -1;
        char c = _inputQueue.front();
        _inputQueue.pop();
        return c;
    }

    std::string readLine() {
        std::string line;
        while (true) {
            if (available()) {
                char c = read();
                if (c == '\r') {  // Windows uses \r\n for newline
                    if (!line.empty()) break;
                } else if (c == 8 || c == 127) { // Backspace or Delete
                    if (!line.empty()) {
                        line.pop_back();
                        // Move cursor back, print space, move back again
                        std::cout << "\b \b" << std::flush;
                    }
                } else if (c >= 32 && c <= 126) { // Printable ASCII
                    line += c;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << std::endl;
        return line;
    }
};

int main() {
    // Set console output to UTF-8
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "[DEBUG] main() started" << std::endl << std::flush;
    std::cout << "=== SerialDisplay Test (Windows) ===" << std::endl << std::flush;
    std::cout << "Type something and press Enter" << std::endl << std::flush;
    std::cout << "Type 'exit' to quit" << std::endl << std::flush;
    
    SerialDisplay serial(true); // true = echo input
    if (!serial.initialize()) {
        std::cerr << "Failed to initialize SerialDisplay!" << std::endl;
        return 1;
    }

    while (true) {
        std::string input = serial.readLine();
        std::cout << "You typed: " << input << std::endl << std::flush;
        
        if (input == "exit") {
            break;
        }
    }

    std::cout << "Test complete!" << std::endl << std::flush;
    return 0;
}
