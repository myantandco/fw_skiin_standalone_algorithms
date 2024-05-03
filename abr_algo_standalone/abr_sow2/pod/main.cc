// Copyright 2023 Applied Brain Research Inc.

#include <math.h>    // modf
#include <string.h>  // strlen

#include "pod-lib/src/constants.h"
#include "pod-lib/src/data_chest.h"
#include "pod-lib/src/data_waist.h"
#include "pod-lib/src/custom_chest.h"
#include "pod-lib/src/custom_waist.h"

#ifdef MAKE_ARM
  #include "app_uart.h"
  #include "bsp.h"
  #include "nrf.h"
  #include "nrf_delay.h"
  #include "nrf_uart.h"

  // https://www.embeddedcomputing.com/technology/processing/measuring-code-execution-time-on-arm-cortex-m-mcus
  #define ARM_CM_DEMCR      (*(uint32_t *)0xE000EDFC)
  #define ARM_CM_DWT_CTRL   (*(uint32_t *)0xE0001000)
  #define ARM_CM_DWT_CYCCNT (*(uint32_t *)0xE0001004)

  #define UART_TX_BUF_SIZE 512 /**< UART TX buffer size. */
  #define UART_RX_BUF_SIZE 256 /**< UART RX buffer size. */
  #define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

#else

  #include <cstdint>
  #include <cstdio>

#endif

#ifdef MAKE_ARM
  uint32_t clock() {
    return ARM_CM_DWT_CYCCNT;
  }

  int clock_init() {
    int clock_error = 1;
    if (ARM_CM_DWT_CTRL != 0) {        // See if DWT is available
      ARM_CM_DEMCR      |= 1 << 24;  // Set bit 24
      ARM_CM_DWT_CYCCNT  = 0;
      ARM_CM_DWT_CTRL   |= 1 << 0;   // Set bit 0
      clock_error = 0;
    }
    return clock_error;
  }

  void uart_error_handle(app_uart_evt_t* p_event) {
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
      APP_ERROR_HANDLER(p_event->data.error_communication);
    } else if (p_event->evt_type == APP_UART_FIFO_ERROR) {
      APP_ERROR_HANDLER(p_event->data.error_code);
    }
  }

  void uart_finish() {
    nrf_delay_ms(30);
    app_uart_flush();
    app_uart_close();
    nrf_delay_ms(30);
  }

  int setup(const int use_waist) {
    uint32_t err_code;

    bsp_board_init(BSP_INIT_LEDS);

    const app_uart_comm_params_t comm_params = {
        RX_PIN_NUMBER, TX_PIN_NUMBER, RTS_PIN_NUMBER,          CTS_PIN_NUMBER,
        UART_HWFC,     false,         NRF_UART_BAUDRATE_115200};

    APP_UART_FIFO_INIT(&comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE,
                       uart_error_handle, APP_IRQ_PRIORITY_LOWEST, err_code);
    APP_ERROR_CHECK(err_code);

    if (use_waist) {
        return custom_waist_setup(kHostInputSize, kStateInputSize, kOutputSize);
    } else {
        return custom_chest_setup(kHostInputSize, kStateInputSize, kOutputSize);
    }
  }

#else
  void nrf_delay_ms(int x) { return; }
  void uart_finish() { return; }
  int clock_init() { return 1; }
  uint32_t clock() { return 0; }

  int setup(const int use_waist) {
    if (use_waist) {
        return custom_waist_setup(kHostInputSize, kStateInputSize, kOutputSize);
    } else {
        return custom_chest_setup(kHostInputSize, kStateInputSize, kOutputSize);
    }
  }

#endif


int main() {
  int status = 0;
  const int use_waist = 1;

  // setup clock
  int has_clock = 0;
  has_clock = clock_init() == 0;

  // setup TFLite
  status = setup(use_waist);
  if (status != 0) {
    nrf_delay_ms(10);
    printf("\n\rSetup failed\n\r");
    uart_finish();
    return status;
  }

  int n_close_rpeak = 0;
  int n_close_quality = 0;
  int n_close_pre = 0;
  float atol = 0.001;
  float rtol = 0.001;

  float preproc[kModelInputSize] = {0};
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

  printf("\n\r\n\rSTART (has_clock=%d, use_waist=%d)\n\r", has_clock, use_waist);

  uint32_t total_clock = clock();

  // Iterate through all input values
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

    for (int j = 0; j < kModelInputSize; j++) {
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
      // quality[j] = q;
    }

    // Set inputs and states, run inference
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
      nrf_delay_ms(10);
      printf("Inference FAILED\n\r");
      uart_finish();
      return status;
    }

    // Get post inference states and outputs
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
    for (int j = 0; j < kModelInputSize; j++) {
      if (abs(ref_pre_i[j] - preproc[j]) <= atol + rtol * abs(ref_pre_i[j])) {
        n_close_pre++;
      }
    }

    // Output to serial for inspection
    if (0) {
      // Output only first (r-peak) output each timestep
      printf("Step %3d: REF: %.4f, PRED: %.4f\n\r", i,
             static_cast<double>(ref_out_i[0]),
             static_cast<double>(outputs[0]));
      nrf_delay_ms(10);  // Slow down so output buffer isn't overrun
    } else if (0) {
      // Output all outputs on only some timesteps
      if (ref_out_i[0] > 0.1f) {
        printf("Step %3d:\n\r", i);
        nrf_delay_ms(10);  // Slow down so output buffer isn't overrun
        printf("REF: ");
        for (int j = 0; j < N_OUTPUTS; j++) {
          printf("%0.4f ", static_cast<double>(ref_out_i[j]));
          nrf_delay_ms(2);  // Slow down so output buffer isn't overrun
        }
        printf("\n\r");
        printf("OUT: ");
        for (int j = 0; j < N_OUTPUTS; j++) {
          printf("%0.4f ", static_cast<double>(ref_out_i[j]));
          nrf_delay_ms(2);  // Slow down so output buffer isn't overrun
        }
        printf("\n\r");
        nrf_delay_ms(10);  // Slow down so output buffer isn't overrun
      }
    }
  }

  total_clock = clock() - total_clock;

  // Print summary
  printf("\n\r");
  printf("Overall pre: %d/%d within (atol %.4f, rtol %.4f) of reference\n\r",
         n_close_pre, N_STEPS * kModelInputSize, static_cast<double>(atol), static_cast<double>(rtol));
  printf("Overall rpeak: %d/%d within (atol %.4f, rtol %.4f) of reference\n\r",
         n_close_rpeak, N_STEPS, static_cast<double>(atol), static_cast<double>(rtol));
  if (N_OUTPUTS > 1) {
    printf("Overall quality: %d/%d within (atol %.4f, rtol %.4f) of reference\n\r",
           n_close_quality, N_STEPS * (N_OUTPUTS - 1),
           static_cast<double>(atol), static_cast<double>(rtol));
  }
  printf("Clock: %ld\n\r", total_clock);

  uart_finish();
  return status;
}
