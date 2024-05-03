/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#include <cstdlib>

// #include "tensorflow/lite/micro/examples/tfmicro_project/main_functions.h"
// // This is the default main used on systems that have the standard C entry
// // point. Other devices (for example FreeRTOS or ESP32) that have different
// // requirements for entry code (like an app_main function) should specialize
// // this main.cc file in a target-specific subfolder.
// int main(int argc, char *argv[]) {
//   // I don't think this will actually get called since we never exit main
//   // and this shouldn't really matter since the MCU only runs the code once,
//   // but I'm dropping this here as good practice/reminder we have cleanup.
//   // TODO: This only triggers on clean exit, might want to handle some abort
//   // exits, e.g. SIGINT (^C), SIGSTP (^Z)
//   std::atexit(cleanup);

//   setup();
//   while (true) {
//     loop();
//   }
// }

#include <string.h>  // strlen
#include <cstdio>
#include <math.h>

#include "constants.h"
#include "data_chest.h"
#include "data_waist.h"
#include "custom_chest.h"
#include "custom_waist.h"

// constexpr int kModelInputSize = 3;  // Number of model input values
// constexpr int kOutputSize = 10;  // Number of model output values
// constexpr int kStateInputSize = 14;  // Total number of model states

int main(int argc, char *argv[]) {
  int status = 0;

  // whether to use the waist model (1), or chest model (0)
  int use_waist = 0;
  if (argc > 1) {
      char* garment = argv[1];
      // printf("Garment: %s, %d, %d\n\r", garment, strcmp(garment, "chest"), strcmp(garment, ""));

      if (strcmp(garment, "chest") == 0) {
          use_waist = 0;
      } else if (strcmp(garment, "waist") == 0) {
          use_waist = 1;
      } else {
          printf("Unrecognized garment: %s\n\r", garment);
      }
  }

  // setup TFLite
  if (use_waist) {
      status = custom_waist_setup(kHostInputSize, kStateInputSize, kOutputSize);
  } else {
      status = custom_chest_setup(kHostInputSize, kStateInputSize, kOutputSize);
  }
  printf("\n\rUsing custom implementation (use_waist=%d)\n\r", use_waist);

  if (status != 0) {
    printf("\n\rSetup failed\n\r");
    return status;
  }

  int n_close_rpeak = 0;
  int n_close_quality = 0;
  float atol = 0.001;
  float rtol = 0.001;

  float preproc[kHostInputSize] = {0};
  float states[kStateInputSize] = {0};
  float outputs[kOutputSize] = {0};

  float lowpass_state0[kModelInputSize] = {0};

  float bp_state0[kModelInputSize] = {0};
  float bp_state1[kModelInputSize] = {0};
  float pf_state0[kModelInputSize] = {0};
  float m_last[kModelInputSize] = {1, 1, 1};
  float x_bp, temp_state;

  char m_targ[kModelInputSize] = {1, 1, 1};

  float clip0, clip1;
  float latch0, latch1;
  if (use_waist) {
      clip0 = -1.0;
      clip1 = 1.0;
      latch0 = 0.1;
      latch1 = 0.7;
  } else {
      clip0 = -2.0;
      clip1 = 2.0;
      latch0 = 0.15;
      latch1 = 0.9;
  }

  printf("START\n\r");

  // Iterate through all input values
  // for (int i = 0; i < 1; i++) {
  for (int i = 0; i < N_STEPS; i++) {
    float *ref_out_i, *ref_pre_i, *inp_i;
    if (use_waist) {
      inp_i = waist_input_data[i % N_STEPS];
      ref_pre_i = waist_preproc_data[i % N_STEPS];
      ref_out_i = waist_reference_output[i % N_STEPS];
    } else {
      inp_i = chest_input_data[i % N_STEPS];
      ref_pre_i = chest_preproc_data[i % N_STEPS];
      ref_out_i = chest_reference_output[i % N_STEPS];
    }

    for (int j = 0; j < kHostInputSize; j++) {
      if (0) {
        // use preprocessed data
        preproc[j] = ref_pre_i[j];
        continue;
      }

      float x = inp_i[j];
      float q = 0.0f;

      // preprocessor: `lowpass_subtract`
      lowpass_state0[j] =
        0.9131007162822623f * lowpass_state0[j] + 0.0868992837177377f * x;
      x -= lowpass_state0[j];

      // bandpass filter (state-space)
      x_bp = 0.31096514029047384f * bp_state0[j] + -0.32037364568502746f * bp_state1[j] + 0.20031153315903816f * x;
      temp_state = bp_state0[j];
      bp_state0[j] = 1.552407569281504f * bp_state0[j] + -0.5993769336819238f * bp_state1[j] + x;
      bp_state1[j] = temp_state;

      q = x - x_bp;

      // quality power with lowpass filter
      q = (q < 0) ? -q : q;  // absolute value
      temp_state = q;
      q = 0.037776709119069996f * pf_state0[j] + 0.019259274202335752f * temp_state;
      pf_state0[j] = 0.9614814515953285f * pf_state0[j] + temp_state;

      // quality mask with latching
      m_targ[j] = (q < latch0) ? 1 :
                  ((q >= latch1) | (m_targ[j] == 0)) ? 0 : 1;
      m_last[j] += (m_targ[j]) ? 0.015625f : -(0.015625f);
      m_last[j] = (m_last[j] > 1.0f) ? 1.0f : (m_last[j] < 0.0f) ? 0.0f : m_last[j];

      x = x_bp * m_last[j];

      // preprocessor: `clip_signals`
      x = x < clip0 ? clip0 : x > clip1 ? clip1 : x;

      preproc[j] = x;
    }

    if (use_waist) {
      custom_waist_set_inputs(preproc);
      custom_waist_set_states(states);
      status = custom_waist_inference();
    } else {
      custom_chest_set_inputs(preproc);
      custom_chest_set_states(states);
      status = custom_chest_inference();
    }

    if (status != 0) {
      printf("Inference FAILED\n\r");
      return status;
    }

    if (use_waist) {
      custom_waist_get_states(states);
      custom_waist_get_outputs(outputs);
    } else {
      custom_chest_get_states(states);
      custom_chest_get_outputs(outputs);
    }

    // Track how many predictions are close to reference
    for (int j = 0; j < N_OUTPUTS; j++) {
      if (abs(ref_out_i[j] - outputs[j]) <= atol + rtol * abs(ref_out_i[j])) {
        if (j == 0) {
          n_close_rpeak++;
        } else {
          n_close_quality++;
        }
      }
    }

    // Output to serial for inspection
    if (0) {
      // Output only first (r-peak) output each timestep
      printf("Step %3d: REF: %.4f, PRED: %.4f\n\r", i,
             static_cast<double>(ref_out_i[0]),
             static_cast<double>(outputs[0]));
    } else if (0) {
      // Output all outputs on only some timesteps
      if (ref_out_i[0] > 0.1f) {
        printf("Step %3d:\n\r", i);
        printf("REF: ");
        for (int j = 0; j < N_OUTPUTS; j++) {
          printf("%0.4f ", static_cast<double>(ref_out_i[j]));
        }
        printf("\n\r");
        printf("OUT: ");
        for (int j = 0; j < N_OUTPUTS; j++) {
          printf("%0.4f ", static_cast<double>(ref_out_i[j]));
        }
        printf("\n\r");
      }
    }
  }

  // Print summary
  printf("\n\rOverall rpeak: %d/%d within (atol %.4f, rtol %.4f) of reference\n\r",
         n_close_rpeak, N_STEPS, static_cast<double>(atol), static_cast<double>(rtol));
  if (N_OUTPUTS > 1) {
    printf("Overall quality: %d/%d within (atol %.4f, rtol %.4f) of reference\n\r",
           n_close_quality, N_STEPS * (N_OUTPUTS - 1),
           static_cast<double>(atol), static_cast<double>(rtol));
  }

  return status;
}
