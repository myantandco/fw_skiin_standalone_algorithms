// Copyright 2022-2023 Applied Brain Research Inc.

#include "constants.h"
#include "custom_kernels.h"
#include "custom_cmsis_kernels.h"

namespace {

  const float input0_scale = 1.83163956e-03;
  const int32_t input0_zero_point = 14;
  const float input1_scale = 7.29666231e-03;
  const int32_t input1_zero_point = 7;
  const float output0_scale = 6.58982471e-02;
  const int32_t output0_zero_point = 78;
  const float output1_scale = 7.66433543e-03;
  const int32_t output1_zero_point = 10;

  //--- Op 0: FULLY_CONNECTED
  FullyConnectedParams op_params_0;
  const RuntimeShape input_shape_0({1, 3});
  const RuntimeShape filter_shape_0({8, 3});
  const RuntimeShape bias_shape_0({8});
  const RuntimeShape output_shape_0({1, 8});
  const int8_t filter_0_data[24] = {18, 18, 15, -111, 16, 21, 74, 85, 86, 0, 4, -127, -39, -80, -80, 0, -125, 8, 21, 8, 6, 102, 27, 28};
  const int32_t bias_0_data[8] = {10443, 59, -804, 41, 7456, 158, 12007, -39};

  //--- Op 1: FULLY_CONNECTED
  FullyConnectedParams op_params_1;
  const RuntimeShape input_shape_1({1, 8});
  const RuntimeShape filter_shape_1({3, 8});
  const RuntimeShape bias_shape_1({});
  const RuntimeShape output_shape_1({1, 3});
  const int8_t filter_1_data[24] = {-3, 123, -12, 127, -27, 124, -23, 46, -43, 13, 107, 13, 18, 15, -47, 26, 18, 16, 125, 54, -119, 55, 6, 103};
  const int32_t *bias_1_data = nullptr;

  //--- Op 3: FULLY_CONNECTED
  FullyConnectedParams op_params_3;
  const RuntimeShape input_shape_3({3, 1});
  const RuntimeShape filter_shape_3({6, 1});
  const RuntimeShape bias_shape_3({});
  const RuntimeShape output_shape_3({3, 6});
  const int8_t filter_3_data[6] = {17, -47, 81, -98, 127, -124};
  const int32_t *bias_3_data = nullptr;

  //--- Op 5: FULLY_CONNECTED
  FullyConnectedParams op_params_5;
  const RuntimeShape input_shape_5({3, 6});
  const RuntimeShape filter_shape_5({6, 6});
  const RuntimeShape bias_shape_5({});
  const RuntimeShape output_shape_5({3, 6});
  const int8_t filter_5_data[36] = {127, -2, -3, -2, -2, -2, 7, 122, -8, -7, -7, -5, -13, 13, 116, -12, -12, -9, 15, -16, 16, 112, -18, -14, -20, 20, -21, 23, 105, -20, 19, -20, 21, -22, 24, 103};
  const int32_t *bias_5_data = nullptr;

  //--- Op 6: ADD
  ArithmeticParams op_params_6;
  const RuntimeShape input1_shape_6({3, 6});
  const RuntimeShape input2_shape_6({3, 6});
  const RuntimeShape output_shape_6({3, 6});

  //--- Op 8: FULLY_CONNECTED
  FullyConnectedParams op_params_8;
  const RuntimeShape input_shape_8({1, 18});
  const RuntimeShape filter_shape_8({15, 18});
  const RuntimeShape bias_shape_8({15});
  const RuntimeShape output_shape_8({1, 15});
  const int8_t filter_8_data[270] = {-70, 42, 38, -25, 35, -39, 29, -4, -22, 27, -27, -9, 9, 13, 30, -26, -15, -31, -76, 24, 58, -4, -29, 29, -36, 19, 23, -34, -90, 9, 13, -47, 22, 23, 11, 8, -105, 49, -1, -19, -78, -42, -6, -26, -6, -9, 15, -58, -76, 80, -44, 35, -34, -39, -13, 65, -19, 4, 71, 40, -51, -6, -11, 0, 39, 74, -1, 29, -10, 11, 23, 30, 16, 30, -18, -14, -23, 19, 27, -10, -14, -19, -2, -3, -31, 14, -9, -57, -26, 1, -52, 68, 9, 2, 49, 30, -127, 56, 3, -43, 22, -3, 64, -19, 6, 16, 8, -30, 4, -72, 20, 16, -5, 4, -85, 10, 20, -28, -9, -4, -9, -6, -18, 11, -17, -48, -70, -54, 27, 39, 38, 74, -99, 110, -39, 57, -24, -6, -23, -42, 38, 25, 29, 13, -33, -34, 48, 75, 75, 74, -91, -14, -18, 16, 1, 45, 4, -16, 6, -22, -2, -4, -44, 24, 34, -22, -60, -43, -13, -1, -2, 11, -51, -50, 4, 22, -9, 35, 36, -23, 16, -35, -36, 50, 9, -6, -25, -27, -36, 1, 14, 12, -89, 31, -2, -13, -15, -7, -15, -63, 55, -38, -50, -101, -66, 47, -37, 51, -40, -23, -96, 33, 23, -36, -37, -7, -5, 52, 34, -14, 15, 85, -65, -12, 72, -1, 78, 17, -23, 51, 8, -7, 26, 46, 22, -35, -35, -21, 7, 21, -42, 7, 29, 68, -34, -37, -58, -6, 11, 41, 42, 15, 6, 9, -21, -27, -17, -107, 11, 23, 35, -2, 60, 42, 17, 20, -16, 45, 16, 78};
  const int32_t bias_8_data[15] = {5453, -9067, -11216, -716, 5368, -8170, -10179, -15247, -11950, -4899, -8929, -15288, -4083, -8554, 5471};

