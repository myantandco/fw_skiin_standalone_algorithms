// Copyright 2022-2023 Applied Brain Research Inc.

#include "constants.h"
#include "custom_kernels.h"
#include "custom_cmsis_kernels.h"

#include "custom_chest.h"

namespace {

  const float input0_scale = 9.40914266e-03;
  const int32_t input0_zero_point = 85;
  const float input1_scale = 2.56207623e-02;
  const int32_t input1_zero_point = 9;
  const float output0_scale = 5.09097539e-02;
  const int32_t output0_zero_point = 69;
  const float output1_scale = 2.56608743e-02;
  const int32_t output1_zero_point = 8;

  //--- Op 0: FULLY_CONNECTED
  FullyConnectedParams op_params_0;
  const RuntimeShape input_shape_0({1, 3});
  const RuntimeShape filter_shape_0({8, 3});
  const RuntimeShape bias_shape_0({8});
  const RuntimeShape output_shape_0({1, 8});
  const int8_t filter_0_data[24] = {1, 37, 36, -54, -54, -19, 61, 65, 44, 0, 127, 0, 17, 14, -60, -1, 3, -61, 4, 2, 104, 114, 1, 2};
  const int32_t bias_0_data[8] = {89, 179, 3814, -88, -4341, -198, -155, -77};

  //--- Op 1: FULLY_CONNECTED
  FullyConnectedParams op_params_1;
  const RuntimeShape input_shape_1({1, 8});
  const RuntimeShape filter_shape_1({3, 8});
  const RuntimeShape bias_shape_1({});
  const RuntimeShape output_shape_1({1, 3});
  const int8_t filter_1_data[24] = {16, 32, -40, 14, -41, 25, 11, 18, -11, -18, 2, -10, -3, -10, -10, -16, 30, 25, -52, 107, -102, 38, 117, 127};
  const int32_t *bias_1_data = nullptr;

  //--- Op 3: FULLY_CONNECTED
  FullyConnectedParams op_params_3;
  const RuntimeShape input_shape_3({3, 1});
  const RuntimeShape filter_shape_3({6, 1});
  const RuntimeShape bias_shape_3({});
  const RuntimeShape output_shape_3({3, 6});
  const int8_t filter_3_data[6] = {14, -41, 70, -90, 117, -127};
  const int32_t *bias_3_data = nullptr;

  //--- Op 5: FULLY_CONNECTED
  FullyConnectedParams op_params_5;
  const RuntimeShape input_shape_5({3, 6});
  const RuntimeShape filter_shape_5({6, 6});
  const RuntimeShape bias_shape_5({});
  const RuntimeShape output_shape_5({3, 6});
  const int8_t filter_5_data[36] = {127, -1, -1, -1, -1, -1, 4, 124, -4, -4, -4, -3, -7, 7, 121, -6, -6, -6, 9, -9, 9, 119, -9, -8, -11, 11, -12, 12, 116, -11, 12, -12, 13, -13, 14, 114};
  const int32_t *bias_5_data = nullptr;

  //--- Op 6: ADD
  ArithmeticParams op_params_6;
  const RuntimeShape input1_shape_6({3, 6});
  const RuntimeShape input2_shape_6({3, 6});
  const RuntimeShape output_shape_6({3, 6});
  const int8_t input1_6_data[18] = {9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};
  const int8_t input2_6_data[18] = {15, -41, 72, -90, 120, -128, -13, 42, -68, 90, -115, 127, 15, -41, 72, -90, 120, -128};

