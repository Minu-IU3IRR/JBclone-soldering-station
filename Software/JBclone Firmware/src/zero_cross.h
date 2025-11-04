#ifndef __ZERO_CROSS_H__
#define __ZERO_CROSS_H__

#include <Arduino.h>
#include "hartbeat.h"
#include "objects.h"

static int zero_cross_counter = 0;

/**
 * @brief Zero cross interrupt service routine.
 * 
 * This function is called when a zero cross event occurs.
 * Turns off heatersand samples thermocouple every _zero_cross_period cycles.
 * Updates the heater output and sets hartbeat flag every time it is called.
 * 
 * @note This function should be called in the zero cross interrupt handler.
 */
void zero_cross_isr()
{
    // flag hartbeat for update
    hartbeat_set();

    // temperature acquisituin cycle
    if (zero_cross_counter >= _zero_cross_period)
    {
        //sample
        for (int i = 0; i < _heater_count; i++)
            heaters[i].pid_schedule_sample();
        
        zero_cross_counter = 0;
        return;
    }
    
    float op_level = float(zero_cross_counter) / float(_zero_cross_period);
    for (int i = 0; i < _heater_count; i++)
        heaters[i].update_output(op_level);
    
    zero_cross_counter++;
}

#endif