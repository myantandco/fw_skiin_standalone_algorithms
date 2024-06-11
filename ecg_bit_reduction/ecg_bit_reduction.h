#ifndef SRC_ALGORITHMS_ECG_BIT_REDUCTION_H_
#define SRC_ALGORITHMS_ECG_BIT_REDUCTION_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

// XXX - Added below for standalone application only
typedef enum
{
    ECG1,
    ECG2,
    ECG3,
    MAX_ECG,
} ecg_sens_id;

int16_t ecgbr_sample_reduction(uint32_t sample, ecg_sens_id ecg_id, bool restart);


#endif /* SRC_ALGORITHMS_ECG_BIT_REDUCTION_H_ */
