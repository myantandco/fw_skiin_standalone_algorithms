#include <string.h>
#include "abr_preprocess.h"
#include "ecg_algo.h"
#include "custom_chest.h"
#include "custom_waist.h"
#include "model.h"

//Global variables definition
constexpr int kModelInputSize = ECG_ALGO_INPUT_SIZE;       // Number of model input values
constexpr int kOutputSize     = ECG_ALGO_OUTPUT_SIZE;      // Number of model output values
constexpr int kStateInputSize = ECG_ALGO_STATE_INPUT_SIZE; // Total number of model states

static bool fInitDone = false;
static float pdStates[kStateInputSize] = {0};
const unsigned char *pModelBuffer = g_model_waist;
static garment_id_e nGarmentID = GARMENT_UNDERWEAR;

void ECGAlgo_SetGarmentID(garment_id_e nID)
{
    // 1) Check arguements
    if (nID >= MAX_GARMENTS)
    {
        return;
    }

    // 2) Set model
    if (nID == GARMENT_UNDERWEAR)
    {
        pModelBuffer = g_model_waist;
    }
    else
    {
        pModelBuffer = g_model_chest;
    }

    // 3) Store garment ID
    nGarmentID = nID;

    return;
}

void ECGAlgo_Init(void)
{
    // 1) Clear pdStates 
    memset(pdStates, 0, kStateInputSize);

    // 2) Based on garment type, set-up the appropriate model
    if (nGarmentID  == GARMENT_UNDERWEAR)
    {
        custom_waist_setup(kModelInputSize, kStateInputSize, kOutputSize);
    }
    else
    {
        custom_chest_setup(kModelInputSize, kStateInputSize, kOutputSize);
    }

    // 3) Mark as initialized
    fInitDone = true;

    return;
}

bool ECGAlgo_Run(float *pdData, uint8_t bChannelCount, bool fRestart)
{
    int ret = 0;
    float pdInput[kModelInputSize] = {0};
    float pdPreprocessorInput[kModelInputSize] = {0};
    float dTemp = 0;

    // 1) Check arguments
    if (bChannelCount != kModelInputSize)
    {
        return false;
    }

    // 2) Check if intialized
    if (!fInitDone)
    {
        return false;
    }

    // 3) If fRestart was set, clear pdStates
    if (fRestart)
    {
        memset(pdStates, 0, kStateInputSize);
    }

    // 4) Extract data for pre-processing
    pdInput[0] = pdData[0];
    pdInput[1] = pdData[1];
    pdInput[2] = pdData[2];

    // 5) Pre-process inputs
    for (uint8_t ecg_ch = 0; ecg_ch < kModelInputSize; ecg_ch++)
    {
        dTemp = pdInput[ecg_ch];

        // subtract_baseline
        dTemp -= ABR_INPUT_BASELINE_VALUE;

        // preprocessor:
        dTemp = ABRPreProcess_GetOutput(dTemp, ecg_ch, fRestart, nGarmentID);

        pdPreprocessorInput[ecg_ch] = (float)dTemp;
    }


    // 6) Set inputs, pdStates based on garment type selected
    if(nGarmentID  == GARMENT_UNDERWEAR)
    {
        custom_waist_set_inputs(pdPreprocessorInput);
        custom_waist_set_states(pdStates);
        ret = custom_waist_inference();
    } 
    else
    {
        custom_chest_set_inputs(pdPreprocessorInput);
        custom_chest_set_states(pdStates);
        ret = custom_chest_inference();
    }

    return (ret==1);
}

void ECGAlgo_GetOutput(float *pdOutputs, uint8_t bLength)
{
    // 1) Check arguments
    if ((bLength < kOutputSize) || (pdOutputs == NULL))
    {
        return;
    }

    // 2) Check if initialized
    if (!fInitDone)
    {
        return;
    }

    // 3) Get post inference pdStates and outputs
    if (nGarmentID  == GARMENT_UNDERWEAR)
    {
        custom_waist_get_states(pdStates);
        custom_waist_get_outputs(pdOutputs);
    }
    else
    {
        custom_chest_get_states(pdStates);
        custom_chest_get_outputs(pdOutputs);
    }

    return;
}
