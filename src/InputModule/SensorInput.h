#pragma once
#include "InputModule.h"

/**
 * SensorInput - Handles remote sensor triggers for lap counting.
 *
 * Output Standardization:
 * - All outputs are standardized InputEvent structs using the shared InputCommand enum.
 * - SensorInput only ever sends AddLap commands for lanes 1-8 (numbers 1-8).
 * - Output format must match KeyboardInput and ButtonInput for AddLap commands.
 *
 * Supported events:
 * - Lane 1-8 triggered: InputCommand::AddLap with sourceId = lane number
 */
class SensorInput : public InputModule {
public:
    /**
     * @brief Poll for sensor input events
     * @param event Reference to store the received event
     * @return true if an event was received, false otherwise
     */
    bool poll(InputEvent& event) override;

    /**
     * @brief Validate that a lane number is within the allowed range (1-8)
     * @param laneNumber The lane number to validate
     * @return true if valid, false otherwise
     */
    bool validateLaneNumber(int laneNumber);

protected:
    static constexpr int MIN_LANE = 1;
    static constexpr int MAX_LANE = 8;
};
