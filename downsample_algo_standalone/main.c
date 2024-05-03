/*
 * ecg_downsampling.c
 *
 * Algorithm that downsamples ecg data coming in at 409.6 Hz 
 * down to 320 Hz. The downsampling algorithm works by applying 
 * a digital low-pass filter to the incoming data to avoid aliasing
 * artifacts, then linearly interpolating the values. 
 * 
 *  Created on: Jan 31, 2024 
 *      Author: Piyush, Toast
 */


#include "input.h"
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SATURATE(x) ((x) > INT32_MAX ? INT32_MAX : ((x) < INT32_MIN ? INT32_MIN : (x)))

// Coefficient for filter
const float    coeff_a[ECGD_FILTER_SIZE] = {1.0, -0.4044849, 0.20064437};
const float    coeff_b[ECGD_FILTER_SIZE] = {0.19903987, 0.39807973, 0.19903987};

static float gpFilteredData[ECGD_PROCESS_BUFF_SIZE] = {0};    // Filtered input data
static float gpPaddedBuffer[ECGD_PROCESS_BUFF_SIZE] = {0};    // Buffer to pad input data for filtering
static float gpPostFilterBuffer[ECGD_FILTER_SIZE]   = {0};    // Output buffer for the filter
static float gpPreFilterBuffer[ECGD_FILTER_SIZE]    = {0};    // Input buffer for the filter
static int32_t gbLastSampledData                      = 0;      // Last sampled data
static bool    gfCnt                                  = false;  // Flag to indicate the first call to the function

static float   gpInputTimeIntervals[ECGD_INPUT_SIZE] = {0.0};   // Array to hold time values for input data
static float   gpOutputTimeIntervals[ECGD_OUTPUT_SIZE] = {0.0}; // Array to hold time values for output data

/**
 * @brief Applies a low-pass filter to the data
 * @param data_in Buffer containing incoming samples
 * @param data_len Length of incoming data
 * @param data_out Buffer to contain outcoming samples
 */
void ecgd_antialias_filter(float *data_in, uint16_t data_len, float *data_out)
{
    int32_t sample_in  = 0;    // temporary Variable to hold input data
    int32_t sample_out = 0;    // temporary Variable to hold output data

    // Check arguments
    if (data_in == NULL || data_out == NULL || data_len == 0)
    {
        return;
    }

    // Anti-aliasing
    for (uint16_t n = 0; n < data_len; n++)
    {
        // Fetch input sample from input buffer and reset output sample
        sample_in = data_in[n];

        // Update input array
        gpPreFilterBuffer[0] = gpPreFilterBuffer[1];
        gpPreFilterBuffer[1] = gpPreFilterBuffer[2];
        gpPreFilterBuffer[2] = sample_in;

        // Apply filtering
        sample_out = 0;
        for (uint16_t k = 0; k < ECGD_FILTER_SIZE; k++)
        {
            sample_out += coeff_b[k] * gpPreFilterBuffer[2 - k];
        }
        for (uint16_t k = 0; k < (ECGD_FILTER_SIZE - 1); k++)
        {
            sample_out -= coeff_a[k + 1] * gpPostFilterBuffer[3 - k - 1];
        }

        // 2D) Update output and store result in output buffer
        gpPostFilterBuffer[0] = gpPostFilterBuffer[1];
        gpPostFilterBuffer[1] = gpPostFilterBuffer[2];
        gpPostFilterBuffer[2] = sample_out;
        data_out[n]           = sample_out;
    }
}

/**
 * @brief Downsamples data using linear interpolation. Do not call this directly
 * on unfiltered data, as it will cause artifacts. Instead, call
 * antialiasing_process_data on each chunk of data.
 * @param fs_inp Sampling frequency of the incoming data
 * @param fs_out Desired output frequency
 * @param data_in Buffer containing incoming samples
 * @param data_len Length of incoming data
 * @param data_out Buffer to contain outcoming samples
 * @param data_len_out Length of resampled data
 */
