#include "Heater.h"
#include "Hardware.h"
#include "parser.h"

/**
 * @brief PID proportional gain controller command handler.
 *
 * This function can be used to set the proportional gain (Kp) of the PID controller.
 * It can also be used to get the current value of Kp.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: kp_value
 *
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::pid_cli_gain(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(_pid_kp, 5); // Return scaled for user
        return true;
    }

    float val;
    if (!parseFloat(cmd, val) || val < 0.0f)
    {
        response = "invalid kp";
        return false;
    }

    _pid_kp = val; // Apply scale

    return save(response);
}
/**
 * @brief PID integral gain controller command handler.
 *
 * This function can be used to set the integral gain of the PID controller.
 * It can also be used to get the current value of Ki.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: Ki (in seconds)
 *
 * Internally, Ki is calculated as 1/Ti.
 *
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::pid_cli_ki(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(_pid_ki, 5); // Return Ti as 1/Ti
        return true;
    }

    float new_ki;
    if (!parseFloat(cmd, new_ki) || new_ki < 0.0f)
    {
        response = "invalid Ki";
        return false;
    }

    _pid_ki = new_ki; // ki = 1/Ti

    return save(response);
}

/**
 * @brief PID derivative time controller command handler.
 *
 * This function can be used to set the derivative gain of the PID controller.
 * It can also be used to get the current value of Td.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: kd
 *
 * Internally, Td is used directly without additional scaling.
 *
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::pid_cli_kd(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(_pid_kd, 5); // Return Td
        return true;
    }

    float val;
    if (!parseFloat(cmd, val) || val < 0.0f)
    {
        response = "invalid kd";
        return false;
    }

    _pid_kd = val;

    return save(response);
}

/**
 * @brief PID derivative low pass filter time constant command handler.
 *
 * This function can be used to set the time constant (t) for the low pass filter preceding the derivative term of the PID controller.
 * It can also be used to get the current value of t.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: t_value in seconds
 *
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */

bool Heater::pid_derivative_filter_t(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(this->_pid_derivative_filter_tau, 5);
        return true;
    }

    float new_value;
    bool valid = parseFloat(cmd, new_value);
    if (!valid)
    {
        response = "inavalid float value";
        return false;
    }

    this->_pid_derivative_filter_tau = new_value;

    return save(response);
}

/**
 * @brief PID output command handler.
 *
 * can be used to get the current value of the PID output.
 * The command format is as follows:
 * - To get the value: ?
 * - does not have a setter as it is read-only.
 *
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::pid_output(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(this->_pid_output, 4);
        return true;
    }

    response = "value is read only";
    return false;
}

/**
 * @brief enable or disable the heater command handeler.
 *
 * This function can be used to enable or disable the heater.
 * It can also be used to get the current state of the heater.
 * The command format is as follows:
 * - To get the value: ?
 * - To set the value: 0 or 1 (0 = off, 1 = on)
 *
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::enable(String &cmd, String &response)
{
    // getter
    if (cmd == "?")
    {
        response = this->_enable ? "1" : "0";
        return true;
    }

    // try parse
    bool new_state;
    bool valid = parseBool(cmd, new_state);
    if (!valid)
    {
        return false;
        response = "invalid value";
    }

    this->_enable = new_state;

    this->pid_reset();

    response = "OK";

    return valid;
}

/**
 * @brief resets the pid controller state.
 */
void Heater::pid_reset()
{
    _pid_integral = 0;
    _pid_derivative_prev_e_t = _pid_TCvoltage_pv;
    _pid_update_pending = false;
    _pid_output = 0;
    _pid_TCvoltsge_pv_old_timestamp = 0;
    _pid_TCvoltage_pv_timestamp = 0;
}

/**
 * @brief computes the PID output based on the current setpoint and process variable.
 *
 * This function calculates the PID output using the current setpoint and process variable.
 * It also handles integral windup protection and derivative filtering.
 *
 * @note This function should be called periodically to update the PID output.
 * @note The function skips calculation if the PID update is not pending.
 */
