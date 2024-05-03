#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "data_processing.h"

double digital_filter(double input, double *in, double *out,
        const double *a, const double *b, uint8_t a_len, uint8_t b_len,
        uint8_t filter_order,bool reset, double initial_sample)
{
    if ((a == NULL) || (b == NULL) || (in == NULL) || (out == NULL))
    {
        return 0;
    }

    if ((!filter_order) || (!a_len) || (!b_len))
    {
        return 0;
    }

    if ((a_len > filter_order) || (b_len > filter_order))
    {
        return 0;
    }

    double output = 0.0;
    double tmp = 0.0;

    if(reset)
    {
        output = 0;
        for(uint8_t i = 0; i < filter_order; i++)
        {
            in[i] = initial_sample;
            out[i] = 0;
        }
    }

    in[filter_order - 1] = input;
    
    for(uint8_t i = 0; i < b_len; i++)
    {
        tmp += b[i] * in[filter_order - 1 - i];
    }

    for(uint8_t i = 1; i < a_len; i++)
    {
        tmp -= a[i] * out[filter_order - 1 - i];
    }

    tmp /= a[0];
    output = tmp;
    out[filter_order - 1] = output;

    for(uint8_t i = 1; i < filter_order; i++)
    {
        in[i - 1] = in[i];
        out[i - 1] = out[i];
    }

    return output;
}
