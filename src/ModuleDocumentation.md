# Module Documentation: Display4 Project

## 1. Introduction

This document provides a comprehensive overview of the Display4 project's software architecture, module system, and data flow. It is intended to help developers understand the system's design and how different components interact.

## 2. Core Architectural Principles

*   **Modularity**: The system is divided into distinct modules, each responsible for a specific set of functionalities (e.g., Display, Input, Race Logic).
*   **Singleton Pattern**: Many core modules (e.g., `SystemController`, `RaceModule`, `InputManager`, `DisplayManager`, `TimeManager`) are implemented as singletons to ensure a single, globally accessible instance.
*   **Interface-Based Design**: Where applicable, modules interact through defined interfaces (e.g., `IBaseDisplay`, `InputModule`) to promote loose coupling and testability.
*   **Centralized Control**: The `SystemController` acts as the central orchestrator, managing system state and coordinating interactions between other modules.
*   **Event-Driven**: Input is handled through an event system (`InputEvent`), and state changes in modules like `RaceModule` and `LightsModule` are communicated via callbacks.
*   **Configuration via `ModuleToggle.h`**: Feature sets and specific module implementations (e.g., types of input, display outputs) can be enabled or disabled at compile-time using preprocessor directives in `ModuleToggle.h`.

## 3. Main Modules and Their Functions

The primary modules reside under the `src/` directory.

### 3.1. `SystemController`
*   **Directory**: `src/SystemController/`
*   **Key Files**: `SystemController.h`, `SystemController.cpp`
*   **Purpose**: This singleton class (`SystemController::getInstance()`) is the brain of the application. It initializes and manages all other major modules. It controls the overall application flow, manages system states (e.g., main menu, race mode, config mode), processes input events from `InputManager`, and delegates tasks to appropriate modules.
*   **Key Functionality**:
    *   Initializes all core modules (`DisplayManager`, `InputManager`, `RaceModule`, `ConfigModule`, `LightsModule`, `TimeManager`).
    *   Manages `SystemState` (e.g., `Main`, `RaceMode`).
    *   Contains the main `update()` loop logic, which polls for input via `InputManager` and calls `update()` on other modules.
    *   `processInputEvent(const InputEvent& event)`: Interprets input events and triggers actions in other modules (e.g., starting a race, changing configuration).
    *   Handles callbacks from `RaceModule` and `LightsModule` to react to state changes (e.g., lap registered, countdown step).
    *   Manages UI flow (e.g., `showMain()`, `showRaceReady()`), instructing `DisplayManager` what to display.
*   **Interactions**:
    *   Receives `InputEvent`s from `InputManager`.
    *   Sends commands to `RaceModule` (e.g., `prepareRace`, `startCountdown`, `registerLap`).
    *   Sends commands to `ConfigModule` (e.g., `handleSetLaps`).
    *   Sends commands/data to `DisplayManager` to update the UI.
    *   Sends commands to `LightsModule` (e.g., `startSequence`).
    *   Receives state change notifications (callbacks) from `RaceModule` and `LightsModule`.

### 3.2. `DisplayModule`
*   **Directory**: `src/DisplayModule/`
*   **Key Files**:
    *   `DisplayModule.h`: Defines display interfaces (`IBaseDisplay`, `IGraphicalDisplay`).
    *   `DisplayManager.h`, `DisplayManager.cpp`: The core display coordinator.
    *   `SerialDisplay.h/.cpp`: Concrete implementation for serial output.
    *   `ESP32_8048S070_Display.h/.cpp`: Concrete implementation for a specific LCD.
    *   (Other display implementations like `WebDisplay` might exist).
*   **Purpose**: Manages all aspects of outputting information to various display devices.
*   **`DisplayManager` (Singleton)**:
    *   Manages a collection of active display instances (e.g., Serial, LCD).
    *   Provides a unified interface for other modules to send data for display (e.g., `info()`, `error()`, `showRaceStatus()`, `setScreen()`).
    *   Formats data appropriately before sending it to the actual display drivers.
    *   Handles different screen states/layouts.
*   **`IBaseDisplay` / `IGraphicalDisplay`**: Interfaces that concrete display implementations must adhere to, ensuring consistent API for basic text and graphical operations.
*   **Interactions**:
    *   `SystemController` is the primary client, telling `DisplayManager` what to show and when.
    *   Other modules might use `DisplayManager` for logging simple messages.
    *   `DisplayManager` calls methods on concrete display instances (`SerialDisplay`, `ESP32_8048S070_Display`) via their common interface.

