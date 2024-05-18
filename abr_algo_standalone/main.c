#include "config.h"
#include "myant/abr_postprocess.h"
#include "myant/abr_preprocess.h"
#include "myant/csv_writers.h"
#include "myant/data_processing.h"
#include "myant/ecg_algo.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define GARMENT_ID_DEFAULT \
    GARMENT_CHEST_BAND    // Assume underwear, all other garments function the
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
    float         ble[7];
    const char   *var_names = "rp_idx,rp_val,q1,q2,q3,slope1,slope2,slope3";
    write_csv_header("ble.csv", var_names);

    // Inputs - hard-coded for now but in future update to pass by arguments
    printf("Setting inputs...\r\n");
    float input_ch1[MAX_ROWS];
    float input_ch2[MAX_ROWS];
    float input_ch3[MAX_ROWS];
    int   num_rows;
    read_csv(INPUT_FILE_NAME, input_ch1, input_ch2, input_ch3, &num_rows);
    ecg_algo_update_garmentid(GARMENT_ID_DEFAULT);
    abr_update_notch_filter_coeff(NOTCH_FILTER_FREQ);

    // Initialize the ECG algorithm
    printf("Initializing algorithm...\r\n");
    ecg_algo_init();

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < num_rows; i++)
    {
        // Pass 3 samples on a rolling basis to the local buffer from channel
        // This assumes data is already converted from raw ADC values to mV but
        // still has baseline
        buffer[ECG1] = input_ch1[i] + ABR_INPUT_BASELINE_VALUE;
        buffer[ECG2] = input_ch2[i] + ABR_INPUT_BASELINE_VALUE;
        buffer[ECG3] = input_ch3[i] + ABR_INPUT_BASELINE_VALUE;

        // ECG Algorithm
        ret = ecg_algo_run(buffer, ECG_ROLLING_DATA_BUFFER_SIZE, restart);
        if (ret)
        {
            printf("ecg_algo_run error %d\r\n", ret);
            printf("Exiting...\r\n");
            return -1;
        }

        ecg_algo_get_output(algo_output, ECG_ALGO_OUTPUT_SIZE);
        write_csv_single("e4_pred.csv", algo_output[0], 2);
        abr_pp_rpeak(algo_output[0], ecg_data_count);

        // Increment count
        ecg_data_count++;

        // Disable restart as we only need to do this on the first iteration
        restart = false;

        if (ecg_data_count >= ECG_DATA_BUFFER_SIZE)
        {
            abr_pp_get_rpeak(&rpeak_max, &rpeak_index);
            ble[0] = (float)rpeak_index;
            ble[1] = (float)rpeak_max;
            for (uint8_t j = 0; j < MAX_ECG; j++)
            {
                abr_prep_get_quality((ecg_sens_id)j, &q_class, &slope);
                // printf("Sample[%d] ECG Channel %d: rpeak_max = %d rpeak_index
                // "
                //        "= %d, q_class = %d, slope = %d...\r\n",
                //        sample_count,
                //        j,
                //        rpeak_max,
                //        rpeak_index,
                //        q_class,
                //        slope);
                ble[2 + j] = (float)q_class;
                ble[5 + j] = (float)slope;
            }
            // printf("\r\n");
            sample_count++;
            ecg_data_count = 0;
            write_csv_row("ble.csv", ble, 8);
        }
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}
