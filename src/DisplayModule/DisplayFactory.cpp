#include "DisplayFactory.h"
#include "../common/DebugUtils.h"
#include "../ModuleToggle.h"
#include "ESP32_8048S070_Lvgl_DisplayDriver.h"

// Static instance for singleton pattern
DisplayFactory* DisplayFactory::_instance = nullptr;

DisplayFactory::DisplayFactory() 
    : _serialDisplay(nullptr)
    , _lcdDisplay(nullptr){
    DPRINTLN("DisplayFactory created");
}

DisplayFactory& DisplayFactory::getInstance() {
    if (_instance == nullptr) {
        _instance = new DisplayFactory();
    }
    return *_instance;
}

IBaseDisplay* DisplayFactory::createDisplay(DisplayType type) {
    // Create display based on type
    switch (type) {
        case DisplayType::Serial:
            if (_serialDisplay == nullptr) {
                _serialDisplay = new SerialDisplay();
            }
            return _serialDisplay;
            
        case DisplayType::LCD:
            if (_lcdDisplay == nullptr) {
                _lcdDisplay = new ESP32_8048S070_Lvgl_DisplayDriver();
            }
            return _lcdDisplay;
            
        case DisplayType::Web:
            // Web display not implemented yet
            return nullptr;
            
        default:
            // Default to serial display
            if (_serialDisplay == nullptr) {
                _serialDisplay = new SerialDisplay();
            }
            return _serialDisplay;
    }
}

IBaseDisplay* DisplayFactory::getDisplay(DisplayType type) {
    // Get existing display or create a new one
    switch (type) {
        case DisplayType::Serial:
            return _serialDisplay != nullptr ? _serialDisplay : createDisplay(type);
            
        case DisplayType::LCD:
            return _lcdDisplay != nullptr ? _lcdDisplay : createDisplay(type);
            
        case DisplayType::Web:
            // Web display not implemented yet
            return nullptr;
            
        default:
            // Default to serial display
            return _serialDisplay != nullptr ? _serialDisplay : createDisplay(type);
    }
}

IGraphicalDisplay* DisplayFactory::createGraphicalDisplay(DisplayType type) {
    // Get existing display instance
    IBaseDisplay* baseDisplay = getDisplay(type);
    if (baseDisplay && baseDisplay->getDisplayType() == DisplayType::LCD) {
        return static_cast<IGraphicalDisplay*>(baseDisplay);
    }
    return nullptr;
}

IGraphicalDisplay* DisplayFactory::getGraphicalDisplay(DisplayType type) {
    // Get existing graphical display or create a new one
    switch (type) {
        case DisplayType::LCD:
            return _lcdDisplay != nullptr ? _lcdDisplay : createGraphicalDisplay(type);
            
        case DisplayType::Web:
            // Web display not implemented yet
            return nullptr;
            
        case DisplayType::Serial:
        default:
            // Serial display is not graphical
            return nullptr;
    }
}

void DisplayFactory::destroyInstance() {
    if (_instance) {
        if (_instance->_serialDisplay) {
            delete _instance->_serialDisplay;
            _instance->_serialDisplay = nullptr;
        }
        if (_instance->_lcdDisplay) {
            delete _instance->_lcdDisplay;
            _instance->_lcdDisplay = nullptr;
        }
        delete _instance;
        _instance = nullptr;
        DPRINTLN("DisplayFactory destroyed");
    }
}
