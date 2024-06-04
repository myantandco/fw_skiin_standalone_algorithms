#ifndef CSV_WRITERS_H_
#define CSV_WRITERS_H_

#include "csv_writers.h"

#define INPUT_FILE_NAME "example_data/196_TIT01B-ID01-T1.csv"
#define MAX_ROWS 100 // 9600 // maximum number of rows in the CSV file

// Function prototypes
int CSVW_ReadCSV(const char *pFilename, float pdInputCh1[], float pdInputCh2[], float pdInputCh3[], int *bNumRows);
int CSVW_WriteCSVHeader(const char *pFileName, const char *pVarNames);
int CSVW_WriteCSVRow(const char *pFilename, float pdData[], int bNumVars);
int CSVW_WriteCSVSingle(const char *pFileName, float dData, int bEcgCh);
void CVSW_PrintVar(float dVar, int bEcgCh);

#endif
