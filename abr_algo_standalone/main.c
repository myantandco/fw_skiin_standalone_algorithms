#include "input.h"
#include "myant/abr_postprocess.h"
#include "myant/abr_preprocess.h"
#include "myant/data_processing.h"
#include "myant/ecg_algo.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define GARMENT_ID_DEFAULT \
    GARMENT_UNDERWEAR    // Assume underwear, all other garments function the
                         // same way
#define NOTCH_FILTER_FREQ            false    // False = 60 Hz, True = 50 Hz

#define ECG_ROLLING_DATA_BUFFER_SIZE 3
#define ECG_ALGO_OUTPUT_SIZE         1
#define ECG_DATA_BUFFER_SIZE         24

static double            ecg_data_buffer_channel_1[ECG_DATA_BUFFER_SIZE] = {0};
static double            ecg_data_buffer_channel_2[ECG_DATA_BUFFER_SIZE] = {0};
static double            ecg_data_buffer_channel_3[ECG_DATA_BUFFER_SIZE] = {0};
static volatile uint32_t ecg_data_count                                  = 0;
static volatile uint32_t sample_count                                    = 0;

int                      main(int argc, const char *argv[])
{
    // Flags
    volatile bool ret     = false;    // Return boolean
    volatile bool restart = true;     // Set true for the first time only, then
                                      // false afterwards

    // Intermediate data
    double        buffer[ECG_ROLLING_DATA_BUFFER_SIZE] = {0.0};
    float         algo_output[ECG_ALGO_OUTPUT_SIZE]    = {0};

    // Outputs
    uint8_t       rpeak_max   = 0;
    uint8_t       rpeak_index = 0;
    uint8_t       q_class     = 0;
    uint8_t       slope       = 0;

    // Inputs - hard-coded for now but in future update to pass by arguments
    printf("Setting inputs...\r\n");
    ecg_algo_update_garmentid(GARMENT_ID_DEFAULT);
    abr_update_notch_filter_coeff(NOTCH_FILTER_FREQ);

    // Initialize the ECG algorithm
    printf("Initializing algorithm...\r\n");
    ecg_algo_init();

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < INPUT_ARRAY_SIZE; i++)
    {
        // Load 1 sample into data buffers by channel
        // This assumes data is already converted from raw ADC values to mV
        ecg_data_buffer_channel_1[ecg_data_count] = input_ch1[i];
        ecg_data_buffer_channel_2[ecg_data_count] = input_ch2[i];
        ecg_data_buffer_channel_3[ecg_data_count] = input_ch3[i];

        // Pass 3 samples on a rolling basis to the local buffer from channel
        // buffers
        buffer[ECG1] = ecg_data_buffer_channel_1[i];
        buffer[ECG2] = ecg_data_buffer_channel_2[i];
        buffer[ECG3] = ecg_data_buffer_channel_3[i];

        // ECG Algorithm
        ret = ecg_algo_run(buffer, ECG_ROLLING_DATA_BUFFER_SIZE, restart);
        if (ret)
        {
            printf("ecg_algo_run error %d\r\n", ret);
            printf("Exiting...\r\n");
            return -1;
        }

        ecg_algo_get_output(algo_output, ECG_ALGO_OUTPUT_SIZE);
        abr_pp_rpeak(algo_output[0], ecg_data_count);

        // Increment count
        ecg_data_count++;

        // Disable restart as we only need to do this on the first iteration
        restart = false;

        if (ecg_data_count >= ECG_DATA_BUFFER_SIZE)
        {
            for (uint8_t j = 0; j < MAX_ECG; j++)
            {
                abr_pp_get_rpeak(&rpeak_max, &rpeak_index);
                abr_prep_get_quality((ecg_sens_id)j, &q_class, &slope);

                printf("Sample[%d] ECG Channel %d: rpeak_max = %d rpeak_index "
                       "= %d, q_class = %d, slope = %d...\r\n",
                       sample_count,
                       j,
                       rpeak_max,
                       rpeak_index,
                       q_class,
                       slope);
            }
            printf("\r\n");

            sample_count++;
            ecg_data_count = 0;
        }
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}
