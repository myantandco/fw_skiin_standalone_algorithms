#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "abr_postprocess.h"
#include "data_processing.h"

// ABR Post processing definition flags
#define ABR_RPEAK_THRESHOLD_MIN     -1.8f
#define ABR_RPEAK_PREDICTION_MAX    3.0f
#define ABR_RPEAK_RANGE             4.8f
#define ABR_RPEAK_RESOLUTION        31.0f

//structure definitions
typedef struct
{
    float max_value;
    uint8_t normalized;
    uint8_t max_index;
}rpeak_pp_t;

//Global variables definition
static rpeak_pp_t rpeak = {0};

void abr_pp_rpeak(float rpeak_output, uint8_t count)
{
    //1. check arguments
    // XXX - commented for standalone application
    // if (count > BIO_ECG_FIFO)
    // {
    //     return;
    // }

    if (rpeak_output > ABR_RPEAK_PREDICTION_MAX)
    {
        rpeak_output = ABR_RPEAK_PREDICTION_MAX;
    }

    //2. if reset flag is true, clear max_rpeak and rpeak_index to 0.
    if (!count)
    {
        rpeak.max_value = ABR_RPEAK_THRESHOLD_MIN;
        rpeak.normalized = 0;
        rpeak.max_index = 0;
    }

    //3. if rpeak is less than threshold return.
    if (rpeak_output < ABR_RPEAK_THRESHOLD_MIN)
    {
        return;
    }

    //4. compare rpeak value with previous max_value
    //and update max_value if it is higher.
    //also set max index to count,
    if (rpeak.max_value < rpeak_output)
    {
        rpeak.max_value = rpeak_output;
        rpeak.max_index = count+1; //offset to get index from 1 to 24
    }
}

void abr_pp_get_rpeak(uint8_t *rpeak_max, uint8_t *rpeak_index)
{
    //1. check arguments
    if (rpeak.max_value > ABR_RPEAK_PREDICTION_MAX)
    {
        return;
    }

    if (!rpeak.max_index)
    {
        *rpeak_max = 0;
        *rpeak_index = 0;
        return;
    }

    //2. normalize rpeak value to 5 bits
    rpeak.normalized = ((uint8_t)((floor)(((rpeak.max_value - ABR_RPEAK_THRESHOLD_MIN)/ABR_RPEAK_RANGE) * ABR_RPEAK_RESOLUTION))) + 1;

    //3. copy normalized value to rpeak_max value for 5 bit resolution
    if(rpeak.normalized > 31)
    {
        return;
    }
    *rpeak_max = rpeak.normalized;

    //4. copy max_index to rpeak_index.
    *rpeak_index = rpeak.max_index;
}
