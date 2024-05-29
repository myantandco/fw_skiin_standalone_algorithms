#include "myant/abr_postprocess.h"
#include "myant/abr_preprocess.h"
#include "myant/csv_writers.h"
#include "myant/data_processing.h"
#include "myant/ecg_algo.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define GARMENT_ID_DEFAULT           GARMENT_CHEST_BAND    // Assume chestband for now
#define NOTCH_FILTER_FREQ            false    // False = 60 Hz, True = 50 Hz

#define ECG_ROLLING_DATA_BUFFER_SIZE 3
#define ECG_ALGO_OUTPUT_SIZE         1
#define ECG_DATA_BUFFER_SIZE         24

static volatile uint32_t ecg_data_count = 0;
static volatile uint32_t sample_count   = 0;

int                      main(int argc, const char *argv[])
{
    // Inputs
    float         dpInputCh1[MAX_ROWS];
    float         dpInputCh2[MAX_ROWS];
    float         dpInputCh3[MAX_ROWS];
    int           bNumRows;

    // Flags
    volatile bool ret     = false;    // Return boolean
    volatile bool restart = true;     // Set true for the first time only, then
                                      // false afterwards

    // Intermediate data
    float         buffer[ECG_ROLLING_DATA_BUFFER_SIZE] = {0.0};
    float         algo_output[ECG_ALGO_OUTPUT_SIZE]    = {0};

    // Outputs
    uint8_t       bRpeakMax   = 0;
    uint8_t       bRpeakIndex = 0;
    uint8_t       fQClass     = 0;
    uint8_t       bSlope      = 0;
    float         pdBleOuts[7];
    const char   *pVarNames = "rp_idx,rp_val,q1,q2,q3,slope1,slope2,slope3";
    csvw_WriteCsvHeader("ble.csv", pVarNames);

    // Read Inputs from CSV
    printf("Setting inputs...\r\n");
    csvw_ReadCsv(INPUT_FILE_NAME, dpInputCh1, dpInputCh2, dpInputCh3, &bNumRows);
    ecg_algo_update_garmentid(GARMENT_ID_DEFAULT);
    abr_update_notch_filter_coeff(NOTCH_FILTER_FREQ);

    // Initialize the ECG algorithm
    printf("Initializing algorithm...\r\n");
    ecg_algo_init();

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < bNumRows; i++)
    {
        // Pass 3 samples on a rolling basis to the local buffer
        // This assumes data is already converted from raw ADC
        // values to mV but still has baseline
        buffer[ECG1] = dpInputCh1[i] + ABR_INPUT_BASELINE_VALUE;
        buffer[ECG2] = dpInputCh2[i] + ABR_INPUT_BASELINE_VALUE;
        buffer[ECG3] = dpInputCh3[i] + ABR_INPUT_BASELINE_VALUE;

        // ECG Algorithm - preprocess and run the model
        ret = ecg_algo_run(buffer, ECG_ROLLING_DATA_BUFFER_SIZE, restart);
        if (ret)
        {
            printf("ecg_algo_run error %d\r\n", ret);
            printf("Exiting...\r\n");
            return -1;
        }

        // Get the algorithm outputs
        ecg_algo_get_output(algo_output, ECG_ALGO_OUTPUT_SIZE);
        csvw_WriteCsvSingle("e4_pred.csv", algo_output[0], 2);

        // Postprocess algo output to rpeak info,
        // reset at the end of every packet
        abr_pp_rpeak(algo_output[0], ecg_data_count);

        // Increment count
        ecg_data_count++;

        // Disable restart as we only need to do this on the first iteration
        restart = false;

        if (ecg_data_count >= ECG_DATA_BUFFER_SIZE)
        {
            abr_pp_get_rpeak(&bRpeakMax, &bRpeakIndex);
            pdBleOuts[0] = (float)bRpeakIndex;
            pdBleOuts[1] = (float)bRpeakMax;
            for (uint8_t j = 0; j < MAX_ECG; j++)
            {
                abr_prep_get_quality((ecg_sens_id)j, &fQClass, &bSlope);
                pdBleOuts[2 + j] = (float)fQClass;
                pdBleOuts[5 + j] = (float)bSlope;
            }

            sample_count++;
            ecg_data_count = 0;
            csvw_WriteCsvRow("ble.csv", pdBleOuts, 8);
        }
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}
