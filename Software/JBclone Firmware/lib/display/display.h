#ifndef __display_H__
#define __display_H__

#include <Arduino.h>

class Display{
private:
    HardwareSerial  &port;
    unsigned long timeout;
    
    const byte terminator = 0xFF;
    const int temrinator_legnth = 3;
    
    String internal_command_prpeamble = "xxx";
    static constexpr char cmd_pause_update = 'P';
    static constexpr char cmd_resume_update = 'R';
    bool pause_update = false;
    
    void display_command(const String command);
public:
    Display(HardwareSerial &port): port(port){};
    void init(uint32_t baud, unsigned long timeout);

    bool read(String &message);

    void text(const String target_field, String txt);   
    void value(const String target_field, int value);
    void color(const String target_field, const long color);

};
#endif