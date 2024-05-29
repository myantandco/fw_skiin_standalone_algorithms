#include "csv_writers.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>       // For mkdir()
#include <windows.h>        // For Windows-specific functions

#define MAX_COLS     3      // maximum number of columns in the CSV file
#define MAX_PATH_LEN 200    // maximum number of chars in the CSV filepath
#define MAX_LINE_LEN 1024

static int csvw_FolderExists(const char *pFolderName)
{
    DWORD dwAttrib = GetFileAttributes(pFolderName);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static int csvw_CreateFolderIfNotExists(const char *pFolderName)
{
    int ret;
    if (!csvw_FolderExists(pFolderName))
    {
        if (CreateDirectory(pFolderName, NULL))
        {
            printf("Folder created successfully: %s\n", pFolderName);
        }
        else
        {
            printf("Error creating folder: %s\n", pFolderName);
            return -errno;
        }
    }
    return 0;
}

int csvw_ReadCsv(const char *pFilename, float pdInputCh1[], float pdInputCh2[], float pdInputCh3[], int *bNumRows)
{
    // Open the CSV file for reading
    FILE *pFile = fopen(pFilename, "r");
    if (pFile == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        return -errno;
    }

    // Declare a buffer to store each line from the file
    char pLine[MAX_LINE_LEN];

    // Read and parse each line from the CSV file
    *bNumRows = 0;
    while (fgets(pLine, sizeof(pLine), pFile) != NULL)
    {
        if (*bNumRows >= MAX_ROWS)
        {
            fprintf(stderr, "Error: Maximum number of rows exceeded\n");
            fclose(pFile);
            errno = EOVERFLOW;    // Value too large for defined data type
            return -errno;
        }

        // Tokenize the line using strtok
        char *pToken = strtok(pLine, ",");
        int   col    = 0;
        while (pToken != NULL && col < MAX_COLS)
        {
            // Convert token to float and store in the appropriate array
            {
                switch (col)
                {
                    case 0:
                        pdInputCh1[*bNumRows] = (float)atof(pToken);
                        break;
                    case 1:
                        pdInputCh2[*bNumRows] = (float)atof(pToken);
                        break;
                    case 2:
                        pdInputCh3[*bNumRows] = (float)atof(pToken);
                        break;
                }
            }
            // Move to the next token
            pToken = strtok(NULL, ",");
            col++;
        }
        (*bNumRows)++;
    }

    // Close the CSV file
    if (fclose(pFile) != 0)
    {
        perror("Error closing file");
        return -errno;
    }

    return 0;
}

int csvw_WriteCsvHeader(const char *pFileName, const char *pVarNames)
{
    csvw_CreateFolderIfNotExists(RES_FOLDER);
    char pFilePath[MAX_PATH_LEN];    // Adjust the size according to your need
    snprintf(pFilePath, sizeof(pFilePath), "%s/%s", RES_FOLDER, pFileName);
    if (BOOL_OUTPUT_CSV)
    {
        FILE *file = fopen(pFilePath, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file: %s\n", pFileName);
            return -errno;
        }

        // Write the header row with variable names
        fprintf(file, "%s\n", pVarNames);
        fclose(file);
    }
    return 0;
}

int csvw_WriteCsvRow(const char *pFilename, float pdData[], int bNumVars)
{
    csvw_CreateFolderIfNotExists(RES_FOLDER);
    char filePath[MAX_PATH_LEN];    // Adjust the size according to your need
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, pFilename);
    if (BOOL_OUTPUT_CSV)
    {
        FILE *file = fopen(filePath, "a");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file: %s\n", pFilename);
            return -errno;
        }

        // Write the data row
        fprintf(file, "%.2f", pdData[0]);
        for (int i = 1; i < bNumVars; i++)
        {
            fprintf(file, ",%.2f", pdData[i]);
        }
        fprintf(file, "\n");

        fclose(file);
    }
    return 0;
}

int csvw_WriteCsvSingle(const char *pFileName, float dData, int bEcgCh)
{
    csvw_CreateFolderIfNotExists(RES_FOLDER);
    char filePath[MAX_PATH_LEN];    // Adjust the size according to your need
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, pFileName);
    if (BOOL_OUTPUT_CSV)
    {
        FILE *file = fopen(filePath, "a");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file: %s\n", pFileName);
            return -errno;
        }

        // Write the data row
        if (bEcgCh == 2)
        {
            fprintf(file, "%.7f", dData);
            fprintf(file, "\n");
        }
        else
        {
            fprintf(file, "%.7f,", dData);
        }

        fclose(file);
    }
    return 0;
}

void csvw_PrintVar(float dVar, int bEcgCh)
{
    printf("%f, ", dVar);
    if (bEcgCh == 2)
    {
        printf("\r\n");
    }
}
