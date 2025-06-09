#include "TimeManager.h"
#include "Arduino.h"

bool TimeManager::Initialize() {
    m_currentTimeMs = millis();
    return true;
}

void TimeManager::Update() {
    if (!m_isPaused) {
        m_currentTimeMs = millis();
    }
}

uint32_t TimeManager::GetCurrentTimeMs() const {
    return m_currentTimeMs;
}

void TimeManager::Pause() {
    if (!m_isPaused) {
        m_isPaused = true;
        m_pausedTimeMs = millis();
    }
}

void TimeManager::Resume() {
    if (m_isPaused) {
        m_isPaused = false;
        // Adjust for the time spent paused
        uint32_t pauseDuration = millis() - m_pausedTimeMs;
        m_currentTimeMs += pauseDuration;
    }
}

bool TimeManager::IsPaused() const {
    return m_isPaused;
}

void TimeManager::updateTime() {
    if (!m_isPaused) {
        m_currentTimeMs = millis();
    }
}