  //--- Op 8: FULLY_CONNECTED
  FullyConnectedParams op_params_8;
  const RuntimeShape input_shape_8({1, 18});
  const RuntimeShape filter_shape_8({15, 18});
  const RuntimeShape bias_shape_8({15});
  const RuntimeShape output_shape_8({1, 15});
  const int8_t filter_8_data[270] = {-41, 28, 3, 0, 28, 8, -39, 31, -21, 29, -68, -50, 6, 26, 3, 5, 21, 11, 10, -13, -5, -3, -30, -24, 81, -41, 44, -30, 65, 24, -6, -27, 7, -10, -34, -28, -3, 3, 5, -2, 2, 9, 12, -9, 6, -41, -16, -100, 1, -6, 5, -7, 0, -6, 14, -43, -10, -30, -42, -33, 7, -28, 57, 20, 55, 71, 0, 9, -7, -9, -5, -14, -12, 12, 2, 11, 13, 2, -36, -23, -18, -37, -49, 24, 16, 29, -3, 10, 13, 2, -66, 42, -28, 2, -6, -23, 34, -15, 28, 9, 31, 29, 20, -1, -10, -1, -9, -14, -2, -9, 2, 6, 1, 7, 23, -25, -10, -23, -29, -104, 1, -12, 4, -9, -12, -18, -11, 12, -31, 8, -28, 13, 27, -45, -24, -17, -51, -92, -1, 5, 9, 8, 18, 19, 2, 18, 10, 12, 15, 41, 83, -57, 20, -57, -25, -127, -26, -9, 9, -10, -11, 3, -3, -16, 11, -25, -34, -28, 24, -10, -5, 0, -61, -59, 0, -15, 2, -3, -24, -10, 56, -38, 17, 13, 1, 54, 13, -27, -15, 8, -35, -31, -30, 23, 6, 21, 30, 32, -36, 6, 17, 44, 19, -3, 6, -38, 16, -79, 2, -52, -1, 8, 28, 39, 15, -6, -42, 35, -9, -5, 15, -18, -3, 9, -2, 8, 1, -43, -2, -3, -3, -8, -2, -8, 14, -4, 29, 11, 66, 50, 17, -42, 17, -16, 14, -73, -9, 10, 2, 5, 9, 21, 11, 15, 12, 5, 42, 43, 42, -27, -24, 17, -49, -38, -17, -10, 28, 5, 29, 36};
  const int32_t bias_8_data[15] = {-1107, -2118, -103, 2408, 1655, -1596, -59, -2024, -901, -244, 1739, -2607, -3449, -799, -3025};

  //--- Op 9: FULLY_CONNECTED
  FullyConnectedParams op_params_9;
  const RuntimeShape input_shape_9({1, 15});
  const RuntimeShape filter_shape_9({1, 15});
  const RuntimeShape bias_shape_9({1});
  const RuntimeShape output_shape_9({1, 1});
  const int8_t filter_9_data[15] = {-48, -101, 80, -76, -51, -62, 51, 112, 82, 34, -88, -50, -127, 85, -91};
  const int32_t bias_9_data[1] = {-7984};

  CMSISFullyConnectedParams op_params_0c;
  CMSISFullyConnectedParams op_params_1c;
  CMSISFullyConnectedParams op_params_3c;
  CMSISFullyConnectedParams op_params_5c;
  CMSISFullyConnectedParams op_params_8c;
  CMSISFullyConnectedParams op_params_9c;

  int8_t input0[kModelInputSize];
  int8_t input1[kStateInputSize];
  int8_t output0[kOutputSize];
  int8_t output1[kStateInputSize];

  int8_t buffer_a[18];
  int8_t buffer_b[18];

  // arm_fully_connected_s8_get_buffer_size currently always returns 0
  // we could get rid of this completely, but things run a hair quicker with it
  const int scratch_size = 0;
  uint8_t scratch[scratch_size];
}

inline int32_t round_int32(float val) {
  return static_cast<int32_t>(round(val));
}

inline int8_t clip_int8(int32_t val) {
  constexpr int32_t lower = -128;
  constexpr int32_t upper = 127;
  return static_cast<int8_t>(std::max(lower, std::min(val, upper)));
}

