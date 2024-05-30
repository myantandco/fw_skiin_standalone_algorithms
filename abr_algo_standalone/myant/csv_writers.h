#ifndef CSV_WRITERS_H_
#define CSV_WRITERS_H_

#include "csv_writers.h"
#include <errno.h>      // for EOVERFLOW
#include <fileapi.h>    // for CreateDirectoryA, GetFileAttributesA, CreateD...
#include <minwindef.h>    // for DWORD, FILE_ATTRIBUTE_DIRECTORY
#include <stdio.h>        // for fprintf, fclose, NULL, fopen, printf, snprintf
#include <stdlib.h>       // for errno, atof
#include <string.h>       // for strtok
#define BOOL_OUTPUT_CSV 1

// Function prototypes
int csvw_ReadCsv(const char *pFilename, float pdInputCh1[], float pdInputCh2[], float pdInputCh3[], int *bNumRows);

int  csvw_WriteCsvHeader(const char *pFileName, const char *pVarNames);

int  csvw_WriteCsvRow(const char *pFilename, float pdData[], int bNumVars);

int  csvw_WriteCsvSingle(const char *pFileName, float dData, int bEcgCh);

void csvw_PrintVar(float dVar, int bEcgCh);

#endif
