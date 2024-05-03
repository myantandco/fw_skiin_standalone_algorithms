#ifndef __ABR_CUSTOM_CMSIS_KERNELS_H__
#define __ABR_CUSTOM_CMSIS_KERNELS_H__

#include "cmsis/CMSIS/NN/Include/arm_nnfunctions.h"
#include "custom_types.h"

void CMSIS_Add(
    const ArithmeticParams& params,
    const RuntimeShape& input1_shape, const int8_t* input1_data,
    const RuntimeShape& input2_shape, const int8_t* input2_data,
    const RuntimeShape& output_shape, int8_t* output_data
);

void CMSIS_FillFCParams(
    CMSISFullyConnectedParams& cmsis_params,
    const FullyConnectedParams& params,
    const RuntimeShape& input_shape,
    const RuntimeShape& filter_shape,
    const RuntimeShape& bias_shape,
    const RuntimeShape& output_shape,
    const int32_t scratch_size,
    void *scratch
);

void CMSIS_FullyConnected(
    const CMSISFullyConnectedParams& params,
    const RuntimeShape& input_shape, const int8_t* input_data,
    const RuntimeShape& filter_shape, const int8_t* filter_data,
    const RuntimeShape& bias_shape, const int32_t* bias_data,
    const RuntimeShape& output_shape, int8_t* output_data
);

#endif  // __ABR_CUSTOM_CMSIS_KERNELS_H__
