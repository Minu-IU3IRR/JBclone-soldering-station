#ifndef __HARDWARE_DEFINITION_H__
#define __HARDWARE_DEFINITION_H__

#include <Arduino.h>
#include <Wire.h>
#include "EEprom.h"
#include "Heater.h"

constexpr int ADC_BITS = 12;
constexpr float ADC_RES = 4096.0f; 
constexpr float ADC_VREF = 3.3f;
constexpr uint32_t _hartbeat_pulse_width = 5000; // us
constexpr uint32_t _tc_amp_recovery_time = 1700; //us

constexpr int _pin_hartbeat = PB15;
constexpr int _pin_zero_cross = PA8;

//100% control output half waves 
constexpr unsigned int _zero_cross_period  = 10;

// bus connecions
constexpr int _pin_wire_sda = PB11;
constexpr int _pin_wire_scl = PB10;

//eeprom
constexpr unsigned int _address_eeprom = 0x50;

// serial
#define _serial_hmi Serial1
constexpr uint32_t _serial_hmi_baud = 115200;
constexpr unsigned long _serial_hmi_timeout = 20; // ms
constexpr uint32_t  _hmi_update_interval = 500; // ms

#define _serial_usb Serial
constexpr uint32_t _serial_usb_baud = 152000;
constexpr unsigned long _serial_usb_timeout = 20; // ms
constexpr char _serial_usb_terminator = '\n';

// board specific
constexpr int _board1_temp = PA2;
constexpr int _board1_heater = PB9;
constexpr float _board1_tc_gain = 200.0f;
constexpr int _board1_stand = PB3;
constexpr size_t _board1_mem_addr = Heater::eeprom_footprint * 0;

constexpr int _board2_temp = PA3;
constexpr int _board2_heater = PB8;
constexpr float _board2_tc_gain = 400.0f;
constexpr int _board2_stand = PB4;
constexpr size_t _board2_mem_addr = Heater::eeprom_footprint * 1;

constexpr int _board3_temp = PA0;
constexpr int _board3_heater = PB7;
constexpr float _board3_tc_gain = 400.0f;
constexpr int _board3_stand = PB5;
constexpr size_t _board3_mem_addr = Heater::eeprom_footprint * 2;

constexpr int _board4_temp = PA1;
constexpr int _board4_heater = PB6;
constexpr float _board4_tc_gain = 400.0f;
constexpr int _board4_stand = _board3_stand;
constexpr size_t _board4_mem_addr = Heater::eeprom_footprint * 3;

// gpio
constexpr int _pin_gpio1 = PA4;
constexpr int _pin_gpio2 = PA5;
constexpr int _pin_gpio3 = PA6;
constexpr int _pin_gpio4 = PA7; // TODO: CHECK
constexpr int _pin_gpio5 = PB0;

#endif