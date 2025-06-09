#pragma once
#include <Arduino.h>
#include "ConfigSettings.h"

namespace ConfigEEPROM {
    bool save(const ConfigSettings& settings);
    bool load(ConfigSettings& settings);
}
