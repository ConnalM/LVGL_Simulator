#pragma once

#include <cstdint>

class TimeManager {
public:
    // Delete copy constructor and assignment operator
    TimeManager(const TimeManager&) = delete;
    TimeManager& operator=(const TimeManager&) = delete;

    // Get the singleton instance
    static TimeManager& GetInstance() {
        static TimeManager instance;
        return instance;
    }

    bool Initialize();
    void Update();
    uint32_t GetCurrentTimeMs() const;
    void Pause();
    void Resume();
    bool IsPaused() const;

private:
    TimeManager() = default;
    ~TimeManager() = default;

    void updateTime();
    
    uint32_t m_currentTimeMs = 0;
    bool m_isPaused = false;
    uint32_t m_pausedTimeMs = 0;
};
