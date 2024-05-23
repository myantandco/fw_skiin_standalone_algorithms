#include "abr_preprocess.h"
#include "csv_writers.h"
#include "data_processing.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Filter length definitions
#define FILTER_LEN_QUALITY       2
#define FILTER_LEN_ECG           3
#define SOFTNESS_FILTER_LEN      12

// Notch filter definitions
#define NOTCH_FILTER_SIZE        5
#define NOTCH_FILTER_COEFF_LEN_A 5
#define NOTCH_FILTER_COEFF_LEN_B 5

#define KMODEL_INPUT_SIZE        3
#define SLOPE_MAX                5

static float notch_out[KMODEL_INPUT_SIZE][NOTCH_FILTER_SIZE] = {0};
static float notch_in[KMODEL_INPUT_SIZE][NOTCH_FILTER_SIZE]  = {0};

static float a_notch[] = {1.0l, -1.50977727l, 2.5144414l, -1.4684226l, 0.94597794l};
static float b_notch[] = {0.9726139l, -1.48909994l, 2.51519154l, -1.48909994l, 0.9726139l};

static const float a_notch_60hz[] = {1.0l, -1.50977727l, 2.5144414l, -1.4684226l, 0.94597794l};
static const float b_notch_60hz[] = {0.9726139l, -1.48909994l, 2.51519154l, -1.48909994l, 0.9726139l};

static const float a_notch_50hz[] = {1.0l, -2.19185687l, 3.14576208l, -2.1318192l, 0.94597794l};
static const float b_notch_50hz[] = {0.9726139l, -2.16183804l, 3.14651222l, -2.16183804l, 0.9726139l};

typedef struct
{
        uint8_t         slope;
        float           max_diff;
        quality_class_e q_class;
        bool            noise_detect;
        uint8_t         latch;
        float           filter_softness;
} quality_t;

typedef enum
{
    FQ_60HZ,
    FQ_50HZ,
} notch_fq;

// Global variables
static notch_fq  notch_cnf_fq_flag     = FQ_60HZ;
static quality_t quality_info[MAX_ECG] = {0};
static bool      filter_restart        = false;

static float   abr_ecg_process(float sample, ecg_sens_id ecg_id, bool restart);
static uint8_t latch_sigmoid(float sample, ecg_sens_id ecg_id);
static float   softness_filter(float sample, uint8_t ecg_ch, bool restart);
static void abr_quality_slope(float sample, ecg_sens_id ecg_id, float *sample_diff, bool restart);
static void abr_quality_process(float x, float sample, ecg_sens_id ecg_id, uint8_t *latch_out, float *filter_softness, bool *noise_detect, bool restart);

/*
 * @brief  This function applies lowpass and high pass to the ecg signal.
 * @param  sample - sample data from channel 1,2 or 3 depending upon the ecg id
 * @param  ecg_id - Channel id is used to keep track of the input data and
 * output data.
 * @retval It returns filtered value to get_preprocess_out function.
 */
static float abr_ecg_process(float sample, ecg_sens_id ecg_id, bool restart)
{
    float              output[] = {0, 0, 0};

    static const float ah[] = {1.0l, -1.88349555l, 0.88991837l};
    static const float bh[] = {0.94335348l, -1.88670696l, 0.94335348l};

    static const float al[] = {1.0l, -1.09241307l, 0.3910474l};
    static const float bl[] = {0.07465858l, 0.14931716l, 0.07465858l};

    static float       input_ecg[MAX_ECG][FILTER_LEN_ECG];
    static float       output_temp[MAX_ECG][FILTER_LEN_ECG];

    static float       input_temp[MAX_ECG][FILTER_LEN_ECG];
    static float       output_ecg[MAX_ECG][FILTER_LEN_ECG];

    output[ecg_id] = digital_filter((float)sample, input_ecg[ecg_id], output_temp[ecg_id], al, bl, 3, 3, FILTER_LEN_ECG, restart, 0);

    output[ecg_id] = digital_filter(output[ecg_id], input_temp[ecg_id], output_ecg[ecg_id], ah, bh, 3, 3, FILTER_LEN_ECG, restart, 0);

    return output[ecg_id];
}

/*
 * @brief  This function find the slope of ecg sample.
 * @param  sample - sample data from channel 1,2 or 3 depending upon the ecg id
 * @param  ecg_id - Channel id is used to keep track of the input data and
 * output data.
 * @param  *sample_diff- is a pointer used to store the max_diff between 2 ecg
 * sample
 * @retval Update the maximum diff between 2 ecg samples.
 */
static void abr_quality_slope(float sample, ecg_sens_id ecg_id, float *sample_diff, bool restart)
{
    static float temp_slope[] = {0, 0, 0};
    float        max          = 0;

    if (restart)
    {
        memset(temp_slope, 0, sizeof(temp_slope));
        *sample_diff = 0;
    }

    max = fabs(temp_slope[ecg_id] - sample);

    if (max > *sample_diff)
    {
        *sample_diff = max;
    }

    if (*sample_diff > SLOPE_MAX)
    {
        *sample_diff = SLOPE_MAX;
    }

    temp_slope[ecg_id] = sample;
    write_csv_single("lw1_rawslope.csv", max, ecg_id);
}

