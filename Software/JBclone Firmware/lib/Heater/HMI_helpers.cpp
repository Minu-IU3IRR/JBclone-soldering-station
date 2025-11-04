#include <Heater.h>

#define _hmi_green 34784L
#define _hmi_red 63504L

int Heater::get_pid_op_percent()
{
    return (int) round(this->_pid_output * 100.0f);
};

String Heater::get_pid_sp_t()
{
    return String(_temp_sp,0);
};

String Heater::get_pid_pv_t()
{
    return String(tcv_to_temp(this->_pid_TCvoltage_pv) ,0);
};

String Heater::get_state_txt()
{
    return this->_enable ? "ON" : "OFF";
}

long Heater::get_state_color()
{
    return this->_enable ? _hmi_green : _hmi_red;
}

String Heater::get_sleep_state_txt()
{
    return this->_sleep_state ? "SLEEP" : "";
}
