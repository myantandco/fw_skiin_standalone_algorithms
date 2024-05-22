#include <string.h>
#include "abr_preprocess.h"
#include "ecg_algo.h"
#include "custom_chest.h"
#include "custom_waist.h"
#include "model.h"

//Low pass definitions
#define LP_FILTER_SIZE                  2
#define LP_FILTER_COEFF_LEN_A           2
#define LP_FILTER_COEFF_LEN_B           1

//Global variables definition
constexpr int kModelInputSize = 3;   // Number of model input values
constexpr int kOutputSize = ECG_ALGO_OUTPUT_SIZE; // Number of model output values
constexpr int kStateInputSize = 18;  // Total number of model states

static bool initialized = false;
static float states[kStateInputSize] = {0};
const unsigned char *model_buffer = g_model_waist;
static garment_id_e gar_id = GARMENT_UNDERWEAR;

static void preprocess_inputs(float *input, float *processed, bool restart)
{
    float x = 0;

    if((input == NULL) || (processed == NULL))
    {
        return;
    }

    for (uint8_t ecg_ch = 0; ecg_ch < kModelInputSize; ecg_ch++)
    {
        x = input[ecg_ch];

        // preprocessor: `subtract_baseline`
        x -= ABR_INPUT_BASELINE_VALUE;

        // preprocessor:
        x = get_preprocess_out(x,ecg_ch,restart, gar_id);

        processed[ecg_ch] = (float)x;
    }
}

void ecg_algo_update_garmentid(garment_id_e id)
{
    if((id>=MAX_GARMENTS))
    {
        return;
    }

    model_buffer = (id == GARMENT_UNDERWEAR) ? g_model_waist : g_model_chest;
    gar_id = id;
}

void ecg_algo_init(void)
{
    memset(states,0,kStateInputSize);

    if(gar_id  == GARMENT_UNDERWEAR)
    {
        custom_waist_setup(kModelInputSize, kStateInputSize, kOutputSize);
    }
    else
    {
        custom_chest_setup(kModelInputSize, kStateInputSize, kOutputSize);
    }

    initialized = true;
}

bool ecg_algo_run(float *data, uint8_t ch_count, bool restart)
{
    int ret = -1;
    float inp[kModelInputSize] = {0};
    float preproc_inp[kModelInputSize] = {0};

    if (!initialized)
    {
        return false;
    }

    if (ch_count != kModelInputSize)
    {
        return false;
    }

    if(restart)
    {
        memset(states,0,kStateInputSize);
    }

    inp[0] = data[0];
    inp[1] = data[1];
    inp[2] = data[2];

    //preprocess inputs
    preprocess_inputs(inp, preproc_inp, restart);

    if(gar_id  == GARMENT_UNDERWEAR)
    {
        custom_waist_set_inputs(preproc_inp);
        custom_waist_set_states(states);
        ret = custom_waist_inference();

    } 
    else
    {
        //chest
        custom_chest_set_inputs(preproc_inp);
        custom_chest_set_states(states);
        ret = custom_chest_inference();
    }

    return (ret==1);
}

void ecg_algo_get_output(float *outputs, uint8_t len)
{
    if ((len < kOutputSize) || (outputs == NULL))
    {
        return;
    }

    if (!initialized)
    {
        return;
    }

    // Get post inference states and outputs
    if (gar_id  == GARMENT_UNDERWEAR)
    {

        custom_waist_get_states(states);
        custom_waist_get_outputs(outputs);

    }
    else
    {

        custom_chest_get_states(states);
        custom_chest_get_outputs(outputs);
    }
}
