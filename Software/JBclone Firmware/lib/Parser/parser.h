#ifndef __PARSERS_H__
#define __PARSERS_H__

#include <Arduino.h>

/**
 * @brief Parses a string to an integer.
 * @param input The string to parse.
 * @param result The parsed integer.
 * @return True if parsing was successful, false otherwise.
 * 
 * The function has this constrains:A0
 * - 10 digits before and after zeroes
 * - no exponent syntax like 1e6
 * - decimal point is represented by "."
 * 
 */
bool parseFloat(const String& input, float& result);

/**
 * @brief Parses a boolean value from a string.
 * @param in The string to parse.
 * @param result The parsed boolean value.
 * @return True if parsing was successful, false otherwise.
 * 
 * The function checks for the following string values:
 * - "1" (true)
 * - "0" (false)
 */
bool parseBool(String &in, bool& result);

#endif // __PARSERS_H__
