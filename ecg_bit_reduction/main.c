#include "csv_writers.h"
#include "ecg_bit_reduction.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ECG_DATA_BUFFER_SIZE 24

int main(int argc, const char *argv[])
{
    // Inputs
    float dpInput[MAX_ROWS] = {0};
    float dpDummyInput1[2]  = {0};
    float dpDummyInput2[2]  = {0};
    int   bNumRows          = 0;

    // Intermediate data
    bool    fECGBufferRefresh = true;
    int16_t bECGReducedData   = 0;

    // Read Inputs from CSV:
    printf("Setting inputs...\r\n");
    CSVW_ReadCSV(INPUT_FILE_NAME, dpInput, dpDummyInput1, dpDummyInput2, &bNumRows);

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < bNumRows; i++)
    {
        // ECG Bit Reduction Algorithm
        bECGReducedData = ECGBitReduction_SampleReduction((uint32_t)dpInput[i], ECG1, fECGBufferRefresh);

        // Write algorithm output to CSV
        CSVW_WriteCSVSingle("in_ch1.csv", dpInput[i], ECG3);
        CSVW_WriteCSVSingle("out1.csv", bECGReducedData, ECG3);

        // Only refresh on the first iteration of the loop
        fECGBufferRefresh = false;
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}
