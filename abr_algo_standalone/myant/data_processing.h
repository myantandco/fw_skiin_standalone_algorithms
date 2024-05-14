#ifndef DATA_PROCESSING_H_
#define DATA_PROCESSING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


double digital_filter(double input, double *in, double *out, const double *a, const double *b, uint8_t a_len, uint8_t b_len, uint8_t filter_order, bool reset, double initial_sample);

#endif /* DATA_PROCESSING_H_ */
