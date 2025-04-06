/**
 * @file analog.h
 * @brief Header file for the analog input module.
 */

#ifndef ANALOG_H
#define ANALOG_H

#include "utils.h"
#include "hardware/adc.h"

/** @brief GPIO pin connected to the analog X input. */
#define ANALOG_X 27
/** @brief GPIO pin connected to the analog Y input. */
#define ANALOG_Y 26
/** @brief GPIO pin connected to the analog button. */
#define ANALOG_BTN 22

/** @brief Maximum value for analog input (used for mapping). */
#define ANALOG_MAX_VALUE 5

/** @brief Deadzone threshold for analog inputs. */
#define DEADZONE 2

/** @brief Last read value for axis X. */
extern int analog_x;
/** @brief Last read value for axis Y. */
extern int analog_y;

/** @brief Initializes the analog inputs and button. */
void initAnalog();

/** @brief Reads analog Y axis value. */
int32_t readAnalogY();

/** @brief Reads analog X axis value. */
int32_t readAnalogX();

/** @brief Applies a deadzone threshold to a value. */
int32_t applyThreshold(int32_t value);

/** @brief Updates analog axis values. */
void updateAxis();

#endif // ANALOG_H