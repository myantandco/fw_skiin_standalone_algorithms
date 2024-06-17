#include "data_processing.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

float digital_filter(float dInput, float *pIn, float *pOut, const float *pA, const float *pB, uint8_t aLength, uint8_t bLength, uint8_t bFilterOrder, bool fReset, float dInitialSample)
{
    float output = 0.0f;
    float tmp    = 0.0f;

    // 1) Check arguments - pointers
    if ((pA == NULL) || (pB == NULL) || (pIn == NULL) || (pOut == NULL))
    {
        return 0;
    }

    // 2) Check arguments - lengths
    if ((bFilterOrder == 0) || (aLength == 0) || (bLength == 0) || (aLength > bFilterOrder) || (bLength > bFilterOrder))
    {
        return 0;
    }

    // 3) If fReset was set, modify input data
    if (fReset)
    {
        for (uint8_t i = 0; i < bFilterOrder; i++)
        {
            pIn[i]  = dInitialSample;   // Initialize input buffer with initial sample
            pOut[i] = 0;                // Initialize output buffer with zero
        }
    }
    pIn[bFilterOrder - 1] = dInput;     // Place new input sample at the end of input buffer

    // 4) Apply the feedforward coefficients to the input samples 
    for (uint8_t i = 0; i < bLength; i++)
    {
        tmp += pB[i] * pIn[bFilterOrder - 1 - i];
    }

    // 5) Apply the feedback coefficients to the output samples
    for (uint8_t i = 1; i < aLength; i++)
    {
        tmp -= pA[i] * pOut[bFilterOrder - 1 - i];
    }

    // 6) Normalize by the first feedback coefficient 
    tmp /= pA[0];
    output = tmp;
    pOut[bFilterOrder - 1]  = output;

    // 7) Shift input and output buffers to make room for the next sample
    for (uint8_t i = 1; i < bFilterOrder; i++)
    {
        pIn[i - 1]  = pIn[i];
        pOut[i - 1] = pOut[i];
    }

    // 8) Return the filtered output sample
    return output;
}

double ecgbr_digital_filter(double dInput, double *pIn, double *pOut, const double *pA, const double *pB, uint8_t aLength, uint8_t bLength, uint8_t bFilterOrder, bool fReset, double dInitialSample)
{    
    double output = 0.0f;
    double tmp    = 0.0f;

    // 1) Check arguments - pointers
    if ((pA == NULL) || (pB == NULL) || (pIn == NULL) || (pOut == NULL))
    {
        return 0;
    }

    // 2) Check arguments - lengths
    if ((bFilterOrder == 0) || (aLength == 0) || (bLength == 0) || (aLength > bFilterOrder) || (bLength > bFilterOrder))
    {
        return 0;
    }

    // 3) If fReset was set, modify input data
    if (fReset)
    {
        for (uint8_t i = 0; i < bFilterOrder; i++)
        {
            pIn[i]  = dInitialSample;   // Initialize input buffer with initial sample
            pOut[i] = 0;                // Initialize output buffer with zero
        }
    }
    pIn[bFilterOrder - 1] = dInput;     // Place new input sample at the end of input buffer

    // 4) Apply the feedforward coefficients to the input samples 
    for (uint8_t i = 0; i < bLength; i++)
    {
        tmp += pB[i] * pIn[bFilterOrder - 1 - i];
    }

    // 5) Apply the feedback coefficients to the output samples
    for (uint8_t i = 1; i < aLength; i++)
    {
        tmp -= pA[i] * pOut[bFilterOrder - 1 - i];
    }

    // 6) Normalize by the first feedback coefficient 
    tmp /= pA[0];
    output = tmp;
    pOut[bFilterOrder - 1]  = output;

    // 7) Shift input and output buffers to make room for the next sample
    for (uint8_t i = 1; i < bFilterOrder; i++)
    {
        pIn[i - 1]  = pIn[i];
        pOut[i - 1] = pOut[i];
    }

    // 8) Return the filtered output sample
    return output;
}