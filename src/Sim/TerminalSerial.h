#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

class TerminalSerial {
public:
    TerminalSerial(bool echo = true);
    ~TerminalSerial();

    void print(const std::string& msg, bool newLine = false);
    void print(int num, bool newLine = false);
    void print(float num, bool newLine = false);
    void print(double num, bool newLine = false);
    void print(unsigned int num, bool newLine = false);
    void print(long num, bool newLine = false);
    void print(unsigned long num, bool newLine = false);
    void print(short num, bool newLine = false);
    bool available();
    char read();
    std::string readLine();

    // Added println and printf overloads
    void println(const std::string& msg = "");
    void println(int num);
    void println(float num);
    void println(double num);
    void println(unsigned int num);
    void println(long num);
    void println(unsigned long num);
    void printf(const char* fmt, ...);

private:
    void inputThreadFunc();
    std::thread _inputThread;
    std::atomic<bool> _running;
    std::queue<char> _inputQueue;
    std::mutex _inputMutex;
    bool _echo;
};
