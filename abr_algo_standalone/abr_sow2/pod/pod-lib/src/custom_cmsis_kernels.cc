#include "custom_cmsis_kernels.h"

inline int MatchingElementsSize(const RuntimeShape& shape,
                                const RuntimeShape& check_shape_0,
                                const RuntimeShape& check_shape_1) {
  const int size_1 = shape.FlatSize();
#ifndef NDEBUG
  const int size_2 = check_shape_0.FlatSize();
  const int size_3 = check_shape_1.FlatSize();
  TFLITE_CHECK_EQ(size_1, size_2);
  TFLITE_CHECK_EQ(size_2, size_3);
#endif
  return size_1;
}

void CMSIS_Add(
    const ArithmeticParams& params,
    const RuntimeShape& input1_shape, const int8_t* input1_data,
    const RuntimeShape& input2_shape, const int8_t* input2_data,
    const RuntimeShape& output_shape, int8_t* output_data
) {
  arm_elementwise_add_s8(
    input1_data, input2_data,
    params.input1_offset, params.input1_multiplier, params.input1_shift,
    params.input2_offset, params.input2_multiplier, params.input2_shift,
    params.left_shift, output_data,
    params.output_offset, params.output_multiplier, params.output_shift,
    params.quantized_activation_min, params.quantized_activation_max,
    MatchingElementsSize(input1_shape, input2_shape, output_shape)
  );
}

void CMSIS_FillFCParams(
    CMSISFullyConnectedParams& cmsis_params,
    const FullyConnectedParams& params,
    const RuntimeShape& input_shape,
    const RuntimeShape& filter_shape,
    const RuntimeShape& bias_shape,
    const RuntimeShape& output_shape,
    const int32_t scratch_size,
    void *scratch
) {
#ifndef NDEBUG
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 2);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 2);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 2);
  TFLITE_DCHECK_EQ(input_shape.Dims(0), output_shape.Dims(0));
  TFLITE_DCHECK_EQ(input_shape.Dims(1), filter_shape.Dims(1));
  TFLITE_DCHECK_EQ(output_shape.Dims(1), filter_shape.Dims(0));
  TFLITE_DCHECK_LE(bias_shape.DimensionsCount(), 1);
  if (bias_shape.DimensionsCount() == 1) {
      TFLITE_DCHECK_EQ(bias_shape.Dims(0), output_shape.Dims(1));
  }
#endif

  cmsis_params.fc_params.input_offset = params.input_offset;
  cmsis_params.fc_params.filter_offset = params.weights_offset;
  cmsis_params.fc_params.output_offset = params.output_offset;
  cmsis_params.fc_params.activation.min = params.quantized_activation_min;
  cmsis_params.fc_params.activation.max = params.quantized_activation_max;
  cmsis_params.q_params.multiplier = params.output_multiplier;
  cmsis_params.q_params.shift = params.output_shift;

  cmsis_params.input_dims.n = input_shape.Dims(0);
  cmsis_params.input_dims.h = 1;
  cmsis_params.input_dims.w = 1;
  cmsis_params.input_dims.c = input_shape.Dims(1);
  cmsis_params.filter_dims.n = filter_shape.Dims(1);
  cmsis_params.filter_dims.h = 1;
  cmsis_params.filter_dims.w = 1;
  cmsis_params.filter_dims.c = filter_shape.Dims(0);
  cmsis_params.bias_dims.n = 1;
  cmsis_params.bias_dims.h = 1;
  cmsis_params.bias_dims.w = 1;
  cmsis_params.bias_dims.c = output_shape.Dims(1);
  cmsis_params.output_dims.n = output_shape.Dims(0);
  cmsis_params.output_dims.h = 1;
  cmsis_params.output_dims.w = 1;
  cmsis_params.output_dims.c = output_shape.Dims(1);

  cmsis_params.ctx.buf = scratch;
  cmsis_params.ctx.size = scratch_size;

// #ifndef NDEBUG
//   const int32_t buf_size = arm_fully_connected_s8_get_buffer_size(&cmsis_params.filter_dims);
//   TFLITE_DCHECK_GE(scratch_size, buf_size);
// #endif
}

void CMSIS_FullyConnected(
    const CMSISFullyConnectedParams& params,
    const RuntimeShape& input_shape, const int8_t* input_data,
    const RuntimeShape& filter_shape, const int8_t* filter_data,
    const RuntimeShape& bias_shape, const int32_t* bias_data,
    const RuntimeShape& output_shape, int8_t* output_data
) {
  arm_fully_connected_s8(
    &params.ctx, &params.fc_params, &params.q_params,
    &params.input_dims, input_data,
    &params.filter_dims, filter_data,
    &params.bias_dims, bias_data,
    &params.output_dims, output_data
  );
}
