#ifndef ABR_POSTPROCESS_H_
#define ABR_POSTPROCESS_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

//Functions declarations
void abr_pp_rpeak(float rpeak, uint8_t count);
void abr_pp_get_rpeak(uint8_t *rpeak_max, uint8_t *rpeak_index);

#endif /* ABR_POSTPROCESS_H_ */
