#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

// These data buffer acts an input for activity algorithm and expects sample values in units of mV

#define INPUT_ARRAY_SIZE 256

// Accelerometer x-axis data
int16_t input_activity_x[INPUT_ARRAY_SIZE] = {0};

// Accelerometer y-axis data
int16_t input_activity_y[INPUT_ARRAY_SIZE] = {0};

// Accelerometer z-axis data
int16_t input_activity_z[INPUT_ARRAY_SIZE] = {0};

#endif // INPUT_H