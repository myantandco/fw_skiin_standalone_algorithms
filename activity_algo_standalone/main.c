#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "activity.h"
#include "input.h"

int main(int argc, char const *argv[])
{
    imu_axis_t acc = {0};
    activity_t current_act = ACT_UNKNOWN;
    activity_t previous_act = ACT_UNKNOWN;

    // Inputs - hard-coded for now but in future update to pass by arguments
    printf("Setting inputs...\r\n");

    // Initialize and enable the activity algorithm
    printf("Initializing and enabling the activity algorithm...\r\n");
    act_init();

    // Loop through entire input array
    printf("Looping through input data...\r\n\r\n");
    for (uint32_t i = 0; i < INPUT_ARRAY_SIZE; i++)
    {
        // Load 1 sample into data buffers by channel
        acc.x = input_activity_x[i];
        acc.y = input_activity_y[i];
        acc.z = input_activity_z[i];

        // Pass accelerometer data to activity algorithm
        act_add_raw_acc(acc);
       
        // Check if enough samples were captured internally

        if (act_get_samples_count() > (N_ACC_SAMPLES - 1))
        {
            // Process data through algorithm
            current_act = process_act_algo();

            // If the activity changed, log and store
            if (current_act == previous_act)
            {
                printf("Activity changed from %d to %d", previous_act, current_act);
                previous_act = current_act;
            }
        }
    }

    printf("Data set complete, exiting...\r\n");
    return 0;
}