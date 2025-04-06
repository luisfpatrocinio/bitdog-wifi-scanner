#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
int32_t mapValue(uint32_t value, uint32_t in_min, uint32_t in_max, int32_t out_min, int32_t out_max);

int approach(int value, int target, int step);

#endif // UTILS_H