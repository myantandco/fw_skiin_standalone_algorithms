#include <math.h>
#include <string.h>
#include "activity.h"

#define GARMENT_ID_DEFAULT  GARMENT_UNDERWEAR   // Assume underwear, all other garments function the same way

//number of samples to accumulate before calculating the posture
// #define N_ACC_SAMPLES   12   XXX - Moved to header
#define ACT_TX_SAMP     1
#define PREPROCESS_FACTOR               0.00239501953125 // 9.81/4096

// the values are provided by data science team
#define ACCX_MEAN_UPPER_THRESHOLD       -4.520100
#define ACCX_MEAN_LOWER_THRESHOLD       -8.251441
#define R_P2P_UPPER_THRESHOLD           1.002060
#define R_P2P_LOWER_THRESHOLD           0.713003
#define THETA_MEAN_THRESHOLD            1.101531
#define ACCX_STD_THRESHOLD              5.999058

//vector of 3-axes point
typedef struct
{
    float x;
    float y;
    float z;
}axis_t;

typedef struct
{
    axis_t raw;
    double accx_mean;
    double accx_std;
    double theta_mean;
    double r_p2p;
    double theta;
    double r;
}algo_pp_t;

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t samples;
    bool processing;
}acc_axis_t;


static acc_axis_t rawaxis = {0};
static act_data_t activity = {0};   // Last valid detected activity
static bool algo_enabled = true;    // Flag to enable/disable activity algo

// Algo variables
static algo_pp_t algo_input = {0};
static double sample_pp_x[N_ACC_SAMPLES] = {0};
static double sample_pp_r[N_ACC_SAMPLES] = {0};

static inline bool is_valid_sample(imu_axis_t *sample);
static void reset_activity_parameters(void);
static void adapt_axis(garment_id_e garment, acc_axis_t *acc);
static void act_algo_preprocess(acc_axis_t *raw, uint8_t sample_number);
static activity_t act_algo_uncalibrated_model(void);

/*
 * @brief  This function resets the all the activity parameters.
 * @details The axis of IMU is updated depending upon the garment type.
 */
static void reset_activity_parameters(void)
{
    rawaxis = (acc_axis_t){0, 0, 0, 0, false};

    activity.current = ACT_UNKNOWN;
    activity.time_detected = 0;

    algo_input.accx_mean = 0;
    algo_input.accx_std = 0;
    algo_input.r = 0;
    algo_input.r_p2p = 0;
    algo_input.theta = 0;
    algo_input.theta_mean = 0;
}

static inline bool is_valid_sample(imu_axis_t *sample)
{
    bool valid = false;

    valid =  0xFFFF != sample->x;
    valid &= 0xFFFF != sample->y;
    valid &= 0xFFFF != sample->z;

    return valid;
}

/*
 * @brief  This function adjust the axis of IMU for the algorithm.
 * @param  garment id: defines which garment is configured
 * @details The axis of IMU is updated depending upon the garment type.
 * @retval this function updates the axis.
 */
static void adapt_axis(garment_id_e garment, acc_axis_t *acc)
{
    int16_t temp16 = 0;

    if ((garment == GARMENT_BRA_TANK) || (garment == GARMENT_BRALETTE))
    {
        acc->x = -1 * acc->x;
        acc->z = -1 * acc->z;
    }
    else if (garment == GARMENT_CHEST_BAND)
    {
        temp16 = acc->x;
        acc->x = -1 * acc->y;
        acc->y = temp16;
    }
}

/*
 * @brief  This is a algorithm model. The THRESHOLD values are uncalibrated.
 * @details Depending upon the threshold values the current activity is detected.
 * @retval this function returns the detected activity.
 */
static activity_t act_algo_uncalibrated_model(void)
{
   activity_t algo_output_act = ACT_UNKNOWN;

    if (algo_input.r_p2p <= R_P2P_UPPER_THRESHOLD)
    {
        if (algo_input.accx_mean <= ACCX_MEAN_UPPER_THRESHOLD)
        {
            if (algo_input.r_p2p <= R_P2P_LOWER_THRESHOLD)
            {
                algo_output_act = ACT_SEAT_STAND;
            }
            else
            {
                algo_output_act = ACT_SEAT_STAND;
            }
        }
        else
        {
            if (algo_input.theta_mean <= THETA_MEAN_THRESHOLD)
            {
                algo_output_act = ACT_SEAT_STAND;
            }
            else
            {
                algo_output_act = ACT_LYING;
            }
        }
    }
    else
    {
        if (algo_input.accx_std <= ACCX_STD_THRESHOLD)
        {
            if (algo_input.accx_mean <= ACCX_MEAN_LOWER_THRESHOLD)
            {
                algo_output_act = ACT_WALKING;
            }
            else
            {
                algo_output_act = ACT_WALKING;
            }
        }
        else
        {
            algo_output_act = ACT_RUNNING;
        }
    }

    algo_input.accx_mean = 0;
    algo_input.theta_mean = 0;

    return algo_output_act;
}

