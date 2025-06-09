#pragma once
#include "common/Types.h"
#include "InputModule/InputCommand.h"

/**
 * @brief Convert input source ID to string
 * 
 * @param sourceId The input source ID to convert
 * @return const char* String representation of the input source
 */
const char* inputSourceToString(int sourceId);

/**
 * @brief Convert input command to string
 * 
 * @param command The input command to convert
 * @return const char* String representation of the input command
 */
const char* inputCommandToString(int command);
