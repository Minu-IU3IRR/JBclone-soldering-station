#include "Heater.h"
#include "parser.h"

/**
 * @brief temperature setpoint command handler.
 * 
 * This function can be used to set the temperature setpoint (SP) of the heater.
 * It can also be used to get the current value of SP.
 * 
 * The command can be used as follows:
 * - To get the value: ?
 * - To set the value: temperature_value (in degrees Celsius)
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::temp_set(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(_temp_sp, 2);
        return true;
    }

    float temp;
    if (!parseFloat(cmd, temp))
    {
        response = "invalid float value";
        return false;
    }

    if (temp < _temp_sp_min || temp > _temp_sp_max)
    {
        response = "out of bounds";
        return false;
    }

    _temp_sp = temp;
    _pid_TCvoltage_sp = temp_to_tcv(temp);
    
    return save(response);
}

/**
 * @brief minimum temperature setpoint command handler.
 * 
 * This function can be used to set the minimum temperature setpoint (SP) of the heater.
 * It can also be used to get the current value of it.
 * The commmand format is as follows:
 * - To get the value: ?
 * - To set the value: temperature_value (in degrees Celsius)
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::temp_set_min(String &cmd, String &response)
{
    // getter
    if (cmd == "?")
    {
        response = String(this->_temp_sp_min, 0);
        return true;
    }

    // parse
    float new_value;
    bool valid = parseFloat(cmd, new_value);
    if (!valid)
    {
        response = "invalid float value";
        return false;
    }

    // valid check
    if (new_value > _temp_sp_max)
    {
        response = "max < min";
        return false;
    }
    if (new_value < 0.0f)
    {
        response = "value < 0.0";
        return false;
    }

    // apply
    _temp_sp_min = new_value;

    return save(response);
}

/**
 * @brief maximum temperature setpoint command handler.
 * 
 * This function can be used to set the maximum temperature setpoint (SP) of the heater.
 * It can also be used to get the current value of it.
 * The commmand format is as follows:
 * - To get the value: ?
 * - To set the value: temperature_value (in degrees Celsius)
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::temp_set_max(String &cmd, String &response)
{
    // getter
    if (cmd == "?")
    {
        response = String(this->_temp_sp_max, 0);
        return true;
    }

    // parse
    float new_value;
    bool valid = parseFloat(cmd, new_value);
    if (!valid)
    {
        response = "invalid float value";
        return false;
    }

    // valid check
    if (new_value < _temp_sp_min)
    {
        response = "min > max";
        return false;
    }
    //constraint to max hardware setpoint
    if (temp_to_tcv(new_value) > _tc_max_voltage_setpoint)
    {
        response = "temperature exceeds hardware capability";
        return false;
    }

    // apply
    _temp_sp_max = new_value;
    
    return save(response);
}

/**
 * @brief temperature read command handler.
 * 
 * This function can be used to read the current temperature (PV) of the heater.
 * It can also be used to get the current value of it.
 * The command format is as follows:
 * - To get the value: ?
 * - does not have a setter as it is read-only.
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::temp_measure(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(this->_temp_pv, 2);
        return true;
    }

    response = "command is read only";
    return false;
}