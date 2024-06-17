#ifndef DATA_PROCESSING_H_
#define DATA_PROCESSING_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

float digital_filter(float dInput, float *pIn, float *pOut, const float *pA, const float *pB, uint8_t aLength, uint8_t bLength, uint8_t bFilterOrder, bool fReset, float dInitialSample);
double ecgbr_digital_filter(double dInput, double *pIn, double *pOut, const double *pA, const double *pB, uint8_t aLength, uint8_t bLength, uint8_t bFilterOrder, bool fReset, double dInitialSample);

#endif /* DATA_PROCESSING_H_ */
