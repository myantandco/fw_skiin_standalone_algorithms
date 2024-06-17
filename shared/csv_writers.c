#include "csv_writers.h"
#include <errno.h>      // for EOVERFLOW
#include <fileapi.h>    // for CreateDirectoryA, GetFileAttributesA, CreateD...
#include <minwindef.h>  // for DWORD, FILE_ATTRIBUTE_DIRECTORY
#include <stdio.h>      // for fprintf, fclose, NULL, fopen, printf, snprintf
#include <stdlib.h>     // for errno, atof
#include <string.h>     // for strtok

#define RES_FOLDER "example_data/example_res4/"
#define BOOL_OUTPUT_CSV 1

#define MAX_COLS        3      // maximum number of columns in the CSV file
#define MAX_PATH_LEN    200    // maximum number of chars in the CSV filepath
#define MAX_LINE_LEN    1024

static int CSVW_FolderExists(const char *pFolderName)
{
    DWORD dwAttrib = GetFileAttributes(pFolderName);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static int CSVW_CreateFolderIfNotExists(const char *pFolderName)
{
    int ret = 0;

    if (!CSVW_FolderExists(pFolderName))
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

int CSVW_ReadCSV(const char *pFileName, float pdInputCh1[], float pdInputCh2[], float pdInputCh3[], int *bNumRows)
{
    int col = 0;
    char *pToken = NULL;
    char pLine[MAX_LINE_LEN] = {0}; // Declare a buffer to store each line from the file

    // 1) Check arguments
    if (!pFileName || !pdInputCh1 || !pdInputCh2 || !pdInputCh3 || !bNumRows)
    {
        return -errno;
    }

    // 2) Open the CSV file for reading
    FILE *pFile = fopen(pFileName, "r");
    if (pFile == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        return -errno;
    }

    // 3) Explicitly clear num rows
    *bNumRows = 0;

    // 4) Read and parse each line from the CSV file
    while (fgets(pLine, sizeof(pLine), pFile) != NULL)
    {
        // Check if we exceeded maximum number of rows
        if (*bNumRows >= MAX_ROWS)
        {
            fprintf(stderr, "Warning: Maximum number of rows exceeded in ReadCsv. Closing file.\n");
            fclose(pFile);  
            return -EOVERFLOW;  // Value too large for defined data type
        }

        // Tokenize the line using strtok
        pToken = strtok(pLine, ",");
        col = 0;

        while (pToken != NULL && col < MAX_COLS)
        {
            // Convert token to float and store in the appropriate array
            {
                switch (col)
                {
                    case 0:
                    {
                        pdInputCh1[*bNumRows] = (float)atof(pToken);
                        break;
                    }
                    case 1:
                    {
                        pdInputCh2[*bNumRows] = (float)atof(pToken);
                        break;
                    }
                    case 2:
                    {
                        pdInputCh3[*bNumRows] = (float)atof(pToken);
                        break;
                    }
                }
            }

            // Move to the next token
            pToken = strtok(NULL, ",");
            col++;
        }

        // Increment row counter
        (*bNumRows)++;
    }

    // 5) Close the CSV file
    if (fclose(pFile) != 0)
    {
        perror("Error closing file");
        return -errno;
    }

    return 0;
}

int CSVW_WriteCSVHeader(const char *pFileName, const char *pVarNames)
{
    FILE *file = NULL;
    char pFilePath[MAX_PATH_LEN] = {0}; // Adjust the size according to your need

    // 1) Check arguments
    if (!pFileName || !pVarNames)
    {
        return -errno;
    }

    // 2) Create folder
    CSVW_CreateFolderIfNotExists(RES_FOLDER);
    
    // 3) Fetch path to file
    snprintf(pFilePath, sizeof(pFilePath), "%s/%s", RES_FOLDER, pFileName);

#ifdef BOOL_OUTPUT_CSV
    // 4) Open file
    file = fopen(pFilePath, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", pFileName);
        return -errno;
    }

    // 5) Write the header row with variable names
    fprintf(file, "%s\n", pVarNames);

    // 6) Close file
    fclose(file);
#endif

    return 0;
}

int CSVW_WriteCSVRow(const char *pFileName, float pdData[], int bNumVars)
{
    FILE *file = NULL;
    char filePath[MAX_PATH_LEN] = {0};  // Adjust the size according to your need

    // 1) Check arguments
    if (!pFileName || !pdData || bNumVars == 0)
    {
        return -errno;
    }

    // 2) Create folder
    CSVW_CreateFolderIfNotExists(RES_FOLDER);
    
    // 3) Fetch path to file
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, pFileName);

#ifdef BOOL_OUTPUT_CSV
    // 4) Open file
    file = fopen(filePath, "a");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", pFileName);
        return -errno;
    }

    // 5) Write the data row
    fprintf(file, "%.2f", pdData[0]);
    for (int i = 1; i < bNumVars; i++)
    {
        fprintf(file, ",%.2f", pdData[i]);
    }
    fprintf(file, "\n");

    // 6) Close file
    fclose(file);
#endif

    return 0;
}

int CSVW_WriteCSVSingle(const char *pFileName, float dData, int bEcgCh)
{
    FILE *file = NULL;
    char filePath[MAX_PATH_LEN] = {0};  // Adjust the size according to your need

    // 1) Check arguments
    if (!pFileName)
    {
        return -errno;
    }

    // 2) Create folder
    CSVW_CreateFolderIfNotExists(RES_FOLDER);
    
    // 3) Fetch path to file
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, pFileName);

#ifdef BOOL_OUTPUT_CSV
    // 4) Open file
    file = fopen(filePath, "a");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", pFileName);
        return -errno;
    }

    // 5) Write the data row
    if (bEcgCh == 2)
    {
        fprintf(file, "%.7f", dData);
        fprintf(file, "\n");
    }
    else
    {
        fprintf(file, "%.7f,", dData);
    }

    // 6) Close file
    fclose(file);
#endif

    return 0;
}

void CVSW_PrintVar(float dVar, int bEcgCh)
{
    // 1) Print value to terminal
    printf("%f, ", dVar);
    if (bEcgCh == 2)
    {
        printf("\r\n");
    }
}
