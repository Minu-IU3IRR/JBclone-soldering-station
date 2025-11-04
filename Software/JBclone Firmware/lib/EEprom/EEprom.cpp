#include "EEprom.h"

EEprom::EEprom(uint16_t address, uint16_t SDA, uint16_t SCL, TwoWire &wire)
    : _address(address), _SDA(SDA), _SCL(SCL), _wire(&wire)
{
}

/**
 * @brief a single byte to the EEPROM at the specified memory address.
 * @param memAddr Memory address to write to (0-2047 for 16kb EEPROM).
 * @param data Byte to write.
 * @return true if write was successful, false otherwise.
 */
bool EEprom::writeByte(uint16_t memAddr, uint8_t data)
{

    if (memAddr >= size)
        return false;

    uint8_t devAddr = _address | ((memAddr >> 8) & 0x07);
    uint8_t regAddr = memAddr & 0xFF;

    _wire->beginTransmission(devAddr);
    _wire->write(regAddr);
    _wire->write(data);

    if (_wire->endTransmission() != 0)
    {
        return false; // Initial transmission failed
    }

    // Poll for EEPROM write complete (ACK polling)
    const unsigned long timeout = 7; // milliseconds
    unsigned long start = millis();

    while (millis() - start < timeout)
    {
        _wire->beginTransmission(devAddr);
        if (_wire->endTransmission() == 0)
        {
            return true; // EEPROM responded — write completed
        }
    }

    return false; // Timed out waiting for EEPROM
}

/**
 * @brief Write multiple bytes to the EEPROM starting from the specified memory address.
 * 
 * @param memAddr Memory start address to write to (0-2047 for 16kb EEPROM).
 * @param data Pointer to the data to write.
 * @param length Number of bytes to write.
 * @return true if write was successful, false otherwise.
 */
bool EEprom::writeBytes(uint16_t memAddr, uint8_t *data, size_t length)
{
    if ((memAddr + length) > size)
        return false;

    size_t remaining = length;
    size_t offset = 0;

    while (remaining > 0)
    {
        // Calculate how many bytes we can write in the current page
        uint8_t pageStart = memAddr % 16;
        uint8_t bytesToPageEnd = 16 - pageStart;
        uint8_t chunkSize = (remaining < bytesToPageEnd) ? remaining : bytesToPageEnd;

        uint8_t devAddr = _address | ((memAddr >> 8) & 0x07);
        uint8_t regAddr = memAddr & 0xFF;

        _wire->beginTransmission(devAddr);
        _wire->write(regAddr);
        for (uint8_t i = 0; i < chunkSize; ++i)
        {
            _wire->write(data[offset + i]);
        }

        if (_wire->endTransmission() != 0)
            return false;

        // ACK polling
        const unsigned long timeout = 7;
        unsigned long start = millis();
        while (millis() - start < timeout)
        {
            _wire->beginTransmission(devAddr);
            if (_wire->endTransmission() == 0)
                break;
        }

        memAddr += chunkSize;
        offset += chunkSize;
        remaining -= chunkSize;
    }

    return true;
}

/**
 * @brief Read a single byte from the EEPROM at the specified memory address.
 * 
 * @param memAddr Memory address to read from (0-2047 for 16kb EEPROM).
 * @param data Reference to the byte to store the read data.
 * @return true if read was successful, false otherwise.
 */
bool EEprom::readByte(uint16_t memAddr, uint8_t &data)
{
    if (memAddr >= size)
        return false;

    uint8_t devAddr = _address | ((memAddr >> 8) & 0x07);
    uint8_t regAddr = memAddr & 0xFF;

    _wire->beginTransmission(devAddr);
    _wire->write(regAddr);
    if (_wire->endTransmission(false) != 0)
        return false;

    if (_wire->requestFrom(devAddr, (uint8_t)1) != 1)
        return false;

    data = _wire->read();
    return true;
}

/**
 * @brief Read multiple bytes from the EEPROM starting from the specified memory address.
 * 
 * @param memAddr Memory start address to read from (0-2047 for 16kb EEPROM).
 * @param buffer Pointer to the buffer to store the read data.
 * @param length Number of bytes to read.
 * @return true if read was successful, false otherwise.
 */
bool EEprom::readBytes(uint16_t memAddr, uint8_t *buffer, size_t length)
{
    if ((memAddr + length) > size)
        return false;

    size_t remaining = length;
    size_t offset = 0;

    while (remaining > 0)
    {
        uint8_t devAddr = _address | ((memAddr >> 8) & 0x07);
        uint8_t regAddr = memAddr & 0xFF;

        // Limit to 32 bytes per I2C read (Wire buffer size)
        uint8_t chunkSize = remaining > 32 ? 32 : remaining;

        // Send memory address
        _wire->beginTransmission(devAddr);
        _wire->write(regAddr);
        if (_wire->endTransmission(false) != 0)
            return false;

        // Request bytes
        uint8_t bytesRead = _wire->requestFrom(devAddr, chunkSize);
        if (bytesRead != chunkSize)
            return false;

        for (uint8_t i = 0; i < chunkSize; ++i)
        {
            buffer[offset + i] = _wire->read();
        }

        memAddr += chunkSize;
        offset += chunkSize;
        remaining -= chunkSize;
    }

    return true;
}

/**
 * @brief Write a float value to the EEPROM at the specified memory address.
 * 
 * @param addr Memory address to write to (0-2044 for 16kb EEPROM (-4 for size)).
 * @param value Float value to write.
 * @return true if write was successful, false otherwise.
 */
bool EEprom::writeFloat(uint16_t addr, float value)
{
    if (addr + 4 > size)
        return false;

    uint8_t bytes[4];
    memcpy(bytes, &value, 4);
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (!writeByte(addr + i, bytes[i]))
            return false;
    }
    return true;
}

/**
 * @brief Read a float value from the EEPROM at the specified memory address.
 * 
 * @param addr Memory address to read from (0-2044 for 16kb EEPROM (-4 for size)).
 * @param value Reference to the float to store the read data.
 * @return true if read was successful, false otherwise.
 */
bool EEprom::readFloat(uint16_t addr, float &value)
{
    if (addr + 4 > size)
        return false;

    uint8_t bytes[4];
    for (uint8_t i = 0; i < 4; ++i)
    {
        if (!readByte(addr + i, bytes[i]))
            return false;
    }
    memcpy(&value, bytes, 4);

    if (isnan(value))
        return false;

    return true;
}
