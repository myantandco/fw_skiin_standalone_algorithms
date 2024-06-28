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
    float dpInput1[MAX_ROWS] = {0};
    float dpInput2[MAX_ROWS] = {0};
    float dpInput3[MAX_ROWS] = {0};
    int   bNumRows           = 0;

    // Intermediate data
    bool fECGBufferRefresh = true;    // apply whenever pod is reset, or there
                                      // is missing data
    int16_t bECGReducedData = 0;

    // Read Inputs from CSV:
    printf("Setting inputs...\r\n");
    CSVW_ReadCSV(INPUT_FILE_NAME, dpInput1, dpInput2, dpInput3, &bNumRows);

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < bNumRows; i++)
    {
        // ECG Bit Reduction Algorithm
        bECGReducedData = ECGBitReduction_SampleReduction((uint32_t)dpInput1[i], ECG1, fECGBufferRefresh);
        CSVW_WriteCSVSingle("in_all.csv", dpInput1[i], ECG1);
        CSVW_WriteCSVSingle("out_all.csv", (float)bECGReducedData, ECG1);
        bECGReducedData = ECGBitReduction_SampleReduction((uint32_t)dpInput2[i], ECG2, fECGBufferRefresh);
        CSVW_WriteCSVSingle("in_all.csv", dpInput2[i], ECG2);
        CSVW_WriteCSVSingle("out_all.csv", (float)bECGReducedData, ECG2);
        bECGReducedData = ECGBitReduction_SampleReduction((uint32_t)dpInput3[i], ECG3, fECGBufferRefresh);
        CSVW_WriteCSVSingle("in_all.csv", dpInput3[i], ECG3);
        CSVW_WriteCSVSingle("out_all.csv", (float)bECGReducedData, ECG3);

        // Write algorithm output to CSV

        // Only refresh on the first iteration of the loop
        fECGBufferRefresh = false;
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}
