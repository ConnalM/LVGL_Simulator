#include "TerminalSerial.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>
#include <cstdarg>
#include <cstdio>
#include <chrono>
#include <windows.h>
#include <conio.h>

TerminalSerial::TerminalSerial(bool echo)
    : _echo(echo), _running(true) {
    _inputThread = std::thread(&TerminalSerial::inputThreadFunc, this);
}

TerminalSerial::~TerminalSerial() {
    _running = false;
    if (_inputThread.joinable()) {
        _inputThread.join();
    }
}

void TerminalSerial::print(const std::string& msg, bool newLine) {
    std::cout << msg;
    if (newLine) std::cout << std::endl;
    std::cout << std::flush;
}

void TerminalSerial::println(const std::string& msg) { print(msg, true); }
void TerminalSerial::println(int num) { print(std::to_string(num), true); }
void TerminalSerial::println(float num) { print(std::to_string(num), true); }
void TerminalSerial::println(double num) { print(std::to_string(num), true); }
void TerminalSerial::println(unsigned int num) { print(std::to_string(num), true); }
void TerminalSerial::println(long num) { print(std::to_string(num), true); }
void TerminalSerial::println(unsigned long num) { print(std::to_string(num), true); }

void TerminalSerial::printf(const char* fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    print(std::string(buffer), false);
}

void TerminalSerial::print(int num, bool newLine) { print(std::to_string(num), newLine); }
void TerminalSerial::print(float num, bool newLine) { print(std::to_string(num), newLine); }
void TerminalSerial::print(double num, bool newLine) { print(std::to_string(num), newLine); }
void TerminalSerial::print(unsigned int num, bool newLine) { print(std::to_string(num), newLine); }
void TerminalSerial::print(long num, bool newLine) { print(std::to_string(num), newLine); }
void TerminalSerial::print(unsigned long num, bool newLine) { print(std::to_string(num), newLine); }
void TerminalSerial::print(short num, bool newLine) { print(std::to_string(num), newLine); }

bool TerminalSerial::available() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    return !_inputQueue.empty();
}

char TerminalSerial::read() {
    std::lock_guard<std::mutex> lock(_inputMutex);
    if (_inputQueue.empty()) return -1;
    char c = _inputQueue.front();
    _inputQueue.pop();
    return c;
}

std::string TerminalSerial::readLine() {
    std::string line;
    while (true) {
        if (available()) {
            char c = read();
            if (c == '\r') {
                if (!line.empty()) break;
            } else if (c == 8 || c == 127) {
                if (!line.empty()) {
                    line.pop_back();
                    std::cout << "\b \b" << std::flush;
                }
            } else if (c >= 32 && c <= 126) {
                line += c;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::cout << std::endl;
    return line;
}

void TerminalSerial::inputThreadFunc() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));

    while (_running) {
        if (_kbhit()) {
            char c = _getch();
            std::lock_guard<std::mutex> lock(_inputMutex);
            _inputQueue.push(c);
            if (_echo && c != '\n' && c != '\r') {
                std::cout << c << std::flush;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    SetConsoleMode(hStdin, mode);
}
