#ifndef SRC_ALGORITHMS_ACTIVITY_H_
#define SRC_ALGORITHMS_ACTIVITY_H_

#include <stdbool.h>
#include <stdint.h>

#define N_ACC_SAMPLES   12  // Number of samples to accumulate before calculating the posture

/// XXX - Added here for standalone application
typedef struct
{
  int16_t x;
  int16_t y;
  int16_t z;
  uint32_t ts;
}imu_axis_t;

// XXX - Added below for standalone application only
typedef enum
{
    GARMENT_UNDERWEAR        = 0x00,
    GARMENT_BRA_TANK         = 0x01,
    GARMENT_CHEST_BAND       = 0x02,
    GARMENT_BRALETTE         = 0x03,
    GARMENT_PEDIATRIC_BAND   = 0x04,
    //do not define garments below
    MAX_GARMENTS,
} garment_id_e;

typedef enum
{
    ACT_UNKNOWN     = 0,
    ACT_WALKING     = 1,
    ACT_RUNNING     = 2,
    ACT_SEAT_STAND  = 3,
    ACT_LYING_LEFT  = 4,
    ACT_LYING_RIGHT = 5,
    ACT_LYING_UP    = 6,
    ACT_LYING_DOWN  = 7,
    ACT_LYING       = 8,
    ACT_RESERVED    = 9,
    ACT_RESERVED2   = 10,
    ACT_RESERVED3   = 11,
    ACT_RESERVED4   = 12,
}activity_t;

typedef struct
{
    uint32_t time_detected;     // Detection time 
    activity_t current;         // Last valid activity
}act_data_t;

void act_init(void);
void act_add_raw_acc(imu_axis_t axis);
activity_t process_act_algo(void);
uint16_t act_get_samples_count(void);

#endif /* SRC_ALGORITHMS_ACTIVITY_H_ */
