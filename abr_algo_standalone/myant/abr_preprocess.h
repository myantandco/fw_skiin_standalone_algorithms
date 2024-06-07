#ifndef ABR_PREPROCESS_H_
#define ABR_PREPROCESS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// XXX - Added below for standalone application only
typedef enum
{
    ECG1,
    ECG2,
    ECG3,
    MAX_ECG,
} ecg_sens_id;

// XXX - Added below for standalone application only
typedef enum
{
    GARMENT_UNDERWEAR      = 0x00,
    GARMENT_BRA_TANK       = 0x01,
    GARMENT_CHEST_BAND     = 0x02,
    GARMENT_BRALETTE       = 0x03,
    GARMENT_PEDIATRIC_BAND = 0x04,
    // do not define garments below
    MAX_GARMENTS,
} garment_id_e;

typedef enum
{
    Q_UNKNOWN = 0,
    Q_NOISY,
    Q_CLEAN,
    MAX_QUALITY,
} quality_class_e;

float ABRPreProcess_GetOutput(float x, uint8_t ecg_ch, bool restart, garment_id_e gar_id);
void ABRPreProcess_GetQuality(ecg_sens_id ecg_id, uint8_t *q_class, uint8_t *slope);
void ABRPreProcess_SetNotchFilterCoeffient(bool freq_update);
void ABRPreProcess_SetLatchLimits(garment_id_e nID);

#endif /* ABR_PREPROCESS_H_ */
