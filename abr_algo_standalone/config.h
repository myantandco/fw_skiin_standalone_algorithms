#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// These data buffer acts an input for ECG Channels and expects sample values in units of mV

#define RES_FOLDER "example_data/example_res4/"
#define INPUT_FILE_NAME "example_data/196_TIT01B-ID01-T1.csv"
#define MAX_ROWS 100 // 9600 // maximum number of rows in the CSV file
#define BOOL_OUTPUT_CSV 1

#endif // CONFIG_H