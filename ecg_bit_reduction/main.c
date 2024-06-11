#include "ecg_bit_reduction.h"
#include "csv_writers.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ECG_DATA_BUFFER_SIZE         24

int main(int argc, const char *argv[])
{
    // Inputs
    float dpInput[MAX_ROWS] = {0};
    int   bNumRows          = 0;

    // Intermediate data
    bool ecg_buffer_refresh = true;
    uint16_t ecg_reduced_data = 0;

    // Write CSV header - TODO: Gloria
    const char *pVarNames = "output";
    CSVW_WriteCSVHeader("out.csv", pVarNames);

    // Read Inputs from CSV:
    printf("Setting inputs...\r\n");
    CSVW_ReadCSV(INPUT_FILE_NAME, dpInput, NULL, NULL, &bNumRows);

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < bNumRows; i++)
    {
        // ECG Bit Reduction Algorithm
        ecg_reduced_data = ecgbr_sample_reduction((uint32_t)dpInput[i], ECG1, ecg_buffer_refresh);

        // Write algorithm output to CSV 
        CSVW_WriteCSVSingle("out.csv", ecg_reduced_data, ECG1);
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}
