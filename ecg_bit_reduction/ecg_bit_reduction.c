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

static const double a_hp[] = {1.0l,                 -1.9991669594972l,  0.9991673063310l};
static const double b_hp[] = {0.99958356645707l,    -1.99916713291414l, 0.99958356645705l};
static double hp_in[MAX_ECG][HP_FILTER_SIZE] = {0};
static double hp_out[MAX_ECG][HP_FILTER_SIZE] = {0};

// --- Functions ---

static bool check_restart_filter( uint32_t raw_sample, ecg_sens_id ecg_id, bool restart)
{
    //mean sample diff
    static double mean[MAX_ECG] = {0};
    static uint32_t ecg_prev[MAX_ECG] = {0};
    static bool last_highamp_flag[MAX_ECG] = {0};
    static uint8_t sample_count[MAX_ECG] = {0};

    int32_t succ_diff = 0;

    // Check arguments
    if (ecg_id >= MAX_ECG)
    {
        return 0;
    }

    if (restart)
    {
        mean[ecg_id] = (double)raw_sample;
        ecg_prev[ecg_id] = raw_sample;
        last_highamp_flag[ecg_id] = 0;
        sample_count[ecg_id] = 0;
        return true;
    }

    // Calculate mean
    mean[ecg_id] =  ((1.0 - BR_MEAN_SAMPLE_COUNT_RES) * mean[ecg_id] + (BR_MEAN_SAMPLE_COUNT_RES) * (double)raw_sample);

    // Calculate difference of successive samples
    succ_diff = raw_sample - ecg_prev[ecg_id];
    succ_diff = abs(succ_diff);
    ecg_prev[ecg_id] = raw_sample;

    /*
     * Check 2 logics to raise high amplitude change flag
     * 1. Check if the differnce between current sample and
     * mean is greater than BR_THRESHOLD_MEAN_DIFF
     * 2. Check if successive difference between two samples
     * is greater than BR_THRESHOLD_SUCCESSIVE_DIFF
     */
    if ((abs(raw_sample - mean[ecg_id]) > BR_THRESHOLD_MEAN_DIFF) &&    (succ_diff > BR_THRESHOLD_SUCCESSIVE_DIFF))
    {
        last_highamp_flag[ecg_id] = 1;
        sample_count[ecg_id] = 0;
    }

    /*
     * If an high amplitude change flag is set, look for sample
     * setting down for BR_SAMPLE_WINDOW samples.
     * This is identified when the change between consecutive
     * samples is less than BR_THRESHOLD_AMP_SETTLING
     */
    if (last_highamp_flag[ecg_id])
    {
        if ((sample_count[ecg_id] > BR_SAMPLE_WINDOW) || (succ_diff < BR_THRESHOLD_AMP_SETTLING))
        {
            last_highamp_flag[ecg_id] = 0;
            sample_count[ecg_id] = 0;
            return true;
        }

        sample_count[ecg_id]++;
    }

    return false;

}

static int16_t lsb_removal(double data_sample)
{
    double processed_lsb = 0;

    //remove lsb and return
    processed_lsb = (double)(data_sample)/BR_LSB_FACTOR;

    return (int16_t)(floor(processed_lsb));
}

static double msb_removal( uint32_t data_sample, ecg_sens_id ecg_id, bool restart)
{
    static bool flt_reset_flag_ecg[MAX_ECG] ={false, false, false};

    double processed_msb = 0;

    // Check arguments
    if (ecg_id >= MAX_ECG)
    {
        return 0;
    }

    // Check_restart
    flt_reset_flag_ecg[ecg_id] = check_restart_filter(data_sample,ecg_id,restart);

    // Filter data
    processed_msb = digital_filter((double)data_sample, hp_in[ecg_id], hp_out[ecg_id], a_hp, b_hp, HP_FILTER_COEFF_LEN_A, HP_FILTER_COEFF_LEN_B, HP_FILTER_SIZE, flt_reset_flag_ecg[ecg_id], (double)data_sample);
    
    // Reset flt flag
    flt_reset_flag_ecg[ecg_id] = false;
    
    // Remove msb
    if ((processed_msb >= BR_MSB_THRSHOLD) || (processed_msb <= -1 * BR_MSB_THRSHOLD))
    {
        processed_msb = (int32_t)(BR_MSB_THRSHOLD) * (processed_msb / (int32_t)abs(processed_msb));
    }

    return processed_msb;
}

int16_t ecgbr_sample_reduction(uint32_t sample, ecg_sens_id ecg_id, bool restart)
{
    double processed_msb = 0;

    // Check arguments
    if (ecg_id >= MAX_ECG)
    {
        return 0;
    }

    // Remove MSB
    processed_msb = msb_removal(sample,ecg_id,restart);

    // Remove LSB
    return lsb_removal(processed_msb);
}
