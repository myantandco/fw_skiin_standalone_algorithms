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

int16_t ECGBitReduction_SampleReduction(uint32_t bSample, ecg_sens_id nECGId, bool fRestart);


#endif /* SRC_ALGORITHMS_ECG_BIT_REDUCTION_H_ */
