#include "ConfigEEPROM.h"
#include <EEPROM.h>

namespace {
    const int EEPROM_SIZE = 32;
    const int EEPROM_ADDR = 0;
}

bool ConfigEEPROM::save(const ConfigSettings& settings) {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_ADDR, settings);
    bool ok = EEPROM.commit();
    EEPROM.end();
    return ok;
}

bool ConfigEEPROM::load(ConfigSettings& settings) {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(EEPROM_ADDR, settings);
    EEPROM.end();
    return true; // Could add more checks if needed
}
