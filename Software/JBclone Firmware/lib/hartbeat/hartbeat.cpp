#include "hartbeat.h"
#include "Hardware.h"


volatile bool hartbeat_set_flag = false;
bool hartbeat_output = LOW;
uint32_t hartbeat_rise_timestamp;

void hartbeat_set()
{
    hartbeat_set_flag = true;
}

void update_hartbeat()
{
    bool changed = false;

    //set
    if(hartbeat_set_flag)
    {
        hartbeat_set_flag = false;
        hartbeat_output = HIGH;
        hartbeat_rise_timestamp = micros();
        changed = true;
    }

    //reset
    else if(micros() - hartbeat_rise_timestamp > _hartbeat_pulse_width)
    {
        hartbeat_output = LOW;
        changed = true;
    }

    //write
    if(changed)
        digitalWrite(_pin_hartbeat, hartbeat_output);
}
