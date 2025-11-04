#include <Heater.h>
#include "parser.h"

/**
 * @brief Sleep delay command handler.
 * 
 * This function can be used to set the sleep delay time, from tip touching stand to setpoint switched to sleep setpoint.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: sleep_delay_value (in seconds)
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::sleep_delay(String &cmd, String &response)
{
    // getter
    if (cmd == "?")
    {
        response = String(_sleep_delay);
        return true;
    }

    // try parse
    float new_value;
    bool valid = parseFloat(cmd, new_value);
    if (!valid)
    {
        response = "invalid value";
        return false;
    }

    if (new_value < 0)
    {
        response = "invalid value < 0";
        return false;
    }

    this->_sleep_delay = new_value;

    return save(response);
}

/**
 * @brief Sleep state command handler.
 * 
 * This function can be used to get the current sleep state of the heater.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: 1 (sleep mode) or 0 (normal mode)
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::sleep_state(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = this->_sleep_state ? "1" : "0";
        return true;
    }
    response = "command is read only";
    return false;
}


/**
 * @brief Sleep temperature command handler.
 * 
 * This function can be used to set the sleep temperature setpoint.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: sleep_temp_value (in degrees Celsius)
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::sleep_temp(String &cmd, String &response)
{
    // getter
    if (cmd == "?")
    {
        response = String(tcv_to_temp(_sleep_TCvoltage_set),1);
        return true;
    }

    // try parse
    float new_value;
    bool valid = parseFloat(cmd, new_value);
    if (!valid)
    {
        response = "invalid value";
        return false;
    }

    new_value = temp_to_tcv(new_value);
    if (new_value < 0.0f)
    {
        response = "value < min hardware limit";
        return false;
    }

    if (new_value > _tc_max_voltage_setpoint)
    {
        response = "value > max hardware limit";
        return false;
    }

    this->_sleep_TCvoltage_set = new_value;

    return save(response);
}

//TODO : MOVE
bool Heater::temp_runaway_threshold(String &cmd, String &response)
{
    // getter
    if (cmd == "?")
    {
        response = String(this->_temp_runaway_threshold,1);
        return true;
    }

    // try parse
    float new_value;
    bool valid = parseFloat(cmd, new_value);
    if (!valid)
    {
        response = "invalid value";
        return false;
    }

    if (new_value < 0.0f)
    {
        response = "value < 0.0";
        return false;
    }
    if (temp_to_tcv(new_value) > _tc_max_voltage_setpoint)
    {
        response = "value > max hardware limit";
        return false;
    }

    this->_temp_runaway_threshold = new_value;

    return save(response);
}
