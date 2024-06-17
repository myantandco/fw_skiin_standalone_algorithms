#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "ecg_bit_reduction.h"
#include "data_processing.h"

// --- Defines and macros ---

// Bit reduction algorithm definitions
#define BR_THRESHOLD_AMP_SETTLING       100
#define BR_THRESHOLD_MEAN_DIFF          233333          // 20mv
#define BR_THRESHOLD_SUCCESSIVE_DIFF    11666           // 1mv
#define BR_MEAN_SAMPLE_COUNT_RES        0.0009765625l   // 1/1024
#define BR_SAMPLE_WINDOW                80              // SAMPLES
#define BR_MSB_TO_REMOVE                5
#define BR_LSB_TO_REMOVE                7

/* 
 * MSB_THRSHOLD Formula   (12800000/2)/(2^MSB_TO_REMOVE)
 * 6400000/32
 * 20000 bit reduced threshold
 */
#define BR_MSB_THRSHOLD                 200000

/* 
 * LSB_FACTOR   (2^LSB_TO_REMOVE)
 * 2^7 = 128
 */
#define BR_LSB_FACTOR                   128.0l

// High Pass Filter definitions
#define HP_FILTER_SIZE                  3
#define HP_FILTER_COEFF_LEN_A           3
#define HP_FILTER_COEFF_LEN_B           3

// --- Globals ---

static const double gflHighpassCoeffiecientsA[] = {1.0l,                 -1.9991669594972l,  0.9991673063310l};
static const double gflHighpassCoeffiecientsB[] = {0.99958356645707l,    -1.99916713291414l, 0.99958356645705l};
static double gflHighpassInput[MAX_ECG][HP_FILTER_SIZE] = {0};
static double gflHighpassOutput[MAX_ECG][HP_FILTER_SIZE] = {0};

static double gflMeanValue[MAX_ECG] = {0};
static uint32_t gbPreviousECG[MAX_ECG] = {0};
static bool gfLastHighAmplitudeFlag[MAX_ECG] = {0};
static uint8_t gbSampleCount[MAX_ECG] = {0};

static bool gfResetFlagECG[MAX_ECG] = {false, false, false};


// --- Functions ---

static bool ECGBitReduction_CheckRestartFilter(uint32_t bRawSample, ecg_sens_id nECGId, bool fRestart)
{
    int32_t bSuccesiveDifference = 0;

    // Check arguments
    if (nECGId >= MAX_ECG)
    {
        return false;
    }

    if (fRestart)
    {
        gflMeanValue[nECGId] = (double)bRawSample;
        gbPreviousECG[nECGId] = bRawSample;
        gfLastHighAmplitudeFlag[nECGId] = 0;
        gbSampleCount[nECGId] = 0;
        return true;
    }

    // Calculate mean
    gflMeanValue[nECGId] =  ((1.0 - BR_MEAN_SAMPLE_COUNT_RES) * gflMeanValue[nECGId] + (BR_MEAN_SAMPLE_COUNT_RES) * (double)bRawSample);

    // Calculate difference of successive samples
    bSuccesiveDifference = bRawSample - gbPreviousECG[nECGId];
    bSuccesiveDifference = abs(bSuccesiveDifference);
    gbPreviousECG[nECGId] = bRawSample;

    /*
     * Check 2 logics to raise high amplitude change flag
     * 1. Check if the differnce between current sample and
     * mean is greater than BR_THRESHOLD_MEAN_DIFF
     * 2. Check if successive difference between two samples
     * is greater than BR_THRESHOLD_SUCCESSIVE_DIFF
     */
    if ((abs(bRawSample - gflMeanValue[nECGId]) > BR_THRESHOLD_MEAN_DIFF) && (bSuccesiveDifference > BR_THRESHOLD_SUCCESSIVE_DIFF))
    {
        gfLastHighAmplitudeFlag[nECGId] = 1;
        gbSampleCount[nECGId] = 0;
    }

    /*
     * If an high amplitude change flag is set, look for sample
     * setting down for BR_SAMPLE_WINDOW samples.
     * This is identified when the change between consecutive
     * samples is less than BR_THRESHOLD_AMP_SETTLING
     */
    if (gfLastHighAmplitudeFlag[nECGId])
    {
        if ((gbSampleCount[nECGId] > BR_SAMPLE_WINDOW) || (bSuccesiveDifference < BR_THRESHOLD_AMP_SETTLING))
        {
            gfLastHighAmplitudeFlag[nECGId] = 0;
            gbSampleCount[nECGId] = 0;
            return true;
        }

        gbSampleCount[nECGId]++;
    }

    return false;
}

static int16_t ECGBitReduction_LSBRemoval(double bSample)
{
    double flProcessedLSB = 0;

    //remove lsb and return
    flProcessedLSB = (double)(bSample) / BR_LSB_FACTOR;

    return (int16_t)(floor(flProcessedLSB));
}

static double ECGBitReduction_MSBRemoval(uint32_t bSample, ecg_sens_id nECGId, bool fRestart)
{
    double flProcessedMSB = 0;

    // Check arguments
    if (nECGId >= MAX_ECG)
    {
        return 0;
    }

    // Check_restart
    gfResetFlagECG[nECGId] = ECGBitReduction_CheckRestartFilter(bSample, nECGId, fRestart);

    // Filter data - XXX remove ecgbr_digital_filter() and replace with standard float-based digital_filter function
    flProcessedMSB = ecgbr_digital_filter((double)bSample, gflHighpassInput[nECGId], gflHighpassOutput[nECGId], gflHighpassCoeffiecientsA, gflHighpassCoeffiecientsB, 
        HP_FILTER_COEFF_LEN_A, HP_FILTER_COEFF_LEN_B, HP_FILTER_SIZE, gfResetFlagECG[nECGId], (double)bSample);
    
    // Reset flt flag
    gfResetFlagECG[nECGId] = false;
    
    // Remove msb
    if ((flProcessedMSB >= BR_MSB_THRSHOLD) || (flProcessedMSB <= -1 * BR_MSB_THRSHOLD))
    {
        flProcessedMSB = (int32_t)(BR_MSB_THRSHOLD) * (flProcessedMSB / (int32_t)abs(flProcessedMSB));
    }

    return flProcessedMSB;
}

int16_t ECGBitReduction_SampleReduction(uint32_t bSample, ecg_sens_id nECGId, bool fRestart)
{
    double flProcessedMSB = 0;

    // Check arguments
    if (nECGId >= MAX_ECG)
    {
        return 0;
    }

    // Remove MSB
    flProcessedMSB = ECGBitReduction_MSBRemoval(bSample, nECGId, fRestart);

    // Remove LSB
    return ECGBitReduction_LSBRemoval(flProcessedMSB);
}
