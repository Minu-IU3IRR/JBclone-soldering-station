#ifndef __EEprom_h__
#define __EEprom_h__

#include <Arduino.h>
#include <Wire.h>

class EEprom
{
private:
    uint16_t _address;
    uint16_t _SDA;
    uint16_t _SCL;
    TwoWire *_wire;

    size_t size = 2048U; // 16kb

public:
    EEprom(uint16_t address, uint16_t SDA, uint16_t SCL, TwoWire &wire);
    bool writeByte(uint16_t memAddr, uint8_t data);
    bool writeBytes(uint16_t memAddr, uint8_t* data, size_t length);
    bool readByte(uint16_t memAddr, uint8_t &data);
    bool readBytes(uint16_t memAddr, uint8_t* buffer, size_t length);
    bool writeFloat(uint16_t addr, float value);
    bool readFloat(uint16_t addr, float &value);
};

#endif