/*
 * @brief  This function applies lowpass and high pass to the quality signal,
 *         detect the noise in the signal and find filter_softness.
 * @param  sample - sample data from channel 1,2 or 3 depending upon the ecg id
 * @param  ecg_id - Channel id is used to keep track of the input data and
 * output data.
 * @retval Updates, noise_detection flag, latch_out and filter_softness.
 */
static void abr_quality_process(float x, float sample, ecg_sens_id ecg_id, uint8_t *latch_out, float *filter_softness, bool *noise_detect, bool restart)
{
    static const float ah[] = {1.0l, -0.9902304l};
    static const float bh[] = {0.9951152l, -0.9951152l};

    static const float al[] = {1.0l, -0.96148145l};
    static const float bl[] = {0.01925927l, 0.01925927l};

    static float       input_lp[MAX_ECG][FILTER_LEN_QUALITY];
    static float       output_lp[MAX_ECG][FILTER_LEN_QUALITY];

    static float       input_hp[MAX_ECG][FILTER_LEN_QUALITY];
    static float       output_hp[MAX_ECG][FILTER_LEN_QUALITY];

    static float       quality[MAX_ECG]      = {0, 0, 0};
    float              temp_quality[MAX_ECG] = {0, 0, 0};
    float              quality_class_temp    = 0;

    temp_quality[ecg_id] = digital_filter(x, input_hp[ecg_id], output_hp[ecg_id], ah, bh, 2, 2, FILTER_LEN_QUALITY, restart, 0);
    write_csv_single("q1_point5filt.csv", temp_quality[ecg_id], ecg_id);

    // calculate abs value
    temp_quality[ecg_id] = temp_quality[ecg_id] - sample;
    write_csv_single("q2_subtractecg.csv", temp_quality[ecg_id], ecg_id);
    temp_quality[ecg_id] = (float)fabs(temp_quality[ecg_id]);
    write_csv_single("q3_abs.csv", temp_quality[ecg_id], ecg_id);

    // lowpass 2Hz
    quality[ecg_id] = digital_filter(temp_quality[ecg_id], input_lp[ecg_id], output_lp[ecg_id], al, bl, 2, 2, FILTER_LEN_QUALITY, restart, 0);
    write_csv_single("q4_lp2hz.csv", quality[ecg_id], ecg_id);

    // Latch
    *latch_out = 1 - latch_sigmoid(quality[ecg_id], ecg_id);
    write_csv_single("q5_oneminuslatch.csv", *latch_out, ecg_id);

    *filter_softness = softness_filter(*latch_out, ecg_id, restart);
    write_csv_single("q6_filtersoft.csv", *filter_softness, ecg_id);

    quality_class_temp = 1 - *filter_softness;
    write_csv_single("q7_qclass.csv", quality_class_temp, ecg_id);
    if ((int)quality_class_temp != 0)
    {
        *noise_detect = true;
    }
    write_csv_single("q8_noisedetect.csv", (int)*noise_detect, ecg_id);
}

/*
 * @brief  This function uses filtered quality signal to produce latch output
 * @param  quality - quality data
 * @detail The filter quality value is checked with the upper and lower
 * threshold values. Depending upon the filter quality the latch is updated.
 * @retval It returns the latch
 */
static uint8_t latch_sigmoid(float quality, ecg_sens_id ecg_id)
{
    static uint8_t latch_q[MAX_ECG] = {0, 0, 0};

    if (quality > 1.2)
    {
        latch_q[ecg_id] = 1;
    }
    else if (quality < 0.15)
    {
        latch_q[ecg_id] = 0;
    }

    return latch_q[ecg_id];
}

/*
 * @brief  This function is used to give average of last 12 samples
 * @param  sample - sample data from channel 1,2 or 3 depending upon the ecg id
 * @detail The aim of this function is to provide moving average of quality
 * @retval It returns the moving average of quality signal
 */
static float softness_filter(float sample, uint8_t ecg_ch, bool restart)
{
    static float   latest_sample[MAX_ECG][SOFTNESS_FILTER_LEN] = {0};
    static uint8_t counter[MAX_ECG]                            = {0, 0, 0};

    const float intl_input[SOFTNESS_FILTER_LEN - 1] = {0.916666, 0.833333, 0.75, 0.666666, 0.583333, 0.5, 0.416666, 0.333333, 0.25, 0.166666, 0.083333};
    float avg_out = 0;

    if (restart)
    {
        memset(latest_sample, 0, sizeof(latest_sample));
        memset(counter, 0, sizeof(counter));
    }

    latest_sample[ecg_ch][SOFTNESS_FILTER_LEN - 1] = sample;

    for (uint8_t i = 0; i < SOFTNESS_FILTER_LEN; i++)
    {
        avg_out += latest_sample[ecg_ch][i] / SOFTNESS_FILTER_LEN;
    }

    if (counter[ecg_ch] < SOFTNESS_FILTER_LEN)
    {
        avg_out += intl_input[counter[ecg_ch]];
        counter[ecg_ch]++;
    }

    for (uint8_t j = 1; j < SOFTNESS_FILTER_LEN; j++)
    {
        latest_sample[ecg_ch][j - 1] = latest_sample[ecg_ch][j];
    }
    return avg_out;
}

