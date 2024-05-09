#ifndef CSV_WRITERS_H_
#define CSV_WRITERS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> // For mkdir()
#include "config.h"
#include "csv_writers.h"

// Function prototypes
void createFolderIfNotExists(const char *folderName);

void read_csv(const char *filename, float input_ch1[], float input_ch2[], float input_ch3[], int *num_rows);

void write_csv_header(const char *filename, const char **var_names, int num_vars);

void write_csv_row(const char *filename, float data[], int num_vars);

void write_csv_single(const char *filename, float data, int ecg_ch);

void print_var(float var, int ecg_id);

#endif