#ifndef ABR_POSTPROCESS_H_
#define ABR_POSTPROCESS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Functions declarations
void ABRPostProcess_RPeak(float rpeak, uint8_t count);
void ABRPostProcess_GetRPeak(uint8_t *rpeak_max, uint8_t *rpeak_index);

#endif /* ABR_POSTPROCESS_H_ */