  //--- Op 9: FULLY_CONNECTED
  FullyConnectedParams op_params_9;
  const RuntimeShape input_shape_9({1, 15});
  const RuntimeShape filter_shape_9({1, 15});
  const RuntimeShape bias_shape_9({1});
  const RuntimeShape output_shape_9({1, 1});
  const int8_t filter_9_data[15] = {-59, -98, 98, -63, -39, -100, 41, 113, 106, 39, -64, -127, -74, 52, -51};
  const int32_t bias_9_data[1] = {-8375};

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

int custom_waist_setup(int input_size, int state_size, int output_size) {
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
  op_params_0.input_offset = -14;
  op_params_0.weights_offset = 0;
  op_params_0.output_offset = -128;
  op_params_0.output_multiplier = 1789795524;
  op_params_0.output_shift = -6;
  op_params_0.quantized_activation_min = -128;
  op_params_0.quantized_activation_max = 127;

  //--- Op 1: FULLY_CONNECTED
  op_params_1.input_offset = 128;
  op_params_1.weights_offset = 0;
  op_params_1.output_offset = -17;
  op_params_1.output_multiplier = 1881995901;
  op_params_1.output_shift = -7;
  op_params_1.quantized_activation_min = -128;
  op_params_1.quantized_activation_max = 127;

  //--- Op 3: FULLY_CONNECTED
  op_params_3.input_offset = 17;
  op_params_3.weights_offset = 0;
  op_params_3.output_offset = -2;
  op_params_3.output_multiplier = 1940119846;
  op_params_3.output_shift = -7;
  op_params_3.quantized_activation_min = -128;
  op_params_3.quantized_activation_max = 127;

  //--- Op 5: FULLY_CONNECTED
  op_params_5.input_offset = -7;
  op_params_5.weights_offset = 0;
  op_params_5.output_offset = 5;
  op_params_5.output_multiplier = 2141842633;
  op_params_5.output_shift = -7;
  op_params_5.quantized_activation_min = -128;
  op_params_5.quantized_activation_max = 127;

  //--- Op 6: ADD
  op_params_6.left_shift = 20;
  op_params_6.input1_offset = -5;
  op_params_6.input1_multiplier = 1073741824;
  op_params_6.input1_shift = 0;
  op_params_6.input2_offset = 2;
  op_params_6.input2_multiplier = 1131307473;
  op_params_6.input2_shift = -2;
  op_params_6.output_offset = 10;
  op_params_6.output_multiplier = 2023176849;
  op_params_6.output_shift = -19;
  op_params_6.quantized_activation_min = -128;
  op_params_6.quantized_activation_max = 127;

  //--- Op 8: FULLY_CONNECTED
  op_params_8.input_offset = -10;
  op_params_8.weights_offset = 0;
  op_params_8.output_offset = -128;
  op_params_8.output_multiplier = 1242103439;
  op_params_8.output_shift = -5;
  op_params_8.quantized_activation_min = -128;
  op_params_8.quantized_activation_max = 127;

  //--- Op 9: FULLY_CONNECTED
  op_params_9.input_offset = 128;
  op_params_9.weights_offset = 0;
  op_params_9.output_offset = 78;
  op_params_9.output_multiplier = 1640081502;
  op_params_9.output_shift = -8;
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

void custom_waist_set_states(float * state_vals) {
  for (int j = 0; j < kStateInputSize; j++) {
    input1[j] = clip_int8(
        round_int32(state_vals[j] / input1_scale) + input1_zero_point
    );
  }
}

void custom_waist_get_states(float * state_vals) {
  for (int j = 0; j < kStateInputSize; j++) {
    state_vals[j] = (output1[j] - output1_zero_point) * output1_scale;
  }
}

void custom_waist_set_inputs(float * input_vals) {
  for (int i = 0; i < kModelInputSize; i++) {
    input0[i] = clip_int8(
        round_int32(input_vals[i] / input0_scale) + input0_zero_point
    );
  }
}

void custom_waist_get_outputs(float * output_vals) {
  for (int i = 0; i < kOutputSize; i++) {
    output_vals[i] = (output0[i] - output0_zero_point) * output0_scale;
  }
}

int custom_waist_inference() {
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