void Heater::pid_compute()
{
    const float dt = (_pid_TCvoltage_pv_timestamp - _pid_TCvoltsge_pv_old_timestamp) / 1e6f;

    // oveesampling skip
    if (dt < 0.001f)
    {
        _pid_update_pending = false;
        return;
    }

    // --- Setpoint selection ---
    const float sp = _sleep_state ? _sleep_TCvoltage_set : _pid_TCvoltage_sp;

    // --- Normalize PV and SP ---
    const float IOspan = _tc_max_voltage_setpoint; // Assumes min = 0
    const float sp_norm = sp / IOspan;
    const float pv_norm = _pid_TCvoltage_pv / IOspan;
    const float error = sp_norm - pv_norm;

    // --- Proportional ---
    const float p_term = _pid_kp * error;

    // --- Derivative ---
    float d_term = 0.0f;
    if (_pid_kd > 0.0f)
    {
        float derivative = 0.0f;

        if (_pid_derivative_filter_tau > 0.0f)
        {
            const float alpha = dt / (_pid_derivative_filter_tau + dt);
            const float filtered_error = alpha * error + (1.0f - alpha) * _pid_derivative_prev_e_t;
            derivative = (filtered_error - _pid_derivative_prev_e_t) / dt;
            _pid_derivative_prev_e_t = filtered_error;
        }
        else
        {
            derivative = (error - _pid_derivative_prev_e_t) / dt;
            _pid_derivative_prev_e_t = error;
        }

        d_term = _pid_kd * derivative;
    }

    // --- Integral ---
    float i_term = 0.0f;
    if (_pid_ki > 0.0f)
    {
        // Predict unconstrained control signal
        const float control_signal_unconstrained = p_term + (_pid_ki * _pid_integral) + d_term;

        // Compute anti-windup correction (difference between saturated and unconstrained output)
        const float aw_correction = (_pid_output - control_signal_unconstrained);

        // Anti-windup gain (can be tuned, start with 1.0)
        const float Kb = 1.0f;

        // Integrate error + back-calculate anti-windup correction
        _pid_integral += (error + Kb * aw_correction) * dt;

        // Clamp integral term for safety
        const float i_max = _pid_output_max / _pid_ki;
        const float i_min = _pid_output_min / _pid_ki;
        _pid_integral = constrain(_pid_integral, i_min, i_max);

        // Compute integral contribution
        i_term = _pid_ki * _pid_integral;
    }

    // --- Compute total control output ---
    const float control_signal = p_term + i_term + d_term;
    _pid_output = constrain(control_signal, _pid_output_min, _pid_output_max);
}

/**
 * @brief sets a flag that trigegrs an update on next update loop
 *
 * Turns output low to remove leads dV from thermocuple voltage
 */
void Heater::pid_schedule_sample()
{
    digitalWrite(this->_heater_pin, LOW);
    this->_sample_scheduled = true;
    this->_sample_Schedule_timestamp = micros();
}

/**
 * @brief samples the thermocouple voltage and updates the process variable (PV).
 *
 * This function reads the thermocouple voltage, converts it to temperature, and updates the PV.
 * It also checks for runaway conditions and disables the heater if necessary.
 *
 * @note This function should be called periodically to update the PID process variable.
 */
void Heater::pid_sample()
{
    float adc_reading_bits = analogRead(this->_tc_pin);
    float adc_voltage = (adc_reading_bits / ADC_RES) * ADC_VREF;
    float tc_voltage_volts = adc_voltage / this->_tc_gain;
    this->_pid_TCvoltage_pv = tc_voltage_volts * 1e6f; // Convert to µV as unit
    this->_temp_pv = tcv_to_temp(this->_pid_TCvoltage_pv);

    this->_pid_TCvoltsge_pv_old_timestamp = _pid_TCvoltage_pv_timestamp;
    this->_pid_TCvoltage_pv_timestamp = micros();

    // set update available flag
    this->_pid_update_pending = true;

    // runaway protection
    bool runaway = false;
    runaway |= this->_temp_pv > this->_temp_runaway_threshold; // temperatue threshold
    runaway |= adc_reading_bits >= ADC_RES;                    // ADC saturation
    if (runaway)
    {
        this->_enable = false;
        this->pid_reset();
        digitalWrite(this->_heater_pin, LOW);
    }
}
