#include "custom_kernels.h"

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

void CheckArithmeticParams(const ArithmeticParams& params) {
  TFLITE_DCHECK_LE(params.quantized_activation_min,
                   params.quantized_activation_max);
  // Input offset is negative input zero point. Activation tensors are
  // asymmetric quantized so they span the full int8 range.
  TFLITE_DCHECK_GE(-params.input1_offset, std::numeric_limits<int8_t>::min());
  TFLITE_DCHECK_GE(-params.input2_offset, std::numeric_limits<int8_t>::min());
  TFLITE_DCHECK_LE(-params.input1_offset, std::numeric_limits<int8_t>::max());
  TFLITE_DCHECK_LE(-params.input2_offset, std::numeric_limits<int8_t>::max());
}

// Element-wise add that can often be used for inner loop of broadcast add as
// well as the non-broadcast add.
void AddElementwise(int size, const ArithmeticParams& params,
                           const int8_t* input1_data, const int8_t* input2_data,
                           int8_t* output_data) {
#ifndef NDEBUG
  CheckArithmeticParams(params);
#endif

  for (int i = 0; i < size; ++i) {
    const int32_t input1_val = params.input1_offset + input1_data[i];
    const int32_t input2_val = params.input2_offset + input2_data[i];
    const int32_t shifted_input1_val = input1_val * (1 << params.left_shift);
    const int32_t shifted_input2_val = input2_val * (1 << params.left_shift);
    const int32_t scaled_input1_val =
        MultiplyByQuantizedMultiplierSmallerThanOneExp(
            shifted_input1_val, params.input1_multiplier, params.input1_shift);
    const int32_t scaled_input2_val =
        MultiplyByQuantizedMultiplierSmallerThanOneExp(
            shifted_input2_val, params.input2_multiplier, params.input2_shift);
    const int32_t raw_sum = scaled_input1_val + scaled_input2_val;
    const int32_t raw_output =
        MultiplyByQuantizedMultiplierSmallerThanOneExp(
            raw_sum, params.output_multiplier, params.output_shift) +
        params.output_offset;
    const int32_t clamped_output =
        std::min(params.quantized_activation_max,
                 std::max(params.quantized_activation_min, raw_output));
    output_data[i] = static_cast<int8_t>(clamped_output);
  }
}

void Add(
    const ArithmeticParams& params,
    const RuntimeShape& input1_shape, const int8_t* input1_data,
    const RuntimeShape& input2_shape, const int8_t* input2_data,
    const RuntimeShape& output_shape, int8_t* output_data
) {
  const int flat_size =
      MatchingElementsSize(input1_shape, input2_shape, output_shape);
  AddElementwise(flat_size, params, input1_data, input2_data, output_data);
}

void FullyConnected(
    const FullyConnectedParams& params,
    const RuntimeShape& input_shape, const int8_t* input_data,
    const RuntimeShape& filter_shape, const int8_t* filter_data,
    const RuntimeShape& bias_shape, const int32_t* bias_data,
    const RuntimeShape& output_shape, int8_t* output_data
) {
  const int32_t input_offset = params.input_offset;
  const int32_t filter_offset = params.weights_offset;
  const int32_t output_offset = params.output_offset;
  const int32_t output_multiplier = params.output_multiplier;
  const int output_shift = params.output_shift;
  const int32_t output_activation_min = params.quantized_activation_min;
  const int32_t output_activation_max = params.quantized_activation_max;
  TFLITE_DCHECK_GE(filter_shape.DimensionsCount(), 2);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 2);

  TFLITE_DCHECK_LE(output_activation_min, output_activation_max);
  const int filter_dim_count = filter_shape.DimensionsCount();
  const int batches = output_shape.Dims(0);
  const int output_depth = output_shape.Dims(1);
  TFLITE_DCHECK_LE(output_depth, filter_shape.Dims(filter_dim_count - 2));
  const int accum_depth = filter_shape.Dims(filter_dim_count - 1);
  for (int b = 0; b < batches; ++b) {
    for (int out_c = 0; out_c < output_depth; ++out_c) {
      int32_t acc = 0;
      for (int d = 0; d < accum_depth; ++d) {
        int32_t input_val = input_data[b * accum_depth + d];
        int32_t filter_val = filter_data[out_c * accum_depth + d];
        acc += (filter_val + filter_offset) * (input_val + input_offset);
      }
      if (bias_data) {
        acc += bias_data[out_c];
      }
      acc = MultiplyByQuantizedMultiplier(acc, output_multiplier, output_shift);
      acc += output_offset;
      acc = std::max(acc, output_activation_min);
      acc = std::min(acc, output_activation_max);
      output_data[out_c + output_depth * b] = static_cast<int8_t>(acc);
    }
  }
}

void Requantize(const int8_t* input_data, int32_t size,
                       int32_t effective_scale_multiplier,
                       int32_t effective_scale_shift, int32_t input_zeropoint,
                       int32_t output_zeropoint, int8_t* output_data) {
  static constexpr int32_t kMinOutput = std::numeric_limits<int8_t>::min();
  static constexpr int32_t kMaxOutput = std::numeric_limits<int8_t>::max();
  for (int i = 0; i < size; ++i) {
    const int32_t input = input_data[i] - input_zeropoint;
    const int32_t output =
        MultiplyByQuantizedMultiplier(
            input, effective_scale_multiplier, effective_scale_shift)
        + output_zeropoint;
    const int32_t clamped_output =
        std::max(std::min(output, kMaxOutput), kMinOutput);
    output_data[i] = static_cast<int8_t>(clamped_output);
  }
}