/*
 * @brief  This function is used to get processed quality slope and class
 * @param  slope and quality pointer from sens_ecg.c
 * @detail It retrieves the data from the quality_t struct.
 * @retval It updates the pointer passed from the sens_ecg.c
 */
void abr_prep_get_quality(ecg_sens_id ecg_id, uint8_t *q_class, uint8_t *slope)
{
    if (quality_info[ecg_id].noise_detect == true)
    {
        quality_info[ecg_id].q_class = Q_NOISY;
    }
    else
    {
        quality_info[ecg_id].q_class = Q_CLEAN;
    }

    *q_class = quality_info[ecg_id].q_class - 1;
    *slope   = (uint8_t)quality_info[ecg_id].slope;

    quality_info[ecg_id].slope        = 0.0l;
    quality_info[ecg_id].noise_detect = false;
    quality_info[ecg_id].max_diff     = 0.0l;
}

/*
 * @brief  This function is used update the operating feq of notch filter
 * @param  Feq update flag
 * @detail The aim of this function is to update the feq of notch filter
 * @retval no return type
 */
void abr_update_notch_filter_coeff(bool freq_update)
{
    filter_restart = true;

    if (freq_update)
    {
        notch_cnf_fq_flag = FQ_50HZ;
        memcpy(a_notch, a_notch_50hz, sizeof(a_notch));
        memcpy(b_notch, b_notch_50hz, sizeof(b_notch));
    }
    else
    {
        notch_cnf_fq_flag = FQ_60HZ;
        memcpy(a_notch, a_notch_60hz, sizeof(a_notch));
        memcpy(b_notch, b_notch_60hz, sizeof(b_notch));
    }
}

/*
 * @brief  This function takes ecg data in mV and provides the quality of the
 *         signal and prepocess the ecg values for the ABR2.0 model. This
 * function also provides with the quality of the ecg signal.
 * @param  x - Our implementation has 3 ecg channels. Here x is a mV output of
 *         ECG channel_1,ECG channel_2 and ECG channel_3.
 * @param  ech_ch - Channel id is used to keep track of the input data and
 * output data.
 * @detail Pre-processing mainly contains filter to remove noise from the signal
 *         and provide with latch, quality and slope of the signal.
 * @retval it returns processed ecg which is feed to ABR2.0 model.
 */
float get_preprocess_out(float x, uint8_t ecg_ch, bool restart, garment_id_e gar_id)
{
    float filtered_ecg[MAX_ECG]  = {0, 0, 0};
    float processed_ecg[MAX_ECG] = {0, 0, 0};
    float temp_ecg[MAX_ECG]      = {0, 0, 0};

    // 1) Reset filter if requested
    if (filter_restart == true)
    {
        memset(notch_in, 0, sizeof(notch_in));
        memset(notch_out, 0, sizeof(notch_out));
        filter_restart = false;
    }

    // 2) Apply digital filter
    temp_ecg[ecg_ch] = digital_filter(x, notch_in[ecg_ch], notch_out[ecg_ch], a_notch, b_notch, NOTCH_FILTER_COEFF_LEN_A, NOTCH_FILTER_COEFF_LEN_B, NOTCH_FILTER_SIZE, restart, 0);
    write_csv_single("e1_notch.csv", temp_ecg[ecg_ch], ecg_ch);

    // 3) Apply bandpass filter
    filtered_ecg[ecg_ch] = abr_ecg_process(temp_ecg[ecg_ch], (ecg_sens_id)ecg_ch, restart);
    write_csv_single("e2_bp.csv", filtered_ecg[ecg_ch], ecg_ch);

    // 4) Calculate quality slope and normalize
    abr_quality_slope(
        filtered_ecg[ecg_ch], (ecg_sens_id)ecg_ch, &quality_info[ecg_ch].max_diff, restart);
    quality_info[ecg_ch].slope = (uint8_t)((quality_info[ecg_ch].max_diff / 5) * 200);

    // 5) Process Quality
    abr_quality_process(x,
                        filtered_ecg[ecg_ch],
                        (ecg_sens_id)ecg_ch,
                        &quality_info[ecg_ch].latch,
                        &quality_info[ecg_ch].filter_softness,
                        &quality_info[ecg_ch].noise_detect,
                        restart);

    // 6) Generate processed ECG output
    processed_ecg[ecg_ch] = (filtered_ecg[ecg_ch] * quality_info[ecg_ch].filter_softness);
    write_csv_single("e3_filtxlatch.csv", processed_ecg[ecg_ch], ecg_ch);

    return processed_ecg[ecg_ch];
}

