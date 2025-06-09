#pragma once
#include "InputCommand.h"

class InputModule {
public:
    virtual ~InputModule() {}
    virtual bool poll(InputEvent& event) = 0;
};