/*
 * @brief This function pre-process the x,y,z axis data which are feed to algorithm model
 * @details This function calculates the parameters needed to process activity algo.
 *          This parameters inculde's mean, theta, and r calculation.
 * @retval This function updates algo_pp_t structure.
 */
static void act_algo_preprocess(acc_axis_t *raw, uint8_t index_number)
{
   algo_input.raw.x = (double) (raw->x*PREPROCESS_FACTOR);
   algo_input.raw.y = (double) (raw->y*PREPROCESS_FACTOR);
   algo_input.raw.z = (double) (raw->z*PREPROCESS_FACTOR) * (-1);
   algo_input.r     = sqrt((algo_input.raw.x*algo_input.raw.x) +
                        (algo_input.raw.y*algo_input.raw.y) +
                        (algo_input.raw.z*algo_input.raw.z));

   algo_input.theta = acos(algo_input.raw.x/algo_input.r);
   algo_input.accx_mean  += algo_input.raw.x;
   algo_input.theta_mean += algo_input.theta;

   if (index_number >= (N_ACC_SAMPLES - 1))
   {
       algo_input.accx_mean  = algo_input.accx_mean/N_ACC_SAMPLES;
       algo_input.theta_mean = algo_input.theta_mean/N_ACC_SAMPLES;
   }

   sample_pp_x[index_number] = algo_input.raw.x;
   sample_pp_r[index_number] = algo_input.r;
}

void act_init(void)
{
    reset_activity_parameters();
}

/*
 * @brief  This function is used to fetch get axis data from IMU
 * @details The axis data is fetched from the IMU.
 * @retval This function trigger the activity task.
 */
void act_add_raw_acc(imu_axis_t axis)
{
    if (!algo_enabled)
    {
        return;
    }

    if (!is_valid_sample(&axis))
    {
        return;
    }

    //activity task is not processing the posture
    rawaxis.x = axis.x;
    rawaxis.y = axis.y;
    rawaxis.z = axis.z;

    adapt_axis(GARMENT_ID_DEFAULT, &rawaxis);
    act_algo_preprocess(&rawaxis,rawaxis.samples);

    rawaxis.samples++;

    // Moved to main.c
    /*if (rawaxis.samples > (N_ACC_SAMPLES - 1))
    {
        rawaxis.processing = true;
        rawaxis.samples = 0;
        tsk_resume_by_id(TSK_ACTIVITY);
    }*/
}

/*
 * @brief  This function finds the standard deviation and peak to peak values from
 *         x - axis data.
 * @details Finds r peak to peak and std values form  x- axis data.
 * @retval this function updates algo_pp_t structure.
 */
activity_t process_act_algo(void)
{
    activity_t detected_act = ACT_UNKNOWN;
    float r_max = 0;
    float r_min = 0;
    float std = 0;

    rawaxis.samples = 0; // XXX - added here clear sample count

    r_max = r_min = sample_pp_r[0]; // to avoid error

    for(uint8_t i = 0; i < N_ACC_SAMPLES; i++)
    {

        std += (sample_pp_x[i] - algo_input.accx_mean) *
            (sample_pp_x[i] - algo_input.accx_mean);// to avoid type casting

        if (sample_pp_r[i] > r_max )
        {
            r_max = sample_pp_r[i];
        }

        if (sample_pp_r[i] < r_min)
        {
            r_min = sample_pp_r[i];
        }
    }

    algo_input.accx_std = sqrt(std/N_ACC_SAMPLES);
    algo_input.r_p2p = r_max - r_min;
    detected_act = act_algo_uncalibrated_model();

    return detected_act;
}

uint16_t act_get_samples_count(void)
{
    return rawaxis.samples;
}