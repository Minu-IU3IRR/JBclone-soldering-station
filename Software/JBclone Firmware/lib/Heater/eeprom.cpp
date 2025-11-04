#include "Heater.h"
#include "parser.h"

/**
 * @brief save the current tipconfiguration and calibration to memory
 * 
 * This function saves the current configuration and calibration data to EEPROM memory.
 * It writes the values of the mapped variables and the thermocouple calibration table to the specified memory address.
 * Assignes to the passed response string the result of the operation as "OK" or "FAIL TO SAVE".
 * 
 * @return true if the operation was successful, false otherwise.
 */
bool Heater::save(String &response)
{
    constexpr float epsilon = 0.0001f;
    size_t addr = _start_address;
    float stored;

    bool good_op = true;

    for (size_t i = 0; i < sizeof(_eeprom_mapped_vars) / sizeof(_eeprom_mapped_vars[0]); ++i)
    {
        good_op &= _memory.writeFloat(addr, *(_eeprom_mapped_vars[i]));
        addr += sizeof(float);
    }

    for (int i = 0; i < _tc_cal_table_size; i++)
    {
        good_op &= _memory.writeFloat(addr, _tc_cal_table[i][0]);
        addr += sizeof(float);

        good_op &= _memory.writeFloat(addr, _tc_cal_table[i][1]);
        addr += sizeof(float);
    }

    response = good_op ? "OK" : "FAIL TO SAVE";
    return good_op;
}

/**
 * @brief load the current tipconfiguration and calibration from memory
 * 
 * This function loads the configuration and calibration data from EEPROM memory.
 * 
 * @return true if the operation was successful, false otherwise.
 */
bool Heater::load_memory()
{
    size_t addr = _start_address;
    bool good_op = true;

    constexpr size_t num_vars = sizeof(_eeprom_mapped_vars) / sizeof(_eeprom_mapped_vars[0]);
    for (size_t i = 0; i < num_vars; ++i)
    {
        good_op &= _memory.readFloat(addr, *(_eeprom_mapped_vars[i]));
        addr += sizeof(float);
    }

    for (int i = 0; i < _tc_cal_table_size; i++)
    {
        good_op &= _memory.readFloat(addr, _tc_cal_table[i][0]);
        addr += sizeof(float);

        good_op &= _memory.readFloat(addr, _tc_cal_table[i][1]);
        addr += sizeof(float);
    }

    if (good_op)
        _temp_sp = tcv_to_temp(_pid_TCvoltage_sp);

    return good_op;
}

/**
 * * @brief Restore default configuration and calibration values.
 * 
 * This function restores the default configuration and calibration values for the heater.
 * The default values are:
 * - Thermocouple S[uV/K]: 0.0 to 40.0
 * - Temperature setpoint range: 100.0 to 400.0
 * - PID gains: kp = 0.0, ti = 0.0, td = 0.0
 * - Derivative filter time constant: 0.25s
 * - Sleep delay: 30s
 * - Sleep temperature setpoint: 150.0C
 * - Thermocouple calibration table: linear interpolation between 0 and 450C
 * @param cmd The command string containing the thermocouple S[uV/K] value.
 * @param response The response string to be sent back to the caller.
 * @return true if the operation was successful, false otherwise.
 * @note The function also saves the new configuration to EEPROM memory.
 * @warning The function will overwrite the current configuration and calibration values.
 */
bool Heater::restore_default_config(String &cmd, String &response)
{
    float tc_s;
    bool valid = parseFloat(cmd, tc_s);
    if (!valid)
    {
        response = "invalid thermocouple S[uV/K]";
        return false;
    }

    if (tc_s <= 0 || tc_s > 40.0f)
    {
        response = "S[uV/K] outside of range";
        return false;
    }

    _pid_TCvoltage_sp = tc_s;
    _temp_sp_min = 100.0f;
    _temp_sp_max = 400.0f;

    _pid_kp = 0.0f;
    _pid_ki = 0.0f;
    _pid_kd = 0.0f;
    _pid_derivative_filter_tau = 0.25f;

    _sleep_delay = 30000.0f; // 30s
    _sleep_TCvoltage_set = temp_to_tcv(150.0f); // 100C
    _temp_runaway_threshold = 480.0f; // 480C


    for (int i = 0; i < _tc_cal_table_size; i++)
    {
        float temp = 450.0f * i / (_tc_cal_table_size - 1);
        _tc_cal_table[i][0] = temp * tc_s;
        _tc_cal_table[i][1] = temp;
    }

    return save(response);
}
