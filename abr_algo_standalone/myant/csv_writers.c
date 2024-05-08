#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h> // For mkdir()
#include <windows.h>  // For Windows-specific functions
#include "config.h"
#include "csv_writers.h"

#define MAX_COLS 3 // maximum number of columns in the CSV file

int folderExists(const char *folderName)
{
    DWORD dwAttrib = GetFileAttributes(folderName);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void createFolderIfNotExists(const char *folderName)
{
    if (!folderExists(folderName))
    {
        if (CreateDirectory(folderName, NULL))
        {
            printf("Folder created successfully: %s\n", folderName);
        }
        else
        {
            printf("Error creating folder: %s\n", folderName);
        }
    }
}

void read_csv(const char *filename, float input_ch1[], float input_ch2[], float input_ch3[], int *num_rows)
{
    // Open the CSV file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        return;
    }

    // Declare a buffer to store each line from the file
    char line[1024];

    // Read and parse each line from the CSV file
    *num_rows = 0;
    while (fgets(line, sizeof(line), file) != NULL && *num_rows < MAX_ROWS)
    {
        // Tokenize the line using strtok
        char *token = strtok(line, ",");
        int col = 0;
        while (token != NULL && col < MAX_COLS)
        {
            // Convert token to float and store in the appropriate array
            {
                switch (col)
                {
                case 0:
                    input_ch1[*num_rows] = atof(token);
                    break;
                case 1:
                    input_ch2[*num_rows] = atof(token);
                    break;
                case 2:
                    input_ch3[*num_rows] = atof(token);
                    break;
                }
            }
            // Move to the next token
            token = strtok(NULL, ",");
            col++;
        }
        (*num_rows)++;
    }

    // Close the CSV file
    fclose(file);
}

void write_csv_header(const char *filename, const char **var_names, int num_vars)
{
    createFolderIfNotExists(RES_FOLDER);
    char filePath[200]; // Adjust the size according to your need
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, filename);
    if (BOOL_OUTPUT_CSV)
    {
        FILE *file = fopen(filename, "w");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file: %s\n", filename);
            return;
        }

        // Write the header row with variable names
        fprintf(file, "%s", var_names[0]);
        for (int i = 1; i < num_vars; i++)
        {
            fprintf(file, ",%s", var_names[i]);
        }
        fprintf(file, "\n");

        fclose(file);
    }
}

void write_csv_row(const char *filename, float data[], int num_vars)
{
    createFolderIfNotExists(RES_FOLDER);
    char filePath[200]; // Adjust the size according to your need
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, filename);
    if (BOOL_OUTPUT_CSV)
    {
        FILE *file = fopen(filename, "a");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file: %s\n", filename);
            return;
        }

        // Write the data row
        fprintf(file, "%.2f", data[0]);
        for (int i = 1; i < num_vars; i++)
        {
            fprintf(file, ",%.2f", data[i]);
        }
        fprintf(file, "\n");

        fclose(file);
    }
}

void write_csv_single(const char *filename, float data, int ecg_ch)
{
    createFolderIfNotExists(RES_FOLDER);
    char filePath[200]; // Adjust the size according to your need
    snprintf(filePath, sizeof(filePath), "%s/%s", RES_FOLDER, filename);
    if (BOOL_OUTPUT_CSV)
    {
        FILE *file = fopen(filePath, "a");
        if (file == NULL)
        {
            fprintf(stderr, "Error opening file: %s\n", filename);
            return;
        }

        // Write the data row
        if (ecg_ch == 2)
        {
            fprintf(file, "%.7f", data);
            fprintf(file, "\n");
        }
        else
        {
            fprintf(file, "%.7f,", data);
        }

        fclose(file);
    }
}

void print_var(float var, int ecg_id)
{
    printf("%f, ", var);
    if (ecg_id == 2)
    {
        printf("\r\n");
    }
}