### 3.3. `InputModule`
*   **Directory**: `src/InputModule/`
*   **Key Files**:
    *   `InputModule.h`: Defines the abstract base class `InputModule` with a `poll()` method.
    *   `InputManager.h`, `InputManager.cpp`: The central manager for all input sources.
    *   `InputCommand.h`: Defines `InputEvent` struct and `InputCommand` enum, standardizing input data.
    *   `KeyboardInput.h/.cpp`: Concrete module for serial/keyboard input.
    *   `ButtonInput.h/.cpp`: Concrete module for physical button input.
    *   `SensorInput.h/.cpp`: Concrete module for race track sensor input.
    *   (Other input modules like `TouchInput` might exist).
*   **Purpose**: Handles all forms of input into the system.
*   **`InputManager` (Singleton)**:
    *   Manages a collection of registered `InputModule` instances.
    *   Provides a single `poll(InputEvent& event)` method that `SystemController` calls. `InputManager` iterates through its registered modules, calling their `poll()` methods until an event is captured.
    *   May also have an `update()` method for modules that require periodic processing.
*   **`InputModule` (Abstract Class)**:
    *   Concrete input handlers (like `KeyboardInput`, `ButtonInput`) inherit from this.
    *   Must implement `poll(InputEvent& event)` which checks for input and populates the `InputEvent` struct if input occurs.
    *   Must implement `initialize()`.
*   **`InputEvent`**: A standardized struct containing `command`, `target`, `value`, `sourceId`, and `timestamp`. This decouples `SystemController` from the specifics of how input was generated.
*   **Interactions**:
    *   `SystemController` polls `InputManager` for events.
    *   `InputManager` polls concrete `InputModule` instances.
    *   Concrete `InputModule`s read from hardware (Serial, GPIO pins).
    *   `main.cpp` initializes and registers concrete `InputModule`s with `InputManager`.

### 3.4. `RaceModule`
*   **Directory**: `src/RaceModule/`
*   **Key Files**: `RaceModule.h`, `RaceModule.cpp`
*   **Purpose**: Manages the state, logic, and data for races. This includes lap counting, timing, race status, and racer data.
*   **Key Functionality (Singleton)**:
    *   Manages `RaceState` (e.g., `Idle`, `Ready`, `Countdown`, `Running`, `Paused`, `Finished`).
    *   `initialize()`: Sets up default race parameters.
    *   `prepareRace()`: Configures a race based on parameters from `SystemController` (e.g., number of laps, race mode).
    *   `startCountdown()`: Initiates the pre-race countdown sequence (often by interacting with `SystemController` which then uses `LightsModule`).
    *   `startRace()`: Begins the actual race timing.
    *   `registerLap(int laneId)`: Records a lap for a given lane, updates lap times, checks for race completion.
    *   `pauseRace()`, `resumeRace()`, `finishRace()`.
    *   Manages `RaceLaneData` for each lane (lap times, current lap, status).
    *   Provides data accessors for `SystemController` to query race status for display.
    *   Uses callbacks (e.g., `_onRaceStateChangedCallback`, `_onLapRegisteredCallback`, `_onSecondTickCallback`) to notify `SystemController` of significant events. These callbacks are registered by `SystemController` during its initialization.
*   **Interactions**:
    *   `SystemController` sends commands to start, stop, pause, and configure races.
    *   `SystemController` calls `registerLap()` based on sensor events.
    *   `SystemController` queries race data for display purposes.
    *   Notifies `SystemController` of state changes and events via callbacks.
    *   Relies on `TimeManager` for all timing.

### 3.5. `LightsModule`
*   **Directory**: `src/LightsModule/`
*   **Key Files**: `LightsModule.h`, `LightsModule.cpp`
*   **Purpose**: Manages the race start light sequence (e.g., countdown lights).
*   **Key Functionality**:
    *   `initialize()`: Sets up GPIO pins for lights.
    *   `startSequence(callback)`: Begins the light countdown sequence. Takes a callback function (typically a method in `SystemController`) to notify on each step and on completion.
    *   `update()`: Handles the timing and progression of the light sequence.
    *   Manages the state of the light sequence.
    *   Uses callbacks (`_onCountdownStepCallback`, `_onCountdownCompletedCallback`) to inform `SystemController` about the progress and completion of the sequence.
*   **Interactions**:
    *   `SystemController` commands it to `startSequence()`.
    *   Notifies `SystemController` of sequence steps and completion via callbacks.
    *   Directly controls hardware (GPIO pins for lights).

### 3.6. `ConfigModule`
*   **Directory**: `src/ConfigModule/`
*   **Key Files**: `ConfigModule.h`, `ConfigModule.cpp`, `ConfigSettings.h`
*   **Purpose**: Manages persistent configuration settings for the application, such as default race parameters, display preferences, etc. It handles loading settings from and saving settings to non-volatile storage (e.g., EEPROM, SPIFFS).
*   **Key Functionality**:
    *   `initialize()`: Loads configuration from storage.
    *   Provides methods to get and set configuration values (e.g., `getLaps()`, `setLaps()`).
    *   `saveSettings()`: Persists current settings to storage.
    *   `ConfigSettings.h` defines the structure (`ConfigSettings`) that holds the configuration data.
