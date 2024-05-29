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
int csvw_ReadCsv(const char *pFilename, float pdInputCh1[], float pdInputCh2[], float pdInputCh3[], int *bNumRows);

int  csvw_WriteCsvHeader(const char *pFileName, const char *pVarNames);

int  csvw_WriteCsvRow(const char *pFilename, float pdData[], int bNumVars);

int  csvw_WriteCsvSingle(const char *pFileName, float dData, int bEcgCh);

void csvw_PrintVar(float dVar, int bEcgCh);

#endif
