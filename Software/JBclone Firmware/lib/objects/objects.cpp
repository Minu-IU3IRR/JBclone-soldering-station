#include "objects.h"
#include "Hardware.h"

TwoWire i2cBus(_pin_wire_sda, _pin_wire_scl);
EEprom eeprom(_address_eeprom, _pin_wire_sda, _pin_wire_scl, i2cBus);
Display _hmi(_serial_hmi);

void HMI_heater1_update(Heater *heater)
{
	_hmi.text("h1meas", heater->get_pid_pv_t());
	_hmi.text("h1set", heater->get_pid_sp_t());

	_hmi.value("h1op", heater->get_pid_op_percent());

	_hmi.text("h1en", heater->get_state_txt());
	_hmi.color("h1en", heater->get_state_color());

	_hmi.text("h1slp", heater->get_sleep_state_txt());
}


void HMI_heater2_update(Heater *heater)
{
	_hmi.text("h2meas", heater->get_pid_pv_t());
	_hmi.text("h23set", heater->get_pid_sp_t());

	_hmi.value("h2op", heater->get_pid_op_percent());

	_hmi.text("h23en", heater->get_state_txt());
	_hmi.color("h23en", heater->get_state_color());

	_hmi.text("h23slp", heater->get_sleep_state_txt());
}


void HMI_heater3_update(Heater *heater)
{
	_hmi.text("h3meas", heater->get_pid_pv_t());

	_hmi.value("h3op", heater->get_pid_op_percent());
}

void HMI_heater4_update(Heater *heater)
{
	_hmi.text("h4meas", heater->get_pid_pv_t());
	_hmi.text("h4set", heater->get_pid_sp_t());

	_hmi.value("h4op", heater->get_pid_op_percent());

	_hmi.text("h4en", heater->get_state_txt());
	_hmi.color("h4en", heater->get_state_color());

	_hmi.text("h4slp", heater->get_sleep_state_txt());
}

//Heaters instantiated
Heater heaters[4] = {
	Heater(_board1_temp, _board1_heater, _board1_stand, _board1_tc_gain, _board1_mem_addr, eeprom, &HMI_heater1_update),
	Heater(_board2_temp, _board2_heater, _board2_stand, _board2_tc_gain, _board2_mem_addr, eeprom, &HMI_heater2_update),
	Heater(_board3_temp, _board3_heater, _board3_stand, _board3_tc_gain, _board3_mem_addr, eeprom, &HMI_heater3_update),
	Heater(_board4_temp, _board4_heater, _board4_stand, _board4_tc_gain, _board4_mem_addr, eeprom, &HMI_heater4_update),
};

size_t _heater_count = sizeof(heaters) / sizeof(heaters[0]);


CommandHandler commandTable[] = {
	{"en", &Heater::enable},
	{"set_t", &Heater::temp_set},
	{"meas_t", &Heater::temp_measure},
	{"meas_uv", &Heater::tc_read_voltage},
	{"sleep_state", &Heater::sleep_state},
	{"pid_op", &Heater::pid_output},
	{"runaway_t", &Heater::temp_runaway_threshold},
	{"set_min_t", &Heater::temp_set_min},
	{"set_max_t", &Heater::temp_set_max},
	{"set_uv", &Heater::pid_voltage_setpoint},
	{"pid_kp", &Heater::pid_cli_gain},
	{"pid_ki", &Heater::pid_cli_ki},
	{"pid_kd", &Heater::pid_cli_kd},
	{"pid_d_tau", &Heater::pid_derivative_filter_t},
	{"sleep_set_t", &Heater::sleep_temp},
	{"sleep_delay", &Heater::sleep_delay},
	{"tc_cal_table", &Heater::tc_cal_table},
	{"restore", &Heater::restore_default_config},
};

size_t commandTableSize = sizeof(commandTable) / sizeof(commandTable[0]);