int custom_chest_setup(int input_size, int state_size, int output_size) {
  if (input_size != kModelInputSize) {
    printf("Input size %d does not match library %d. Check constants.h\n\r",
           input_size, kModelInputSize);
    return 1;
  }
  if (state_size != kStateInputSize) {
    printf("State size %d does not match library %d. Check constants.h\n\r",
           state_size, kStateInputSize);
    return 1;
  }
  if (output_size != kOutputSize) {
    printf("Output size %d does not match library %d. Check constants.h\n\r",
           output_size, kOutputSize);
    return 1;
  }
  if (kNumInputs != 2) {
    printf("Must have two inputs: input and state\n\r");
    return 1;
  }
  if (kNumOutputs != 2) {
    printf("Must have two outputs: output and state\n\r");
    return 1;
  }

  //--- Op 0: FULLY_CONNECTED
  op_params_0.input_offset = -85;
  op_params_0.weights_offset = 0;
  op_params_0.output_offset = -128;
  op_params_0.output_multiplier = 1368493253;
  op_params_0.output_shift = -5;
  op_params_0.quantized_activation_min = -128;
  op_params_0.quantized_activation_max = 127;

  //--- Op 1: FULLY_CONNECTED
  op_params_1.input_offset = 128;
  op_params_1.weights_offset = 0;
  op_params_1.output_offset = -2;
  op_params_1.output_multiplier = 1594970839;
  op_params_1.output_shift = -5;
  op_params_1.quantized_activation_min = -128;
  op_params_1.quantized_activation_max = 127;

  //--- Op 3: FULLY_CONNECTED
  op_params_3.input_offset = 2;
  op_params_3.weights_offset = 0;
  op_params_3.output_offset = 1;
  op_params_3.output_multiplier = 1082196420;
  op_params_3.output_shift = -6;
  op_params_3.quantized_activation_min = -128;
  op_params_3.quantized_activation_max = 127;

  //--- Op 5: FULLY_CONNECTED
  op_params_5.input_offset = -9;
  op_params_5.weights_offset = 0;
  op_params_5.output_offset = 9;
  op_params_5.output_multiplier = 1080459350;
  op_params_5.output_shift = -6;
  op_params_5.quantized_activation_min = -128;
  op_params_5.quantized_activation_max = 127;

  //--- Op 6: ADD
  op_params_6.left_shift = 20;
  op_params_6.input1_offset = -9;
  op_params_6.input1_multiplier = 1073741824;
  op_params_6.input1_shift = 0;
  op_params_6.input2_offset = -1;
  op_params_6.input2_multiplier = 1281114312;
  op_params_6.input2_shift = -3;
  op_params_6.output_offset = 8;
  op_params_6.output_multiplier = 2124416468;
  op_params_6.output_shift = -19;
  op_params_6.quantized_activation_min = -128;
  op_params_6.quantized_activation_max = 127;

  //--- Op 8: FULLY_CONNECTED
  op_params_8.input_offset = -8;
  op_params_8.weights_offset = 0;
  op_params_8.output_offset = -128;
  op_params_8.output_multiplier = 1777503744;
  op_params_8.output_shift = -5;
  op_params_8.quantized_activation_min = -128;
  op_params_8.quantized_activation_max = 127;

  //--- Op 9: FULLY_CONNECTED
  op_params_9.input_offset = 128;
  op_params_9.weights_offset = 0;
  op_params_9.output_offset = 69;
  op_params_9.output_multiplier = 1234343347;
  op_params_9.output_shift = -7;
  op_params_9.quantized_activation_min = -128;
  op_params_9.quantized_activation_max = 127;

// #ifdef CMSIS_FC
  CMSIS_FillFCParams(
      op_params_0c, op_params_0, input_shape_0, filter_shape_0,
      bias_shape_0, output_shape_0, scratch_size, scratch);
  CMSIS_FillFCParams(
      op_params_1c, op_params_1, input_shape_1, filter_shape_1,
      bias_shape_1, output_shape_1, scratch_size, scratch);
  CMSIS_FillFCParams(
      op_params_3c, op_params_3, input_shape_3, filter_shape_3,
      bias_shape_3, output_shape_3, scratch_size, scratch);
  CMSIS_FillFCParams(
      op_params_5c, op_params_5, input_shape_5, filter_shape_5,
      bias_shape_5, output_shape_5, scratch_size, scratch);
  CMSIS_FillFCParams(
      op_params_8c, op_params_8, input_shape_8, filter_shape_8,
      bias_shape_8, output_shape_8, scratch_size, scratch);
  CMSIS_FillFCParams(
      op_params_9c, op_params_9, input_shape_9, filter_shape_9,
      bias_shape_9, output_shape_9, scratch_size, scratch);
// #endif

  return 0;
}

