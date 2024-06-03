#include "abr_postprocess.h"
#include "data_processing.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// ABR Post processing definition flags
#define ABR_RPEAK_THRESHOLD_MIN  -1.7f    // changes based on garment (see ALDD)
#define ABR_RPEAK_PREDICTION_MAX 3.0f
#define ABR_RPEAK_RESOLUTION     31.0f
#define ABR_RPEAK_RANGE          4.7f     // changes based on garment, range = pred_max (fixed) - threshold_min (changes per garment)

// Structure definitions
typedef struct
{
    float   max_value;
    uint8_t normalized;
    uint8_t max_index;
} rpeak_pp_t;

// Global variables definition
static rpeak_pp_t rpeak = {0};

void ABRPostProcess_RPeak(float rpeak_output, uint8_t count)
{
    // 1. check arguments
    //  XXX - commented for standalone application
    //  if (count > BIO_ECG_FIFO)
    //  {
    //      return;
    //  }

    // 2) Cap rpeak_output to ABR_RPEAK_PREDICTION_MAX to ensure it stays within expected range 
    if (rpeak_output > ABR_RPEAK_PREDICTION_MAX)
    {
        rpeak_output = ABR_RPEAK_PREDICTION_MAX;
    }

    // 3) Reset rpeak values if count is zero (indicates a reset condition)
    if (!count)
    {
        rpeak.max_value  = ABR_RPEAK_THRESHOLD_MIN;
        rpeak.normalized = 0;
        rpeak.max_index  = 0;
    }

    // 4) Ignore rpeak values below the minimum threshold
    if (rpeak_output < ABR_RPEAK_THRESHOLD_MIN)
    {
        return;
    }

    // 5) Update the maximum rpeak value and corresponding max index
    //    if the current rpeak_output is greater than the stored max_value
    if (rpeak.max_value < rpeak_output)
    {
        rpeak.max_value = rpeak_output;
        rpeak.max_index = count + 1;    // Offset to get index from 1 to 24
    }
}

void ABRPostProcess_GetRPeak(uint8_t *rpeak_max, uint8_t *rpeak_index)
{
    // 1) Check arguments
    if (rpeak.max_value > ABR_RPEAK_PREDICTION_MAX || !rpeak_max || !rpeak_index)
    {
        return;
    }

    // 2) If max_index is NULL, return 0
    if (!rpeak.max_index)
    {
        *rpeak_max   = 0;
        *rpeak_index = 0;
        return;
    }

    // 3) Normalize rpeak value to 5 bits
    rpeak.normalized = ((uint8_t)((floor)(((rpeak.max_value - ABR_RPEAK_THRESHOLD_MIN) / ABR_RPEAK_RANGE) * ABR_RPEAK_RESOLUTION))) + 1;

    // 4) copy normalized value to rpeak_max value for 5 bit resolution
    if (rpeak.normalized > 31)
    {
        return;
    }
    *rpeak_max = rpeak.normalized;

    // 5) Copy max_index to rpeak_index.
    *rpeak_index = rpeak.max_index;
}
