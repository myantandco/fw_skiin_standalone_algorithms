#ifndef DATA_PROCESSING_H_
#define DATA_PROCESSING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

float digital_filter(float input, float *in, float *out, const float *a, const float *b, uint8_t a_len, uint8_t b_len, uint8_t filter_order, bool reset, float initial_sample);

#endif /* DATA_PROCESSING_H_ */