void custom_chest_set_states(float * state_vals) {
  for (int j = 0; j < kStateInputSize; j++) {
    input1[j] = clip_int8(
        round_int32(state_vals[j] / input1_scale) + input1_zero_point
    );
  }
}

void custom_chest_get_states(float * state_vals) {
  for (int j = 0; j < kStateInputSize; j++) {
    state_vals[j] = (output1[j] - output1_zero_point) * output1_scale;
  }
}

void custom_chest_set_inputs(float * input_vals) {
  for (int i = 0; i < kModelInputSize; i++) {
    input0[i] = clip_int8(
        round_int32(input_vals[i] / input0_scale) + input0_zero_point
    );
  }
}

void custom_chest_get_outputs(float * output_vals) {
  for (int i = 0; i < kOutputSize; i++) {
    output_vals[i] = (output0[i] - output0_zero_point) * output0_scale;
  }
}

void print_array(RuntimeShape shape, int8_t *data) {
  const int ndim = shape.DimensionsCount();
  int size = (ndim > 0) ? 1 : 0;
  for (int i = 0; i < ndim; i++)
    size *= shape.Dims(i);

  for (int i = 0; i < size; i++) {
    printf("%d", data[i]);
    if (i < size - 1) printf(", ");
  }
}

int custom_chest_inference() {
  //--- Op 0: FULLY_CONNECTED
  //--- Op 1: FULLY_CONNECTED
  //--- Op 3: FULLY_CONNECTED
  //--- Op 5: FULLY_CONNECTED
  //--- Op 6: ADD
  //--- Op 8: FULLY_CONNECTED
  //--- Op 9: FULLY_CONNECTED

  // Op connectivity:
  // input0 -> op0 -> op1 -> op2 -> op3 -> input_b of op6
  // input1 -> op4 -> op5 -> input_a of op6
  // op6 -> op7 -> op8 -> op9 -> output0
  //            -> output1

  // Buffer usage:
  // op0(input0) -> buffer_a
  // op1(buffer_a) -> buffer_b
  // op2(buffer_b) -> buffer_b  (reshape no-op)
  // op3(buffer_b) -> buffer_a
  // op4(input1) -> input1  (reshape no-op)
  // op5(input1) -> buffer_b
  // op6(buffer_b, buffer_a) -> output1
  // op7(output1) -> output1 (reshape no-op)
  // op8(output1) -> buffer_a
  // op9(buffer_a) -> output0

  // --- Op 0: FULLY_CONNECTED
#ifdef CMSIS_FC
  CMSIS_FullyConnected(
      op_params_0c,
      input_shape_0,
      input0,  // input
      filter_shape_0,
      filter_0_data,
      bias_shape_0,
      bias_0_data,
      output_shape_0,
      buffer_a  // output
  );
#else
  FullyConnected(
      op_params_0,
      input_shape_0,
      input0,  // input
      filter_shape_0,
      filter_0_data,
      bias_shape_0,
      bias_0_data,
      output_shape_0,
      buffer_a  // output
  );
#endif

  // printf("Op0:\n");
  // print_array(output_shape_0, buffer_a);
  // printf("\n");

  // --- Op 1: FULLY_CONNECTED
#ifdef CMSIS_FC_EXTRA
  CMSIS_FullyConnected(
      op_params_1c,
      input_shape_1,
      buffer_a,  // input
      filter_shape_1,
      filter_1_data,
      bias_shape_1,
      bias_1_data,
      output_shape_1,
      buffer_b  // output
  );
#else
  FullyConnected(
      op_params_1,
      input_shape_1,
      buffer_a,  // input
      filter_shape_1,
      filter_1_data,
      bias_shape_1,
      bias_1_data,
      output_shape_1,
      buffer_b  // output
  );
#endif

  // printf("Op1:\n");
  // print_array(output_shape_1, buffer_b);
  // printf("\n");

  // --- Op 3: FULLY_CONNECTED
#ifdef CMSIS_FC_EXTRA
  CMSIS_FullyConnected(
      op_params_3c,
      input_shape_3,
      buffer_b,  // input
      filter_shape_3,
      filter_3_data,
      bias_shape_3,
      bias_3_data,
      output_shape_3,
      buffer_a  // output
  );
#else
  FullyConnected(
      op_params_3,
      input_shape_3,
      buffer_b,  // input
      filter_shape_3,
      filter_3_data,
      bias_shape_3,
      bias_3_data,
      output_shape_3,
      buffer_a  // output
  );
#endif

  //--- Op 5: FULLY_CONNECTED
#ifdef CMSIS_FC_EXTRA
  CMSIS_FullyConnected(
      op_params_5c,
      input_shape_5,
      input1,  // input
      filter_shape_5,
      filter_5_data,
      bias_shape_5,
      bias_5_data,
      output_shape_5,
      buffer_b  // output
  );
#else
  FullyConnected(
      op_params_5,
      input_shape_5,
      input1,  // input
      filter_shape_5,
      filter_5_data,
      bias_shape_5,
      bias_5_data,
      output_shape_5,
      buffer_b  // output
  );
#endif

  // printf("Op5:\n");
  // print_array(input_shape_5, input1);
  // printf("\n");
  // print_array(output_shape_5, buffer_b);
  // printf("\n");

  //--- Op 6: ADD
#ifdef CMSIS_ADD
  CMSIS_Add(
      op_params_6,
      input1_shape_6,
      buffer_b,  // input
      input2_shape_6,
      buffer_a,  // input
      output_shape_6,
      output1  // output
  );
#else
  Add(
      op_params_6,
      input1_shape_6,
      buffer_b,  // input
      input2_shape_6,
      buffer_a,  // input
      output_shape_6,
      output1  // output
  );
#endif

  // printf("Op6:\n");
  // print_array(output_shape_6, output1);
  // printf("\n");

  //--- Op 8: FULLY_CONNECTED
#ifdef CMSIS_FC
  CMSIS_FullyConnected(
      op_params_8c,
      input_shape_8,
      output1,  // input
      filter_shape_8,
      filter_8_data,
      bias_shape_8,
      bias_8_data,
      output_shape_8,
      buffer_a  // output
  );
#else
  FullyConnected(
      op_params_8,
      input_shape_8,
      output1,  // input
      filter_shape_8,
      filter_8_data,
      bias_shape_8,
      bias_8_data,
      output_shape_8,
      buffer_a  // output
  );
#endif

  //--- Op 9: FULLY_CONNECTED
#ifdef CMSIS_FC_EXTRA
  CMSIS_FullyConnected(
      op_params_9c,
      input_shape_9,
      buffer_a,  // input
      filter_shape_9,
      filter_9_data,
      bias_shape_9,
      bias_9_data,
      output_shape_9,
      output0  // output
  );
#else
  FullyConnected(
      op_params_9,
      input_shape_9,
      buffer_a,  // input
      filter_shape_9,
      filter_9_data,
      bias_shape_9,
      bias_9_data,
      output_shape_9,
      output0  // output
  );
#endif

  return 0;
}
