#include "parser.h"

/**
 * @brief Parses a string to a float value.
 * 
 * This function attempts to convert a string representation of a float into an actual float value.
 * It handles optional signs, decimal points, and ensures that the number of digits before and after the decimal point does not exceed 10.
 * 
 * @param input The string to parse.
 * @param result The resulting float value.
 * @return true if the parsing was successful, false otherwise.
 */
bool parseFloat(const String& input, float& result) {
    const char* str = input.c_str();
    result = 0.0f;
    bool isNegative = false;
    bool seenDot = false;
    int digitCountBefore = 0;
    int digitCountAfter = 0;
    float fractional = 0.0f;
    float factor = 0.1f;

    // Handle optional sign
    if (*str == '-') {
        isNegative = true;
        ++str;
    } else if (*str == '+') {
        ++str;
    }

    // Must start with a digit or a dot
    if (!isdigit(*str) && *str != '.') return false;

    while (*str) {
        if (*str == '.') {
            if (seenDot) return false; // Only one dot allowed
            seenDot = true;
            ++str;
            continue;
        }

        if (!isdigit(*str)) return false;

        int digit = *str - '0';

        if (!seenDot) {
            if (digitCountBefore >= 10) return false;
            result = result * 10.0f + digit;
            digitCountBefore++;
        } else {
            if (digitCountAfter >= 10) return false;
            fractional += digit * factor;
            factor *= 0.1f;
            digitCountAfter++;
        }

        ++str;
    }

    result += fractional;
    if (isNegative) result = -result;
    return true;
}

/**
 * @brief Parses a string to a boolean value.
 * 
 * This function attempts to convert a string representation of a boolean into an actual boolean value.
 * It recognizes "1" as true and "0" as false.
 * 
 * @param in The string to parse.
 * @param result The resulting boolean value.
 * @return true if the parsing was successful, false otherwise.
 */
bool parseBool(String &in, bool &result)
{
    bool a = in == "1";
    bool b = in == "0";
    
    if(!a && !b)
        return false;
    
    result = a;
    return true;
}
