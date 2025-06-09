#include "SensorInput.h"
#include "DisplayModule/DisplayManager.h"

bool SensorInput::poll(InputEvent& event) {
    // Implementation note: This is a stub that would normally read from sensors
    // For now, it returns false as there's no actual sensor reading logic
    return false;
}

bool SensorInput::validateLaneNumber(int laneNumber) {
    // Validate that lane number is between 1 and 8 (inclusive)
    bool isValid = (laneNumber >= 1 && laneNumber <= 8);
    
    if (!isValid) {
        String errorMsg = "Invalid lane number: ";
        errorMsg += laneNumber;
        errorMsg += ". Must be between 1-8.";
        DisplayManager::getInstance().error(errorMsg, "SensorInput");
    }
    
    return isValid;
}
