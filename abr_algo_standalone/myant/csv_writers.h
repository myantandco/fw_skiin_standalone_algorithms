#ifndef CSV_WRITERS_H_
#define CSV_WRITERS_H_

#include "csv_writers.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>    // For mkdir()

#define RES_FOLDER      "example_data/354"
#define INPUT_FILE_NAME "example_data/354_TIT01C-ID06-T2.csv"
#define MAX_ROWS        96000    // maximum number of rows in the CSV file
#define BOOL_OUTPUT_CSV 1

// Function prototypes
void csvw_CreateFolderIfNotExists(const char *folderName);

void csvw_ReadCsv(const char *filename, float input_ch1[], float input_ch2[], float input_ch3[], int *num_rows);

void csvw_WriteCsvHeader(const char *filename, const char *var_names);

void csvw_WriteCsvRow(const char *filename, float data[], int num_vars);

void csvw_WriteCsvSingle(const char *filename, float data, int ecg_ch);

void csvw_PrintVar(float var, int ecg_id);

#endif
