#include "utils.h"

/**
 * @brief Maps a value from one range to another.
 *
 * This function takes a value within a specified input range and maps it to a corresponding value within a specified output range.
 *
 * @param value The value to be mapped.
 * @param in_min The minimum value of the input range.
 * @param in_max The maximum value of the input range.
 * @param out_min The minimum value of the output range.
 * @param out_max The maximum value of the output range.
 * @return The mapped value within the output range.
 */
int32_t mapValue(uint32_t value, uint32_t in_min, uint32_t in_max, int32_t out_min, int32_t out_max)
{
    return (int32_t)((value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}

int approach(int value, int target, int step)
{
    if (value < target)
    {
        value += step;
        if (value > target)
            value = target;
    }
    else if (value > target)
    {
        value -= step;
        if (value < target)
            value = target;
    }
    return value;
}