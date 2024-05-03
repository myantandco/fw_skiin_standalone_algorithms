#ifndef __ABR_CUSTOM_KERNELS_H__
#define __ABR_CUSTOM_KERNELS_H__

#include "fixedpoint/fixedpoint.h"
#include "custom_types.h"

inline int32_t MultiplyByQuantizedMultiplierSmallerThanOneExp(
    int32_t x, int32_t quantized_multiplier, int left_shift) {
  using gemmlowp::RoundingDivideByPOT;
  using gemmlowp::SaturatingRoundingDoublingHighMul;
  return RoundingDivideByPOT(
      SaturatingRoundingDoublingHighMul(x, quantized_multiplier), -left_shift);
}

inline int32_t MultiplyByQuantizedMultiplierGreaterThanOne(
    int32_t x, int32_t quantized_multiplier, int left_shift) {
  using gemmlowp::SaturatingRoundingDoublingHighMul;
  return SaturatingRoundingDoublingHighMul(x * (1 << left_shift),
                                           quantized_multiplier);
}

inline int32_t MultiplyByQuantizedMultiplier(int32_t x,
                                             int32_t quantized_multiplier,
                                             int shift) {
  using gemmlowp::RoundingDivideByPOT;
  using gemmlowp::SaturatingRoundingDoublingHighMul;
  int left_shift = shift > 0 ? shift : 0;
  int right_shift = shift > 0 ? 0 : -shift;
  return RoundingDivideByPOT(SaturatingRoundingDoublingHighMul(
                                 x * (1 << left_shift), quantized_multiplier),
                             right_shift);
}

inline int32_t MultiplyByQuantizedMultiplier(int64_t x,
                                             int32_t quantized_multiplier,
                                             int shift) {
  // Inputs:
  // - quantized_multiplier has fixed point at bit 31
  // - shift is -31 to +7 (negative for right shift)
  //
  // Assumptions: The following input ranges are assumed
  // - quantize_scale>=0  (the usual range is (1<<30) to (1>>31)-1)
  // - scaling is chosen so final scaled result fits in int32_t
  // - input x is in the range -(1<<47) <= x < (1<<47)
  assert(quantized_multiplier >= 0);
  assert(shift >= -31 && shift < 8);

  int32_t reduced_multiplier = (quantized_multiplier + (1 << 15)) >> 16;
  int total_shift = 15 - shift;
  x = (x * (int64_t)reduced_multiplier) + ((int64_t)1 << (total_shift - 1));
  int32_t result = x >> total_shift;
  return result;
}

void Add(const ArithmeticParams& params,
         const RuntimeShape& input1_shape, const int8_t* input1_data,
         const RuntimeShape& input2_shape, const int8_t* input2_data,
         const RuntimeShape& output_shape, int8_t* output_data);

void FullyConnected(
    const FullyConnectedParams& params, const RuntimeShape& input_shape,
    const int8_t* input_data, const RuntimeShape& filter_shape,
    const int8_t* filter_data, const RuntimeShape& bias_shape,
    const int32_t* bias_data, const RuntimeShape& output_shape,
    int8_t* output_data);

void Requantize(const int8_t* input_data, int32_t size,
                       int32_t effective_scale_multiplier,
                       int32_t effective_scale_shift, int32_t input_zeropoint,
                       int32_t output_zeropoint, int8_t* output_data);

#endif  // __ABR_CUSTOM_KERNELS_H__
