#ifndef _SERIAL_CONTROLS_H_
#define _SERIAL_CONTROLS_H_

#include <Arduino.h>
#include "objects.h"
#include "Heater.h"

/**
 * @brief Evaluates and executes a serial command addressed to a heater device.
 *
 * This function parses a colon-separated serial command string in the format:
 *     "id:command:value"
 *
 * - `id` must be a single-digit heater index (e.g., 0–9).
 * - `command` is matched against entries in the command table.
 * - `value` is the value to pass to the command function as text.
 *
 * If the command is valid, the corresponding Heater method is called.
 *
 * @param message        Null-terminated command string from serial input.
 * @param response       Optional output buffer for a response string.
 * @param responseSize   Size of the response buffer (in bytes).
 *
 * @return true if the command was successfully parsed and executed,
 *         false if the command was malformed, unknown, or failed to execute.
 */
bool eval_serial_command(const String message, String& response)
{

    // Find the first ':' separator (between ID and command)
    int c1 = message.indexOf(':');

    // Find the second ':' separator (between command and value)
    int c2 = message.indexOf(':', c1 + 1);

    // If either separator is missing, report malformed command
    if (c1 == -1 || c2 == -1)
    {
        response = "Malformed command. Format: id:command:value_or_?";
        return false;
    }

    // Parse device ID (only valid for single-digit IDs)
    int id = -1;
    if (isdigit(message[0]))
    {
        id = message[0] - '0';
    }

    // Validate heater index
    if (id < 0 || id >= (int)(sizeof(heaters) / sizeof(heaters[0])))
    {
        response = "Invalid device ID";
        return false;
    }

    // Extract command and value parts
    String command = message.substring(c1 + 1, c2);
    String target_cmd = message.substring(c2 + 1);

    // Resolve the target heater object
    Heater& target = heaters[id];

    // Look for the command in the command table
    for (int i = 0; i < commandTableSize; ++i)
    {
        if (command == commandTable[i].name)
        {
            return (target.*(commandTable[i].func))(target_cmd, response);
        }
    }

    // Command not recognized
    response = "Unknown command";
    return false;
}


#endif