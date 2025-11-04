#include "Heater.h"
#include "parser.h"

/**
 * @brief pid thermocouple voltage direct setpoint command handler.
 * 
 * This function can be used to set the thermocouple voltage setpoint (SP) of the heater directly.
 * It can also be used to get the current value of it.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: voltage_value (in microvolts)
 * 
 * @note The voltage setpoint is converted to temperature using the tcv_to_temp function, but the loop resons un uV.
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::pid_voltage_setpoint(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(_pid_TCvoltage_sp, 5);
        return true;
    }

    float voltage;
    if (!parseFloat(cmd, voltage))
    {
        response = "invalid float value";
        return false;
    }
    if (voltage < 0.0f)
    {
        response = "voltage < 0.0";
        return false;
    }
    if(voltage >_tc_max_voltage_setpoint){
        response = "voltage > max hardware tcv setpoint";
        return false;
    }
    
    _pid_TCvoltage_sp = voltage;

    _temp_sp = tcv_to_temp(voltage);
    
    return save(response);
}

/**
 * @brief pid thermocouple voltage readback command handler.
 * 
 * This function can be used to get the current thermocouple voltage (PV) of the heater.
 * The command format is as follows:
 * - To get the value: ?
 * - does not have a setter as it is read-only.
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::tc_read_voltage(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(this->_pid_TCvoltage_pv, 5);
        return true;
    }

    response = "value is read only";
    return false;
}