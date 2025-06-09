#include "StringUtils.h"
#include "common/Types.h"
#include "InputModule/InputCommand.h"

const char* inputSourceToString(int sourceId) {
    switch (static_cast<InputSourceId>(sourceId)) {
        case InputSourceId::KEYBOARD: return "Keyboard";
        case InputSourceId::TOUCH: return "Touch";
        case InputSourceId::SENSOR: return "Sensor";
        case InputSourceId::WEB: return "Web";
        default: return "Unknown";
    }
}

const char* inputCommandToString(int command) {
    switch (static_cast<InputCommand>(command)) {
        case InputCommand::AddLap: return "AddLap";
        case InputCommand::RemoveLap: return "RemoveLap";
        case InputCommand::StartCountdown: return "StartCountdown";
        case InputCommand::StartRace: return "StartRace";
        case InputCommand::PauseRace: return "PauseRace";
        case InputCommand::ResumeRace: return "ResumeRace";
        case InputCommand::StopRace: return "StopRace";
        case InputCommand::ResetRace: return "ResetRace";
        case InputCommand::SetNumLaps: return "SetNumLaps";
        case InputCommand::SetNumLanes: return "SetNumLanes";
        case InputCommand::ChangeMode: return "ChangeMode";
        case InputCommand::SetRaceTime: return "SetRaceTime";
        case InputCommand::ToggleBestLap: return "ToggleBestLap";
        case InputCommand::ToggleReactionTime: return "ToggleReactionTime";
        case InputCommand::EnterConfig: return "EnterConfig";
        case InputCommand::EnableLane: return "EnableLane";
        case InputCommand::DisableLane: return "DisableLane";
        case InputCommand::AddRacer: return "AddRacer";
        case InputCommand::RemoveRacer: return "RemoveRacer";
        case InputCommand::SetCountdownInterval: return "SetCountdownInterval";
        default: return "Unknown";
    }
}