void ecgd_down_sample(float fs_inp, float fs_out, int32_t *data_in, uint16_t data_len, int32_t *data_out, uint16_t *data_len_out)
{
    uint8_t  id_nxt_packet    = 0;                                    // Index for the next packet
    uint16_t start_p          = 0;                                    // Start index for interpolation
    uint16_t end_p            = 0;                                    // End index for interpolation
    float    multiplier_coeff = 0.0;                                  // Coefficient for linear interpolation
    float    total_time       = (float)data_len / fs_inp;             // Total time duration of the input data
    size_t   resample_len     = (size_t)(total_time * fs_out) + 1;    // Length of resampled data

    // Check arguments
    if (fs_inp == 0 || fs_out == 0 || data_in == NULL || data_out == NULL || data_len == 0)
    {
        return;
    }

    // Initialize arrays
    for (int i = 0; i < resample_len; i++)
    {
        gpOutputTimeIntervals[i] = (float)i / fs_out;    // Calculate time values for output data
    }

    for (int i = 0; i < data_len; i++)
    {
        gpInputTimeIntervals[i] = (float)i / fs_inp;    // Calculate time values for input data
    }

    // Adjust resample_len and dt_out if output time exceeds input time
    if (gpOutputTimeIntervals[resample_len - 1] > gpInputTimeIntervals[data_len - 1])
    {
        for (int i = 0; i < resample_len; i++)
        {
            if (gpOutputTimeIntervals[i] > gpInputTimeIntervals[data_len - 1])
            {
                // Find the index where output time exceeds input time
                id_nxt_packet = i;
                break;
            }
        }
        resample_len = id_nxt_packet;    // Update resample_len
    }

    *data_len_out = resample_len;

    // Downsampling Loop
    for (uint16_t i = 0; i < resample_len; i++)
    {
        // Find the start index for interpolation
        for (uint16_t j = 1; j < data_len; j++)
        {
            if (gpInputTimeIntervals[j] > gpOutputTimeIntervals[i])
            {
                start_p = j - 1;
                break;
            }
        }

        // Find the end index for interpolation
        if (gpInputTimeIntervals[data_len - 1] > gpOutputTimeIntervals[i])
        {
            for (uint16_t j = 0; j < data_len; j++)
            {
                if (gpInputTimeIntervals[j] >= gpOutputTimeIntervals[i])
                {
                    end_p = j;
                    break;
                }
            }

            // Perform linear interpolation
            if (gpInputTimeIntervals[end_p] == gpInputTimeIntervals[start_p])
            {
                data_out[i] = data_in[start_p];
            }
            else
            {
                multiplier_coeff = (float)((data_in[end_p] - data_in[start_p]) / (gpInputTimeIntervals[end_p] - gpInputTimeIntervals[start_p]));
                data_out[i]      = data_in[start_p] + multiplier_coeff * (gpOutputTimeIntervals[i] - gpInputTimeIntervals[start_p]);
            }
        }
    }
}

void ecgd_process_data(float fs_inp, float fs_out, float *data_in, uint16_t data_len, float *data_out, uint16_t *out_len)
{
    // Check arguments
    if (data_in == NULL || data_out == NULL || data_len == 0 || fs_inp == 0 || fs_out == 0)
    {
        return;
    }

    // Initialize packet expansion and filters
    if (!gfCnt)
    {
        gpPreFilterBuffer[0] = gpPreFilterBuffer[1] = gpPreFilterBuffer[2] = data_in[0];
        
        // Prepend the first five values of the packet with first value
        for (uint8_t i = 0; i < 5; i++)
        {
            gpPaddedBuffer[i] = data_in[0];
        }
        memcpy(&gpPaddedBuffer[5], data_in, (sizeof(int32_t) * data_len));

        ecgd_antialias_filter(gpPaddedBuffer, data_len + 5, gpFilteredData);
        // Adjust filtered data
        memmove(gpFilteredData, &gpFilteredData[5], (sizeof(float) * data_len));
    }
    else
    {
        // Update filter input/output
        memcpy(gpPaddedBuffer, data_in, (sizeof(int32_t) * data_len));

        // Store last sampled data
        gbLastSampledData = gpFilteredData[data_len - 1];
        ecgd_antialias_filter(data_in, data_len, gpFilteredData);
        
        // Adjust filtered data
        memmove(&gpFilteredData[1], gpFilteredData, sizeof(gpFilteredData) - sizeof(float));
        gpFilteredData[0] = gbLastSampledData;
    }

    // Perform downsampling with antialiasing
    ecgd_down_sample(fs_inp, fs_out, gpFilteredData, data_len, data_out, out_len);
    gfCnt = true;    // Update flag for subsequent calls
}

int main() {
    static int32_t output[256] = {0};
    static int32_t outputs[8] = {0};
    static int32_t outputf[8] = {0};
    static uint16_t len = 0;

    for (int i = 0; i < 8; i++) 
    {
        ecgd_process_data(409.6, 320.0, input + (32 * i), 32, output + (25 * i), &len);
    }

    // printf("Input: \n");
    // for (int i = 0; i < 256; i++) {
    //     printf("%d, %d\n", i, input[i]);
    // }

    printf("Output: \n");
    for (int i = 0; i < 200; i++) {
        printf("%d, %d\n", i, output[i]);
    }

    return 0;
}
