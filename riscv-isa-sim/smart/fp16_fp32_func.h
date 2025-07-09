#ifndef _fp_half_func_h
#define _fp_half_func_h

#include <stdio.h>
#include <string.h>

//fp16 to fp32 func
typedef unsigned short Half;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef union suf32
{
    int i;
    unsigned u;
    float f;
} suf32;

uint as_uint(const float x);
float as_float(const uint x);
float half_to_float(Half h);
Half float_to_half(float f);
void float_to_half_array(Half *dst, float *src, int size);
void half_to_float_array(float *dst, Half *src, int size);
Half half_add(Half a, Half b);
Half half_sub(Half a, Half b);
Half half_mul(Half a, Half b);
Half half_div(Half a, Half b);
Half half_min(Half a, Half b);
Half half_max(Half a, Half b);

#endif // _fp_half_func_h