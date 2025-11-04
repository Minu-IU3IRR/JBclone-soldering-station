#include <Arduino.h>
#include "Hardware.h"
#include "objects.h"
#include "zero_cross.h"
#include "hartbeat.h"
#include "Serial_controls.h"
#include "display.h"

void setup()
{

    // pheripheral init
    _serial_usb.begin(_serial_usb_baud);
    _serial_usb.setTimeout(_serial_usb_timeout);

    i2cBus.begin();

    _hmi.init(_serial_hmi_baud, _serial_hmi_timeout);

    // board init
    analogReadResolution(ADC_BITS);
    attachInterrupt(digitalPinToInterrupt(_pin_zero_cross), zero_cross_isr, RISING);
    pinMode(_pin_hartbeat, OUTPUT);
    hartbeat_set();

    // heaters init
    for (int i = 0; i < _heater_count; i++)
        heaters[i].init();
}

void loop()
{
    // hartbear routine
    update_hartbeat();

    // heater update
    for (int i = 0; i < _heater_count; i++)
    {
        heaters[i].update();
    }

    // interfaces
    String message;
    String response;

    if (_serial_usb.available() > 0)
    {
        message = _serial_usb.readStringUntil(_serial_usb_terminator);
        bool success = eval_serial_command(message, response);

        if (!success)
            _serial_usb.print("ERROR ");

        if (response.length())
            _serial_usb.print(response);

        if (!success || response.length() > 0)
            _serial_usb.print(_serial_usb_terminator);
    }

    bool hmi_message = _hmi.read(message);
    if (hmi_message)
    {
        eval_serial_command(message, response);
    }
}