*   **Interactions**:
    *   `SystemController` interacts with it to retrieve current settings or to update settings based on user input.
    *   Other modules might query it for configuration values during their initialization or operation.
    *   Reads from/writes to persistent storage.

### 3.7. `common` Utilities
*   **Directory**: `src/common/`
*   **Purpose**: Contains shared utility code, type definitions, and base classes used by multiple modules.
*   **Key Files**:
    *   `Types.h`: Defines fundamental data types, enumerations (`RaceMode`, `ErrorCode`, `InputSourceId`), constants (`MAX_LANES`), and common data structures (`ErrorInfo`, `LapData`, `LaneData`, `RaceData`). Note: `RaceModule` uses its own more detailed `RaceLaneData` for live race tracking.
    *   `TimeManager.h/.cpp`: Singleton providing a precise, centralized time source using `TickTwo` library. All modules should use `TimeManager::getInstance()->GetCurrentTimeMs()` for timestamps. Supports `Pause()` and `Resume()`.
    *   `Debug.h/.cpp`: Advanced debugging utility (`Debug` global instance) with levels, channels, and macros for file/line info. Distinct from user-facing logging via `DisplayManager`.
    *   `StringUtils.h/.cpp`: (Assumed) Helper functions for string manipulation.
    *   `ModuleTemplate.h`: (Assumed) A template/example for creating new modules to ensure consistency.
*   **Interactions**: These utilities are included and used by most other modules as needed.

## 4. `main.cpp` and `ModuleToggle.h`

*   **`src/main.cpp`**:
    *   **Purpose**: The entry point of the Arduino application (`setup()` and `loop()` functions).
    *   **`setup()`**:
        *   Initializes serial communication and hardware.
        *   Crucially, initializes all singleton modules in the correct order of dependency (e.g., `DisplayManager` first for logging, then `TimeManager`, then other functional modules, and `SystemController` last).
        *   Uses `ModuleToggle.h` directives (`#ifdef`) to conditionally compile and initialize optional modules/features.
        *   Registers concrete `InputModule`s with `InputManager`.
        *   Registers callbacks between `SystemController` and other modules like `RaceModule` and `LightsModule`.
        *   Typically ends by telling `SystemController` to show the initial UI screen.
    *   **`loop()`**:
        *   Kept minimal.
        *   Primarily calls the `update()` methods of `SystemController` and any other modules that require continuous polling or processing (e.g., `TimeManager`, `InputManager`, `RaceModule`, `DisplayManager`, `LightsModule`). The bulk of the application logic resides within these `update()` methods, especially `SystemController::update()`.
*   **`src/ModuleToggle.h`**:
    *   **Purpose**: Uses C preprocessor `#define` directives to enable or disable the compilation of specific modules or features (e.g., `#define ENABLE_INPUT_KEYBOARD`).
    *   Allows for different build configurations from the same codebase.

## 5. Data Flow and Inter-Module Communication

*   **Input Processing**:
    1.  Physical input (button press, sensor trigger, serial command) occurs.
    2.  A concrete `InputModule` (e.g., `ButtonInput`) detects this.
    3.  During `SystemController::update() -> InputManager::poll() -> ConcreteInputModule::poll()`, the `InputModule` creates an `InputEvent` struct.
    4.  `InputManager` returns this event to `SystemController`.
    5.  `SystemController::processInputEvent()` analyzes the event based on current `SystemState` and `event.command`, `event.target`, etc.
    6.  `SystemController` then calls appropriate methods on other modules (e.g., `RaceModule::startCountdown()`, `ConfigModule::handleSetLaps(event.value)`).

*   **Display Output**:
    1.  `SystemController` (or another module) determines something needs to be displayed.
    2.  It calls a method on `DisplayManager::getInstance()` (e.g., `showRaceStatus(raceData)`, `info("Message")`).
    3.  `DisplayManager` formats the data if necessary and then iterates through its active display instances (e.g., `SerialDisplay`, `LCDDisplay`).
    4.  It calls the appropriate method (e.g., `printLine()`, `drawText()`) on each active display instance, usually via the `IBaseDisplay` or `IGraphicalDisplay` interface.

*   **Race Logic and Callbacks**:
    1.  `SystemController` commands `RaceModule` (e.g., `prepareRace()`).
    2.  `RaceModule` changes its internal state and performs actions.
    3.  When a significant event occurs in `RaceModule` (e.g., race state changes, lap registered, second ticks), it invokes a callback function that was registered by `SystemController`.
    4.  The `SystemController`'s callback handler (e.g., `onRaceStateChanged()`) is executed.
    5.  Inside this handler, `SystemController` may update its own state, instruct `DisplayManager` to show new information, or command other modules.
    6.  A similar callback mechanism is used by `LightsModule` to inform `SystemController` about the progress of the start light sequence.
