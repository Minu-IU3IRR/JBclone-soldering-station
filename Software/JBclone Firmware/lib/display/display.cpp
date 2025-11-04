#include "display.h"

void Display::display_command(const String command)
{
    if (pause_update)
        return;
    port.write(command.c_str());
    port.write(terminator);
    port.write(terminator);
    port.write(terminator);
}

void Display::init(uint32_t baud, unsigned long timeout)
{
    this->port.begin(baud);
    this->timeout = timeout;
    this->port.setTimeout(timeout);
}

bool Display::read(String &message)
{
    if (port.available() == 0)
        return false;

    // data in buffer
    String received_data = "";
    int terminator_counter = 0;
    bool terminator_found = false;
    long start_time = millis();

    while (millis() - start_time < timeout)
    {
        if (port.available() > 0)
        {
            char incoming = port.read();
            received_data += incoming;

            // terminator sequence counter
            if (incoming == terminator)
                terminator_counter++;
            else
                terminator_counter = 0;

            // positive exit condition
            if (terminator_counter == temrinator_legnth)
            {
                terminator_found = true;
                break;
            }
        }
    }

    // data not parsed
    if (!terminator_found)
        return false;

    // remove terminator
    received_data = received_data.substring(0, received_data.length() - temrinator_legnth);
    bool is_internal_command = received_data.startsWith(internal_command_prpeamble);
    
    //normal message received
    if (!is_internal_command)
    {
        message = received_data;
        return true;
    }

    received_data = received_data.substring(internal_command_prpeamble.length());
    char command = received_data[0];

    switch (command)
    {
    case cmd_pause_update:
        pause_update = true;
        break;
    case cmd_resume_update:
        pause_update = false;
        break;
    }

    return false; // internal command does not trigger valid respoonse
}

void Display::text(const String target_field, String txt)
{
    String command = target_field + ".txt=\"" + txt + "\"";
    display_command(command);
}

void Display::value(const String target_field, int value)
{
    String command = target_field + ".val=" + String(static_cast<int>(value));
    display_command(command);
}

void Display::color(const String target_field, const long color)
{
    String command = target_field + ".pco=" + String(color);
    display_command(command);
}