/**
 * @file analog.c
 * @brief Implementation for the analog input module.
 *
 * This module handles reading analog values from the analog stick
 * and applying a deadzone.
 */

#include "analog.h"
#include <stdio.h>

/** @brief Store for last axis value - X*/
int analog_x = 0;
/** @brief Store for last axis value - Y */
int analog_y = 0;

/**
 * @brief Initializes the analog inputs and button.
 *
 * This function sets up the ADC (Analog-to-Digital Converter) and configures
 * the GPIO (General-Purpose Input/Output) pins for the analog inputs and button.
 * It initializes the ADC, sets up the GPIO pins for the analog X and Y inputs,
 * and configures the button pin as an input with a pull-up resistor.
 */
void initAnalog()
{
    adc_init();
    adc_gpio_init(ANALOG_X);
    adc_gpio_init(ANALOG_Y);
    gpio_init(ANALOG_BTN);
    gpio_set_dir(ANALOG_BTN, GPIO_IN);
    gpio_pull_up(ANALOG_BTN);
}

/**
 * @brief Reads analog Y axis value.
 *
 * This function select the gpio that represents the axis Y, reads the adc value,
 * map the value from a range to another and after it apply a threshold to reduce sensibility
 *
 * @return The calibrated analog value for the Y axis, with deadzone applied.
 * @note The ANALOG_Y definition must be a valid ADC pin. This is a axis that must be inverted because of the analog system
 */
int32_t readAnalogY()
{
    adc_select_input(0);
    uint32_t adc_value = adc_read();
    int32_t mapped_value = mapValue(adc_value, 0, 4095, -ANALOG_MAX_VALUE, ANALOG_MAX_VALUE);
    int32_t inverted_value = -mapped_value;
    return applyThreshold(inverted_value);
}

/**
 * @brief Reads analog X axis value.
 *
 * This function select the gpio that represents the axis X, reads the adc value,
 * map the value from a range to another and after it apply a threshold to reduce sensibility
 *
 * @return The calibrated analog value for the X axis, with deadzone applied.
 * @note The ANALOG_X definition must be a valid ADC pin.
 */
int32_t readAnalogX()
{
    adc_select_input(1);
    uint32_t adc_value = adc_read();
    int32_t mapped_value = mapValue(adc_value, 0, 4095, -ANALOG_MAX_VALUE, ANALOG_MAX_VALUE);
    return applyThreshold(mapped_value);
}

/**
 * @brief Applies a deadzone threshold to a value.
 *
 * This function checks if the given value is within the defined deadzone range.
 * If it is, the function returns 0, otherwise, it returns the original value.
 *
 * @param value The value to apply the threshold to.
 * @return The original value if outside the deadzone, 0 otherwise.
 */
int32_t applyThreshold(int32_t value)
{
    if (value > DEADZONE || value < -DEADZONE)
    {
        return value;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Updates analog axis values.
 *
 * Reads current value to both axis in the system.
 */
void updateAxis()
{
    analog_x = readAnalogX();
    analog_y = readAnalogY();
}