
#ifndef ABR_RPEAK_MODEL_HPP_
#define ABR_RPEAK_MODEL_HPP_

#ifdef __cplusplus
extern "C"
{
#endif

#include "abr_preprocess.h"
#include "data_processing.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ECG_ALGO_OUTPUT_SIZE     1
// ABR model definitions
#define ABR_INPUT_BASELINE_VALUE 685.7142857f

    void ecg_algo_update_garmentid(garment_id_e id);
    void ecg_algo_init(void);
    bool ecg_algo_run(float *data, uint8_t ch_count, bool restart);
    void ecg_algo_get_output(float *outputs, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* SRC_DRIVERS_RPEAK_MODEL_HPP_ */
