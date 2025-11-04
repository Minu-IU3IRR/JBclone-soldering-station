#include "Heater.h"
#include "Hardware.h"
#include "parser.h"

/**
 * @brief constructor for the Heater class.
 *
 * @param tc_pin The pin number for the thermocouple (MUST BE ANALOG).
 * @param heater_pin The pin number for the heater.
 * @param stand_sense_pin The pin number for the stand sense (LOW = on stand).
 * @param tc_gain The gain of the thermocouple.
 * @param start_address The starting address for EEPROM storage, occupancy is in public const variable eeprom_footprint.
 * @param eeprom The EEPROM object for memory operations.
 * @param _hmi_update_function The function to update the HMI (Human-Machine Interface).
 * @note The constructor does not initialize the heater; call init() after creating the object.
 */
Heater::Heater(int tc_pin,
               int heater_pin,
               int stand_sense_pin,
               float tc_gain,
               size_t start_address,
               EEprom &eeprom,
               void (*_hmi_update_function)(Heater *)) : _memory(eeprom),
                                                         _hmi_update_function(_hmi_update_function)
{
    this->_tc_pin = tc_pin;
    this->_heater_pin = heater_pin;
    this->_stand_sense_pin = stand_sense_pin;
    this->_tc_gain = tc_gain;
    this->_tc_max_voltage_setpoint = ADC_VREF * 1e6f / _tc_gain; // to uV
    this->_start_address = start_address;
    this->_enable = false;
}

/**
 * @brief Initializes the heater by setting pin modes and loading memory.
 *
 * This function should be called after creating the Heater object.
 */
void Heater::init()
{
    pinMode(_tc_pin, INPUT_ANALOG);
    pinMode(_heater_pin, OUTPUT);
    digitalWrite(_heater_pin, LOW); // Turn off heater by default
    pinMode(_stand_sense_pin, INPUT);

    load_memory();

    pid_reset();
}

/**
 * @brief updates the heater state and performs periodic tasks.
 *
 * This function should be called periodically to update the heater state, perform PID calculations,
 * and handle sleep mode detection.
 */
void Heater::update()
{
    // sample scheduled
    if (_sample_scheduled && micros() - _sample_Schedule_timestamp > _tc_amp_recovery_time)
    {
        this->pid_sample();
        //skip first sample to avoi zero dT
        if(this->_pid_TCvoltsge_pv_old_timestamp != 0)
            _sample_scheduled = false;
    }

    // pid otuptu compute
    if (_pid_update_pending && _enable)
    {
        this->pid_compute();
        _pid_update_pending = false;
    }

    // update hmi values
    if (_hmi_update_function != nullptr)
    {
        constexpr uint32_t _hmi_update_interval = 200;
        uint32_t now = millis();
        if (now - this->_hmi_last_update_timestamp > _hmi_update_interval)
        {
            _hmi_update_function(this);
            _hmi_last_update_timestamp = now;
        }
    }

    // stand detection and rest condition
    if (this->_enable)
    {
        if (digitalRead(this->_stand_sense_pin) == LOW) // iron placed on thand
        {
            if (!_sleep_delay_running && !_sleep_state) // sleep setpoint delay
            {
                _sleep_delay_start_time = millis();
                _sleep_delay_running = true;
            }
            else if (millis() - _sleep_delay_start_time > _sleep_delay) // delay elapsed, sleep triggered
            {
                _sleep_state = true;
                _sleep_delay_running = false;
            }
        }
        else // iron not on stand
        {
            _sleep_state = false;
            _sleep_delay_running = false;
        }
    }
}

/**
 * @brief updates the heater output based on the PID output level.
 *
 * This function sets the heater pin to HIGH or LOW based on the PID output level and enable state.
 * uses Time Proportional Output (TPO) — more precisely, Zero-Cross Burst Firing.
 *
 * @param op_level The output level from the PID controller.
 */
void Heater::update_output(float op_level)
{
    bool output_state = this->_enable;            // output can be high only if otuput is enabled
    output_state &= !_sample_scheduled;            // keep output low until new sample is acquired if sample has been flagged
    output_state &= op_level < this->_pid_output; // output level value
    digitalWrite(_heater_pin, output_state ? HIGH : LOW);
}