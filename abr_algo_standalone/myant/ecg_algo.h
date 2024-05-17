
#ifndef ABR_RPEAK_MODEL_HPP_
#define ABR_RPEAK_MODEL_HPP_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "data_processing.h"
#include "abr_preprocess.h"

#define ECG_ALGO_OUTPUT_SIZE    1
//ABR model definitions
#define ABR_INPUT_BASELINE_VALUE        685.7142857142857l

void ecg_algo_update_garmentid(garment_id_e id);
void ecg_algo_init(void);
bool ecg_algo_run(double *data, uint8_t ch_count, bool restart);
void ecg_algo_get_output(float *outputs, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DRIVERS_RPEAK_MODEL_HPP_ */
