#include "ConfigModule.h"
#include "ConfigEEPROM.h"
#include "ConfigDisplay.h"

ConfigModule configModule; // Definition of the global instance

ConfigModule::ConfigModule() : _initialized(false) {}

bool ConfigModule::initialize() {
    // Check if already initialized
    if (_initialized) {
        Serial.println(F("ConfigModule: Already initialized"));
        return true;
    }
    
    Serial.println(F("ConfigModule: Initializing..."));
    
    // Load settings from persistent storage
    loadSettings();
    
    // Print current settings for debugging
    printSettingsToSerial();
    
    // Display settings on the UI
    ConfigDisplay::showSettings(settings);
    
    _initialized = true;
    Serial.println(F("ConfigModule: Initialized successfully"));
    return true;
}

void ConfigModule::update() {
    // Skip if not initialized
    if (!_initialized) {
        return;
    }
    
    // Currently no periodic updates needed
    // This method is included for consistency with other modules
}

void ConfigModule::handleSetLaps(uint8_t laps) {
    // Skip if not initialized
    if (!_initialized) {
        feedback("ConfigModule not initialized", true);
        return;
    }
    
    if (laps < 1 || laps > 99) {
        feedback("Invalid lap count!", true);
        return;
    }
    settings.numLaps = laps;
    saveSettings();
    feedback("Lap count updated.");
    printSettingsToSerial();
    ConfigDisplay::showSettings(settings);
}

void ConfigModule::handleSetLanes(uint8_t lanes) {
    // Skip if not initialized
    if (!_initialized) {
        feedback("ConfigModule not initialized", true);
        return;
    }
    
    if (lanes < 1 || lanes > 8) {
        feedback("Invalid lane count!", true);
        return;
    }
    settings.numLanes = lanes;
    saveSettings();
    feedback("Lane count updated.");
    printSettingsToSerial();
    ConfigDisplay::showSettings(settings);
}

void ConfigModule::handleSetRaceMode(uint8_t mode) {
    // Skip if not initialized
    if (!_initialized) {
        feedback("ConfigModule not initialized", true);
        return;
    }
    
    if (mode > 3) {
        feedback("Invalid race mode!", true);
        return;
    }
    settings.raceMode = mode;
    saveSettings();
    feedback("Race mode updated.");
    printSettingsToSerial();
    ConfigDisplay::showSettings(settings);
}

void ConfigModule::saveSettings() {
    if (ConfigEEPROM::save(settings)) {
        feedback("Settings saved to EEPROM.");
    } else {
        feedback("EEPROM write failed!", true);
    }
}

void ConfigModule::loadSettings() {
    ConfigEEPROM::load(settings);
}

void ConfigModule::feedback(const char* msg, bool error) {
    if (error) Serial.print("ERROR: ");
    Serial.println(msg);
    ConfigDisplay::showMessage(msg);
}

void ConfigModule::printSettingsToSerial() const {
    Serial.println("---- Current Config Settings ----");
    Serial.print("Num Laps: "); Serial.println(settings.numLaps);
    Serial.print("Num Lanes: "); Serial.println(settings.numLanes);
    Serial.print("Mode: ");
    switch (settings.raceMode) {
        case 0: Serial.println("LAPS"); break;
        case 1: Serial.println("TIMER"); break;
        case 2: Serial.println("DRAG"); break;
        case 3: Serial.println("RALLY"); break;
        default: Serial.println("UNKNOWN"); break;
    }
    Serial.println("--------------------------------");
}
