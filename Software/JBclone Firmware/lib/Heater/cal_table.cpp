#include "Heater.h"
#include "parser.h"

/**
 * @brief Thermocouple calibration table command handler.
 * 
 * This function handles the calibration table for the thermocouple. It can be used to get or set values in the table.
 * The command format is as follows:
 * - To get a value: index
 * - To set a value: index[x,y]
 * - To get the size of the table: ?
 * 
 * @param cmd The command string.
 * @param response The response string.
 * @return true if the command was successful, false otherwise.
 */
bool Heater::tc_cal_table(String &cmd, String &response)
{
    if (cmd == "?")
    {
        response = String(_tc_cal_table_size);
        return true;
    }

    // command intent
    bool isRequest = true;
    for (size_t i = 0; i < cmd.length(); ++i)
    {
        char c = cmd[i];
        if (c == '[' || c == ',' || c == ']')
        {
            isRequest = false;
            break;
        }
    }

    if (isRequest)
    {
        int index = cmd.toInt();
        if (index < 0 || index >= (int)_tc_cal_table_size)
        {
            response = "Invalid index";
            return false;
        }

        String xStr(_tc_cal_table[index][0], 2);
        String yStr(_tc_cal_table[index][1], 2);
        response = "[" + xStr + "," + yStr + "]";
        return true;
    }

    // Parse index and update x/y values
    int index = -1;
    float x = 0, y = 0;

    int bOpen = cmd.indexOf('[');
    int comma = cmd.indexOf(',', bOpen);
    int bClose = cmd.indexOf(']', comma);

    if (bOpen == -1 || comma == -1 || bClose == -1)
    {
        response = "Format must be index[x,y]";
        return false;
    }

    String indexStr = cmd.substring(0, bOpen);
    index = indexStr.toInt();

    if (index < 0 || index >= (int)_tc_cal_table_size)
    {
        response = "Invalid index";
        return false;
    }

    String xPart = cmd.substring(bOpen + 1, comma);
    String yPart = cmd.substring(comma + 1, bClose);

    if (!parseFloat(xPart, x) || !parseFloat(yPart, y))
    {
        response = "Invalid float value";
        return false;
    }

    _tc_cal_table[index][0] = x;
    _tc_cal_table[index][1] = y;

    return save(response);
}


/**
 * @brief Convert thermocouple voltage to temperature.
 * 
 * This function uses a linear interpolation method to convert the thermocouple voltage to temperature.
 * It handles extrapolation for values outside the calibration table range.
 * 
 * @param v The thermocouple voltage in microvolts.
 * @return The corresponding temperature in degrees Celsius.
 */
float Heater::tcv_to_temp(float v)
{
    float t = NAN;

    if (v <= _tc_cal_table[0][0])
    {
        // Below range — extrapolate using first two points
        float x1 = _tc_cal_table[0][0];
        float y1 = _tc_cal_table[0][1];
        float x2 = _tc_cal_table[1][0];
        float y2 = _tc_cal_table[1][1];
        float slope = (y2 - y1) / (x2 - x1);
        t = y1 + slope * (v - x1);
    }
    else if (v >= _tc_cal_table[_tc_cal_table_size - 1][0])
    {
        // Above range — extrapolate using last two points
        float x1 = _tc_cal_table[_tc_cal_table_size - 2][0];
        float y1 = _tc_cal_table[_tc_cal_table_size - 2][1];
        float x2 = _tc_cal_table[_tc_cal_table_size - 1][0];
        float y2 = _tc_cal_table[_tc_cal_table_size - 1][1];
        float slope = (y2 - y1) / (x2 - x1);
        t = y1 + slope * (v - x1);
    }
    else
    {
        for (int i = 1; i < _tc_cal_table_size; ++i)
        {
            float x1 = _tc_cal_table[i - 1][0];
            float y1 = _tc_cal_table[i - 1][1];
            float x2 = _tc_cal_table[i][0];
            float y2 = _tc_cal_table[i][1];
            if (v < x2)
            {
                float slope = (y2 - y1) / (x2 - x1);
                t = y1 + slope * (v - x1);
                break;
            }
        }
    }
    return t;
}

/**
 * @brief Convert temperature to thermocouple voltage.
 * 
 * This function uses a linear interpolation method to convert temperature to thermocouple voltage.
 * It handles extrapolation for values outside the calibration table range.
 * 
 * @param temp The temperature in degrees Celsius.
 * @return The corresponding thermocouple voltage in microvolts.
 */
float Heater::temp_to_tcv(float temp)
{
    float voltage = NAN;

    if (temp <= _tc_cal_table[0][1])
    {
        // Below range
        float x1 = _tc_cal_table[0][1];
        float y1 = _tc_cal_table[0][0];
        float x2 = _tc_cal_table[1][1];
        float y2 = _tc_cal_table[1][0];
        float slope = (y2 - y1) / (x2 - x1);
        voltage = y1 + slope * (temp - x1);
    }
    else if (temp >= _tc_cal_table[_tc_cal_table_size - 1][1])
    {
        // Above range
        float x1 = _tc_cal_table[_tc_cal_table_size - 2][1];
        float y1 = _tc_cal_table[_tc_cal_table_size - 2][0];
        float x2 = _tc_cal_table[_tc_cal_table_size - 1][1];
        float y2 = _tc_cal_table[_tc_cal_table_size - 1][0];
        float slope = (y2 - y1) / (x2 - x1);
        voltage = y1 + slope * (temp - x1);
    }
    else
    {
        //in range search
        for (int i = 1; i < _tc_cal_table_size; ++i)
        {
            float t1 = _tc_cal_table[i - 1][1];
            float t2 = _tc_cal_table[i][1];
            if (temp < t2)
            {
                float v1 = _tc_cal_table[i - 1][0];
                float v2 = _tc_cal_table[i][0];
                float slope = (v2 - v1) / (t2 - t1);
                voltage = v1 + slope * (temp - t1);
                break;
            }
        }
    }

    return voltage;
}
