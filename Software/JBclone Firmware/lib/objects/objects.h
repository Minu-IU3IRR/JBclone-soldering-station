#ifndef __PINS_H__
#define __PINS_H__

/**
 * @file pins.h
 * @brief main configuration for the heater control system.
 * 
 * This file contains the configuration for the heater control system
 * Additionally, it defines the command handlers for the heater control system and the GPIO pin assignments.
 */

#include <Arduino.h>
#include "EEprom.h"
#include "display.h"
#include "Heater.h"


// i2c interface for EEPROM
extern TwoWire i2cBus;

// EEprom object passed to Heater instances for access
extern EEprom eeprom;

// hmi instance and update functions
extern Display _hmi;

void HMI_heater1_update(Heater *heater);
void HMI_heater2_update(Heater *heater);
void HMI_heater3_update(Heater *heater);
void HMI_heater4_update(Heater *heater);

//Heaters instantiated
extern Heater heaters[4];
extern size_t _heater_count;

// Serial commands
typedef bool (Heater::*CommandFunc)(String &cmd, String &response);

struct CommandHandler
{
	const char *name;
	CommandFunc func;
};

//table is optimized so most common commands are parsed faster
extern CommandHandler commandTable[18];
extern size_t commandTableSize;

#endif // __PINS_H__
