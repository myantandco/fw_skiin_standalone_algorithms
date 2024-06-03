#ifndef ABR_RPEAK_MODEL_HPP_
#define ABR_RPEAK_MODEL_HPP_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ECG_ALGO_OUTPUT_SIZE      1
#define ECG_ALGO_INPUT_SIZE		  3
#define ECG_ALGO_STATE_INPUT_SIZE 18

// ABR model definitions
#define ABR_INPUT_BASELINE_VALUE 685.7142857f

void ECGAlgo_SetGarmentID(garment_id_e nID);
void ECGAlgo_Init(void);
bool ECGAlgo_Run(float *pdData, uint8_t bChannelCount, bool fRestart);
void ECGAlgo_GetOutput(float *pdOutputs, uint8_t bLength);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DRIVERS_RPEAK_MODEL_HPP_ */
