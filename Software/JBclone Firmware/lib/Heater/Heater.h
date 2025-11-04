#ifndef __HEATER_H__
#define __HEATER_H__

#include <Arduino.h>
#include "EEprom.h"
class Heater
{
private:
    //temperature constrains and lookup
    float _temp_sp_min;
    float _temp_sp_max;
    float _temp_runaway_threshold;
    float _temp_sp;
    float _temp_pv;

    // [thermocouple_voltage_uV, temperature_C]
    const static size_t _tc_cal_table_size = 10;
    float _tc_cal_table[_tc_cal_table_size][2];
    float _tc_max_voltage_setpoint;
    float _tc_gain;

    // PID loop
    float _pid_kp;

    float _pid_ki;
    float _pid_integral;

    float _pid_kd;
    float _pid_derivative_prev_e_t;
    float _pid_derivative_filter_tau;

    const float _pid_output_min = 0.0f;
    const float _pid_output_max = 1.0f;
    float _pid_TCvoltage_sp;
    float _pid_output;

    bool _sample_scheduled = false;
    volatile uint32_t _sample_Schedule_timestamp = 0;
    
    uint32_t _pid_TCvoltsge_pv_old_timestamp;
    uint32_t _pid_TCvoltage_pv_timestamp;
    float _pid_TCvoltage_pv;
    
    bool _pid_update_pending = false;

    void pid_reset();
    void pid_compute();
    void pid_sample();

    // general
    int _tc_pin;
    int _heater_pin;
    int _stand_sense_pin;
    bool _enable;

    //sleep mode
    uint32_t _sleep_delay_start_time = 0;
    bool _sleep_delay_running = false;
    float _sleep_delay = 0;
    bool _sleep_state = false;
    float _sleep_TCvoltage_set;

    // hmi
    void (*_hmi_update_function)(Heater *);
    uint32_t _hmi_last_update_timestamp = 0;

    // EEPROM
    EEprom &_memory;
    size_t _start_address;
    bool save(String &response);
    bool load_memory();

public:

    Heater(
        int tc_pin, 
        int heater_pin, 
        int stand_sense_pin,
        float tc_gain, 
        size_t start_address, 
        EEprom &eeprom, 
        void (*_hmi_update_function)(Heater *) = nullptr
    );
    void init();

    //HMI helpers
    int get_pid_op_percent();
    String get_pid_pv_t();
    String get_pid_sp_t();
    String get_state_txt();
    long get_state_color();
    String get_sleep_state_txt();


    //state control
    bool enable(String &cmd, String &response);

    //temperatuere mode set
    bool temp_set(String &cmd, String &response);
    bool temp_measure(String &cmd, String &response);
    bool temp_set_min(String &cmd, String &response);
    bool temp_set_max(String &cmd, String &response);
    bool temp_runaway_threshold(String &cmd, String &response);

    //sleep
    bool sleep_temp(String &cmd, String &response);
    bool sleep_delay(String &cmd, String &response);
    bool sleep_state(String &cmd, String &response);

    //pid
    bool pid_cli_gain(String &cmd, String &response);
    bool pid_cli_ki(String &cmd, String &response);
    bool pid_cli_kd(String &cmd, String &response);
    bool pid_derivative_filter_t(String &cmd, String &response);
    bool pid_output(String &cmd, String &response);
    bool pid_voltage_setpoint(String &cmd, String &response);
    void pid_schedule_sample();

    //thermocouple
    bool tc_cal_table(String &cmd, String &response);
    bool tc_read_voltage(String &cmd, String &response);
    float tcv_to_temp(float voltage);
    float temp_to_tcv(float temp);

    //heater
    void update();
    void update_output(float op_level);

    // EEPROM
    float* _eeprom_mapped_vars[10] = {
        &_pid_TCvoltage_sp,
        &_temp_sp_min,
        &_temp_sp_max,
        &_pid_kp,
        &_pid_ki,
        &_pid_kd,
        &_pid_derivative_filter_tau,
        &_sleep_delay,
        &_sleep_TCvoltage_set,
        &_temp_runaway_threshold,
    };

    static constexpr size_t eeprom_footprint =
        (sizeof(float) * (sizeof(_eeprom_mapped_vars) / sizeof(_eeprom_mapped_vars[0]))) +
        sizeof(_tc_cal_table);

    bool restore_default_config(String &cmd, String &response);
};


#endif
