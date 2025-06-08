#include "SimulatorAdapter.h"
#include "DisplayFactory.h"

// Initialize static members
DisplayManager* SimulatorAdapter::_displayManager = nullptr;
SDL_Window* SimulatorAdapter::_window = nullptr;
SDL_Renderer* SimulatorAdapter::_renderer = nullptr;
SDL_Texture* SimulatorAdapter::_texture = nullptr;
bool SimulatorAdapter::_initialized = false;

bool SimulatorAdapter::initialize(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* texture) {
    if (_initialized) {
        return true;
    }
    
    // Store SDL pointers
    _window = window;
    _renderer = renderer;
    _texture = texture;
    
    // Get DisplayManager instance
    _displayManager = &DisplayManager::getInstance();
    
    // Initialize DisplayManager with Serial display for now
    // We'll add the simulator display later when we implement it
    DisplayType displayTypes[] = { DisplayType::Serial };
    if (!_displayManager->initialize(displayTypes, 1)) {
        printf("Failed to initialize DisplayManager\n");
        return false;
    }
    
    _initialized = true;
    printf("SimulatorAdapter initialized successfully\n");
    return true;
}

void SimulatorAdapter::update() {
    if (!_initialized) {
        return;
    }
    
    // Update DisplayManager
    _displayManager->update();
}

DisplayManager& SimulatorAdapter::getDisplayManager() {
    if (!_displayManager) {
        _displayManager = &DisplayManager::getInstance();
    }
    return *_displayManager;
}

void SimulatorAdapter::setScreen(ScreenType screen) {
    if (!_initialized) {
        return;
    }
    
    // Set screen in DisplayManager
    _displayManager->setScreen(screen);
}
